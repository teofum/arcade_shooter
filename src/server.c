#include <fcntl.h>
#include <raylib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "entity.h"
#include "entity_list.h"
#include "game.h"
#include "network_shared.h"
#include "server.h"
#include "utils.h"

typedef struct ServerState {
  i32 sock;
  u64 last_heartbeat_sent;
  u32 last_seq_recvd;
  u32 seq;

  bool main_menu;

  Connection clients[MAX_CLIENTS];

  Message recvd_msg;
  struct sockaddr_in sender_addr;
} ServerState;

static ServerState server = {0};

i32 server_init() {
  printf("Starting server...\n");

  // Initialize socket
  i32 sock = socket(PF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
    perror("Failed to create socket");
    return -1;
  }

  // Bind socket
  struct sockaddr_in serv_addr = {
      .sin_family = AF_INET,
      .sin_port = htons(SERVER_PORT),
  };
  inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
  if (bind(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    perror("Failed to bind socket");
    return -1;
  }

  // Make reads from socket nonblocking
  fcntl(sock, F_SETFL, fcntl(sock, F_GETFL) | O_NONBLOCK);

  server = (ServerState){
      .sock = sock,
      .last_heartbeat_sent = now(),
      .main_menu = false,
      .clients = {0},
      .recvd_msg = {0},
  };

  printf("Server listening on 127.0.0.1:%d\n", SERVER_PORT);
  return 0;
}

static bool cmp_addr(struct sockaddr_in *a, struct sockaddr_in *b) {
  return memcmp(a, b, sizeof(struct sockaddr_in)) == 0;
}

static Connection *find_client(struct sockaddr_in *addr) {
  for (u32 i = 0; i < MAX_CLIENTS; i++) {
    Connection *cli = &server.clients[i];
    if (is_connected(cli) && cmp_addr(&cli->addr, addr))
      return cli;
  }

  return NULL;
}

static bool connect_client(struct sockaddr_in *addr) {
  for (u32 i = 0; i < MAX_CLIENTS; i++) {
    if (!is_connected(&server.clients[i])) {
      server.clients[i] = (Connection){
          .addr = *addr,
          .last_seen = now(),
      };
      return true;
    }
  }

  return false;
}

static bool disconnect_client(struct sockaddr_in *addr) {
  for (u32 i = 0; i < MAX_CLIENTS; i++) {
    Connection *cli = &server.clients[i];
    if (is_connected(cli) && cmp_addr(&cli->addr, addr)) {
      cli->last_seen = 0;
      return true;
    }
  }

  return false;
}

static void cleanup_clients(u64 frame_time) {
  for (u32 i = 0; i < MAX_CLIENTS; i++) {
    Connection *cli = &server.clients[i];
    if (is_connected(cli) && frame_time - cli->last_seen > SERVER_TIMEOUT) {
      printf("Disconnect client %d\n", i);
      cli->last_seen = 0;
    }
  }
}

static void send_message_impl(Message *msg, Connection *cli) {

#if ENABLE_PACKET_LOSS

  if (frand() < PACKET_LOSS_RATE) {
    return;
  }

#endif

  sendto(server.sock, msg, sizeof(Message), 0, (struct sockaddr *)&cli->addr,
         sizeof(cli->addr));
}

static void send_message(Message *msg, Connection *cli) {
  // If there are no queued up priority messages, send
  if (!has_queued_msgs(&cli->queue)) {
    send_message_impl(msg, cli);
  }

  // If the message is priority, queue it until ACKed by client
  if (msg->priority) {
    if (enqueue_msg(&cli->queue, msg)) {
      printf("enqueue priority message %u (%u in queue)\n", msg->seq,
             get_queued_count(&cli->queue));
    } else {
      // The queue is full, fail and disconnect the client
      printf("fatal: queue full, client disconnected\n");
      disconnect_client(&cli->addr);
    }
  }
}

static void broadcast_message(Message *msg, bool priority) {
  msg->seq = server.seq++;
  msg->priority = priority;

  for (u32 i = 0; i < MAX_CLIENTS; i++) {
    Connection *cli = &server.clients[i];
    if (is_connected(cli))
      send_message(msg, cli);
  }
}

static bool recv_message() {
  u32 size = sizeof(struct sockaddr);
  i32 recvd_bytes = recvfrom(server.sock, &server.recvd_msg, sizeof(Message), 0,
                             (struct sockaddr *)&server.sender_addr, &size);
  return recvd_bytes > 0;
}

void server_update(Game game) {
  u64 frame_time = now();

  // Server heartbeat
  if (frame_time - server.last_heartbeat_sent > SERVER_HB_INTERVAL) {
    Message heartbeat = {.type = MSG_HEARTBEAT};
    broadcast_message(&heartbeat, false);
    server.last_heartbeat_sent = frame_time;
  }

  if (game->state == GS_MAIN_MENU) {
    if (!server.main_menu) {
      Message reset = {
          .type = MSG_RESET,
      };
      broadcast_message(&reset, true);
    }
    server.main_menu = true;
  } else {
    server.main_menu = false;
  }

  // Send updates
  Message game_state = {
      .type = MSG_GAME_STATE,
      .game =
          (GameStateData){
              .state = game->state,
              .total_time = game->total_time,
              .delta_time = game->delta_time,
              .boss_idx = game->boss_idx,
              .boss_timer = game->boss_timer,
              .score = game->score,
              .player_state = game->player->player,
              .menu_layout = game->menu_layout,
              .menu_n_options = game->menu_n_options,
              .menu_selected_option = game->menu_selected_option,
          },
  };
  broadcast_message(&game_state, false);

  if (game->state == GS_RUNNING) {
    u32 change_count;
    EntityListChange *changes = el_get_changes(game->world, &change_count);
    el_flush_changes(game->world);

    for (u32 i = 0; i < change_count; i += CHANGE_ENT_COUNT) {
      u32 local_count = min(change_count - i, CHANGE_ENT_COUNT);
      Message change = {
          .type = MSG_CHANGES,
          .changes = (ChangeData){.changes = {0}, .count = local_count},
      };
      for (u32 j = 0; j < local_count; j++) {
        change.changes.changes[j] = changes[i + j];
      }
      broadcast_message(&change, true);
    }

    u32 count = el_size(game->world);
    for (u32 i = 0; i < count; i += MOVE_ENT_COUNT) {
      u32 local_count = min(count - i, MOVE_ENT_COUNT);
      Message move = {
          .type = MSG_MOVE,
          .move = (MoveData){.entities = {0}, .count = local_count},
      };
      for (u32 j = 0; j < local_count; j++) {
        move.move.entities[j] = (EntityMoveData){
            .idx = i + j,
            .position = el_get(game->world, i + j)->position,
        };
      }
      broadcast_message(&move, false);
    }

    Message update = {
        .type = MSG_UPDATE,
        .updates = (UpdateData){.updates = {0}, .count = 0},
    };
    count = 0;
    for (u32 i = 0; i < el_size(game->world); i++) {
      Entity *ent = el_get(game->world, i);
      if (ent->type == ENT_ENEMY) {
        update.updates.updates[count++] = (EntityUpdateData){
            .idx = i,
            .enemy = ent->enemy,
        };
        if (count == UPDATE_ENT_COUNT) {
          update.updates.count = count;
          broadcast_message(&update, false);
          count = 0;
        }
      }
    }
    if (count > 0) {
      update.updates.count = count;
      broadcast_message(&update, false);
    }
  }

  // Receive messages
  while (recv_message()) {
    Connection *sender = find_client(&server.sender_addr);
    if (sender) {
      sender->last_seen = frame_time;
    } else if (server.recvd_msg.type != MSG_HELLO) {
      continue;
    }

    printf("Received: ");
    switch (server.recvd_msg.type) {
    case MSG_HELLO:
      printf("client\n");
      connect_client(&server.sender_addr);
      break;
    case MSG_GOODBYE:
      printf("bye\n");
      disconnect_client(&server.sender_addr);
      break;
    case MSG_HEARTBEAT:
      printf("heartbeat\n");
      break;
    case MSG_ACK:
      if (ack_msg(&sender->queue, server.recvd_msg.seq) < 0) {
        // Client ACKed a message that it shouldn't have received yet
        // Something is very wrong, disconnect the client
        disconnect_client(&server.sender_addr);
      }
      break;
    default:
      printf("something else; ignored\n");
      break;
    }
  }

  // Resend queued messages until ACKed
  for (u32 i = 0; i < MAX_CLIENTS; i++) {
    Connection *cli = &server.clients[i];
    if (is_connected(cli) && has_queued_msgs(&cli->queue)) {
      // Resend message, do it as non-priority so it won't get re-queued
      printf("resending message %u (%u in queue)\n", peek_msg(&cli->queue)->seq,
             get_queued_count(&cli->queue));
      send_message_impl(peek_msg(&cli->queue), cli);
    }
  }

  // Timeout clients
  cleanup_clients(frame_time);
}

void server_start_game(Game game) {
  Message reset = {
      .type = MSG_RESET,
  };
  broadcast_message(&reset, true);
}

void server_end_game(Game game) {
  Message end_game = {
      .type = MSG_END_GAME,
  };
  broadcast_message(&end_game, true);
}

void server_shutdown() {
  Message goodbye = {.type = MSG_GOODBYE};
  broadcast_message(&goodbye, true);

  close(server.sock);
}
