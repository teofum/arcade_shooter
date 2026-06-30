#include <fcntl.h>
#include <raylib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "config.h"
#include "entity.h"
#include "entity_list.h"
#include "game.h"
#include "network_shared.h"
#include "player.h"
#include "server.h"
#include "utils.h"

typedef struct ServerState {
  i32 sock;
  u64 last_heartbeat_sent;
  u32 seq;

  bool game_running;

  Connection clients[MAX_CLIENTS];
  bool last_players_enabled[MAX_CLIENTS];

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
      .game_running = false,
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

static u32 get_client_idx(struct sockaddr_in *addr) {
  for (u32 i = 0; i < MAX_CLIENTS; i++) {
    Connection *cli = &server.clients[i];
    if (is_connected(cli) && cmp_addr(&cli->addr, addr))
      return i;
  }

  return -1;
}

static i32 connect_client(struct sockaddr_in *addr) {
  for (u32 i = 0; i < MAX_CLIENTS; i++) {
    if (!is_connected(&server.clients[i])) {
      server.clients[i] = (Connection){
          .addr = *addr,
          .last_seen = now(),
      };
      return i;
    }
  }

  return -1;
}

static i32 disconnect_client(struct sockaddr_in *addr, Game game) {
  for (u32 i = 0; i < MAX_CLIENTS; i++) {
    Connection *cli = &server.clients[i];
    if (is_connected(cli) && cmp_addr(&cli->addr, addr)) {
      cli->last_seen = 0;
      if (game != NULL) {
        game_remove_player(game, i);
      }
      return i;
    }
  }

  return -1;
}

static void cleanup_clients(u64 frame_time, Game game) {
  for (u32 i = 0; i < MAX_CLIENTS; i++) {
    Connection *cli = &server.clients[i];
    if (is_connected(cli) && frame_time - cli->last_seen > SERVER_TIMEOUT) {
      printf("Disconnect client %d: connection timed out\n", i);
      cli->last_seen = 0;
      if (game != NULL) {
        game_remove_player(game, i);
      }
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

static void send_message(Message *msg, Connection *cli, Game game) {
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
      disconnect_client(&cli->addr, game);
    }
  }
}

static void broadcast_message(Message *msg, bool priority, Game game) {
  msg->seq = ++server.seq;
  msg->priority = priority;

  for (u32 i = 0; i < MAX_CLIENTS; i++) {
    Connection *cli = &server.clients[i];
    if (is_connected(cli))
      send_message(msg, cli, game);
  }
}

static void send_message_to(Message *msg, Connection *cli, bool priority,
                            Game game) {
  msg->seq = ++server.seq;
  msg->priority = priority;

  send_message(msg, cli, game);
}

static bool recv_message() {
  u32 size = sizeof(struct sockaddr);
  i32 recvd_bytes = recvfrom(server.sock, &server.recvd_msg, sizeof(Message), 0,
                             (struct sockaddr *)&server.sender_addr, &size);
  return recvd_bytes > 0;
}

void server_update(Game game) {
  u64 frame_time = now();

  // Send heartbeat
  if (frame_time - server.last_heartbeat_sent > SERVER_HB_INTERVAL) {
    Message heartbeat = {.type = MSG_HEARTBEAT};
    broadcast_message(&heartbeat, false, game);
    server.last_heartbeat_sent = frame_time;
  }

  // Shutdown server when all players disconnect
  if (game->state != GS_MAIN_MENU) {
    bool players_connected = false;
    for (u32 i = 0; i < MAX_CLIENTS && !players_connected; i++) {
      players_connected |= game->players_enabled[i];
    }

    if (!players_connected) {
      game_set_state(game, GS_QUIT);
    }
  }

  // If host disconnects, make the next player host
  if (game->host_player_idx == -1 ||
      !game->players_enabled[game->host_player_idx]) {
    for (u32 i = 0; i < MAX_CLIENTS; i++) {
      if (game->players_enabled[i]) {
        game->host_player_idx = i;
        printf("Player %u is now host\n", i);
        break;
      }
    }
  }

  // Game reset on game start
  if (game->state == GS_RUNNING) {
    if (!server.game_running) {
      Message reset = {
          .type = MSG_RESET,
          .reset = (ResetData){.players_enabled = {0}},
      };
      memcpy(reset.reset.players_enabled, game->players_enabled,
             MAX_CLIENTS * sizeof(bool));
      broadcast_message(&reset, true, game);
    }
    server.game_running = true;
  } else {
    server.game_running = false;
  }

  // Send updates
  Message game_state = {
      .type = MSG_GAME_STATE,
      .game =
          (GameStateData){
              .state = game->state,
              .players_enabled = {0},
              .host_player_idx = game->host_player_idx,
              .total_time = game->total_time,
              .delta_time = game->delta_time,
              .boss_idx = game->boss_idx,
              .boss_timer = game->boss_timer,
              .score = game->score,
              .menu_layout = game->menu_layout,
              .menu_n_options = game->menu_n_options,
              .menu_selected_option = game->menu_selected_option,
          },
  };
  memcpy(game_state.game.players_enabled, game->players_enabled,
         MAX_CLIENTS * sizeof(bool));
  broadcast_message(&game_state, false, game);

  if (game->state == GS_RUNNING) {
    for (u32 i = 0; i < MAX_CLIENTS; i++) {
      if (game->players_enabled[i]) {
        Message player_state = {
            .type = MSG_PLAYER_STATE,
            .player =
                (PlayerStateData){
                    .player_data = game->players[i]->player,
                    .player_idx = i,
                },
        };
        broadcast_message(&player_state, false, game);
      } else if (server.last_players_enabled[i]) {
        // This player just disconnected...
        Message player_leave = {
            .type = MSG_PLAYER_LEAVE,
            .player_change = (PlayerChangeData){.player_idx = i},
        };
        broadcast_message(&player_leave, true, game);
      }
      server.last_players_enabled[i] = game->players_enabled[i];
    }

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
      broadcast_message(&change, true, game);
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
      broadcast_message(&move, false, game);
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
          broadcast_message(&update, false, game);
          count = 0;
        }
      }
    }
    if (count > 0) {
      update.updates.count = count;
      broadcast_message(&update, false, game);
    }

    for (u32 i = 0; i < MAX_CLIENTS; i++) {
      if (game->players_enabled[i] && game->players[i]->player.leveled_up) {
        game->players[i]->player.leveled_up = false;

        Message level_up = {
            .type = MSG_LEVEL_UP,
            .level_up = (LevelUpData){.player_state = game->players[i]->player},
        };
        send_message_to(&level_up, &server.clients[i], true, game);
      }
    }
  }

  // Receive messages
  while (recv_message()) {
    Connection *sender = find_client(&server.sender_addr);
    u32 sender_idx = get_client_idx(&server.sender_addr);
    if (sender) {
      sender->last_seen = frame_time;
    } else if (server.recvd_msg.type != MSG_HELLO) {
      continue;
    }

    // If we receive a priority message, ACK, even if it is outdated
    // (server might not have received previous ACK)
    if (server.recvd_msg.priority) {
      Message ack = {.type = MSG_ACK, .seq = server.recvd_msg.seq};
      send_message_impl(&ack, sender);
    }

    printf("Received from client %u: ", sender_idx);
    if (server.recvd_msg.type != MSG_ACK && sender &&
        server.recvd_msg.seq <= sender->last_seq_recvd) {
      printf("old data; ignored\n");
      continue;
    }

    if (server.recvd_msg.type != MSG_ACK && sender) {
      sender->last_seq_recvd = server.recvd_msg.seq;
    }

    switch (server.recvd_msg.type) {
    case MSG_HELLO: {
      printf("client\n");
      i32 idx = connect_client(&server.sender_addr);
      if (idx == -1) {
        printf("Connection rejected: too many players\n");
        break;
      }

      game->players_enabled[idx] = true;

      // First player to connect becomes the host
      if (game->host_player_idx == -1) {
        game->host_player_idx = idx;
        printf("Player %u is now host\n", idx);
      }

      Message response = {
          .type = MSG_HELLO,
          .hello =
              (HelloData){
                  .player_idx = idx,
                  .game_running = game->state == GS_RUNNING,
              },
      };
      if (game->state == GS_RUNNING) {
        // Create a new player and give them the same XP as player 1
        Entity *new_player = player_create();
        new_player->player.xp = game->players[0]->player.xp;
        game->players[idx] = el_add(game->world, new_player);
        el_flush_changes(game->world);

        for (u32 i = 0; i < MAX_CLIENTS; i++) {
          response.hello.players_enabled[i] = game->players_enabled[i];
          if (game->players_enabled[i]) {
            response.hello.player_indices[i] =
                el_indexof(game->world, game->players[i]);
          }
        }
      }
      send_message_to(&response, &server.clients[idx], true, game);

      if (game->state == GS_RUNNING) {
        // Sync entities with the new player
        u32 count = el_size(game->world);
        printf("sync %u entities\n", count);
        for (u32 i = 0; i < count; i += SYNC_ENT_COUNT) {
          u32 local_count = min(count - i, SYNC_ENT_COUNT);
          Message sync = {
              .type = MSG_ENTITY_SYNC,
              .entity_sync =
                  (EntitySyncData){
                      .entities = {0},
                      .first = i,
                      .count = local_count,
                  },
          };
          for (u32 j = 0; j < local_count; j++) {
            printf("sync entity %u: %u\n", i + j,
                   el_get(game->world, i + j)->type);
            sync.entity_sync.entities[j] = *el_get(game->world, i + j);
          }
          send_message_to(&sync, &server.clients[idx], true, game);
        }

        // Tell the other players a new player has joined
        Message join = {
            .type = MSG_PLAYER_JOIN,
            .player_change = (PlayerChangeData){.player_idx = idx},
        };
        broadcast_message(&join, true, game);
      }

      break;
    }
    case MSG_GOODBYE: {
      printf("bye\n");
      i32 idx = disconnect_client(&server.sender_addr, game);
      break;
    }
    case MSG_HEARTBEAT:
      printf("heartbeat\n");
      break;
    case MSG_ACK:
      printf("ACK for message %d\n", server.recvd_msg.seq);
      if (ack_msg(&sender->queue, server.recvd_msg.seq) < 0) {
        // Client ACKed a message that it shouldn't have received yet
        // Something is very wrong, disconnect the client
        disconnect_client(&server.sender_addr, game);
      }
      break;
    case MSG_INPUT:
      printf("input\n");
      game->server.input[sender_idx] = server.recvd_msg.input;
      break;
    case MSG_LEVEL_UP: {
      printf("level up\n");
      Player *p = &game->players[sender_idx]->player;
      LevelUpOption *option =
          &p->level_up_options[server.recvd_msg.level_up.chosen_option];

      if (option->type == LU_NEW) {
        SpecialBulletSlot *bullet =
            &p->special_bullets[p->special_bullet_count];

        bullet->fired = false;
        bullet->level = 1;
        bullet->type = option->bullet_type;
        p->special_bullet_count++;
      } else {
        SpecialBulletSlot *bullet = &p->special_bullets[option->bullet_idx];

        bullet->level++;
      }
      break;
    }
    case MSG_START_GAME:
      game_reset(game);
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
  cleanup_clients(frame_time, game);
}

void server_start_game(Game game) {
  Message reset = {
      .type = MSG_RESET,
  };
  broadcast_message(&reset, true, game);
}

void server_end_game(Game game) {
  Message end_game = {
      .type = MSG_END_GAME,
  };
  broadcast_message(&end_game, true, game);
}

void server_shutdown() {
  Message goodbye = {.type = MSG_GOODBYE};
  broadcast_message(&goodbye, true, NULL);

  close(server.sock);
}
