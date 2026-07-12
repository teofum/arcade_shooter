#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "bullet.h"
#include "client.h"
#include "config.h"
#include "dmg_number.h"
#include "enemy.h"
#include "enemy_bullet.h"
#include "entity.h"
#include "entity_list.h"
#include "explosion.h"
#include "game.h"
#include "laser.h"
#include "network_shared.h"
#include "player.h"
#include "powerup.h"
#include "types.h"
#include "utils.h"
#include "xp_gem.h"

typedef struct ClientState {
  i32 sock;
  u64 last_heartbeat_sent;
  u32 seq;

  Connection server;

  Message recvd_msg;
} ClientState;

static ClientState client = {0};

i32 client_init() {
  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
    perror("Failed to create socket");
    return -1;
  }
  fcntl(sock, F_SETFL, fcntl(sock, F_GETFL) | O_NONBLOCK);

  client = (ClientState){
      .sock = sock,
      .last_heartbeat_sent = 0,
      .seq = 0,
      .server = {0},
      .recvd_msg = {0},
  };

  return 0;
}

void send_message_impl(Message *msg) {

#if ENABLE_PACKET_LOSS

  // Don't lose hello messages, it's only annoying for testing
  if (msg->type != MSG_HELLO && frand() < PACKET_LOSS_RATE) {
    return;
  }

#endif

  sendto(client.sock, msg, sizeof(Message), 0,
         (struct sockaddr *)&client.server.addr, sizeof(client.server.addr));
}

static i32 send_message(Message *msg, bool priority) {
  msg->seq = ++client.seq;
  msg->priority = priority;

  // If there are no queued up priority messages, send
  if (!has_queued_msgs(&client.server.queue)) {
    send_message_impl(msg);
  }

  // If the message is priority, queue it until ACKed by client
  if (msg->priority) {
    if (enqueue_msg(&client.server.queue, msg)) {
      printf("enqueue priority message %u (%u in queue)\n", msg->seq,
             get_queued_count(&client.server.queue));
    } else {
      // The queue is full, fail and disconnect the client
      printf("Disconnected from server: message queue full\n");
      return -1;
    }
  }

  return 0;
}

static bool recv_message() {
  i32 recvd_bytes = recv(client.sock, &client.recvd_msg, sizeof(Message), 0);
  return recvd_bytes > 0;
}

i32 client_connect(const char *address, Game game) {
  client.server = (Connection){
      .addr =
          (struct sockaddr_in){
              .sin_family = AF_INET,
              .sin_port = htons(SERVER_PORT),
          },
      .last_seen = now(),
      .queue = {0},
      .last_seq_recvd = 0,
  };
  inet_pton(AF_INET, address, &client.server.addr.sin_addr);
  client.seq = 0;

  printf("Connecting to server at %s...\n", address);

  Message greeting = {.type = MSG_HELLO};
  send_message(&greeting, false);

  u64 connect_time = now();
  bool connected = false;
  game->client.conn_state = CS_CONNECTING;
  while (now() - connect_time < CONNECTION_TIMEOUT && !connected) {
    if (recv_message() && client.recvd_msg.type == MSG_HELLO) {
      if (client.recvd_msg.priority) {
        Message ack = {.type = MSG_ACK, .seq = client.recvd_msg.seq};
        send_message_impl(&ack);
      }

      client.server.last_seq_recvd = client.recvd_msg.seq;

      game->client.local_player_idx = client.recvd_msg.hello.player_idx;
      printf("i am player %u\n", game->client.local_player_idx);

      // If we joined a game in progress
      if (client.recvd_msg.hello.game_running) {
        // Start game and temporarily set state to main menu until entities are
        // synced
        game_reset(game);
        game->state = GS_MAIN_MENU;

        // Sync player data
        for (u32 i = 0; i < MAX_CLIENTS; i++) {
          game->player_type[i] = client.recvd_msg.hello.player_type[i];
          if (game->player_type[i] != PLAYER_NONE) {
            game->players[i] =
                el_get(game->world, client.recvd_msg.hello.player_indices[i]);
          }
        }
      }

      connected = true;
    }
  }

  if (connected) {
    printf("Connected\n");
    game->client.conn_state = CS_READY;
    return 0;
  } else {
    printf("Connection failed\n");
    game->client.conn_state = CS_FAILED;
    client.server = (Connection){0};
    return -1;
  }
}

i32 client_update(Game game) {
  u64 frame_time = now();

  // Send heartbeat
  if (frame_time - client.last_heartbeat_sent > CLIENT_HB_INTERVAL) {
    Message heartbeat = {.type = MSG_HEARTBEAT};
    send_message(&heartbeat, false);
    client.last_heartbeat_sent = frame_time;
  }

  if (game->client.should_start_game) {
    game->client.should_start_game = false;
    Message start = {.type = MSG_START_GAME};
    send_message(&start, true);
  }

  if (game->client.should_add_ai) {
    game->client.should_add_ai = false;
    Message add_ai = {.type = MSG_ADD_AI};
    send_message(&add_ai, true);
  }

  if (game->state == GS_RUNNING) {
    // Send input
    Message input = {.type = MSG_INPUT, .input = game->client.input};
    send_message(&input, false);

    // Send level up
    if (game->level_up_option != -1) {
      Message level_up = {
          .type = MSG_LEVEL_UP,
          .level_up = (LevelUpData){.chosen_option = game->level_up_option},
      };
      send_message(&level_up, true);

      game->level_up_option = -1;
    }
  }

  while (recv_message()) {
    client.server.last_seen = frame_time;

    // If we receive a priority message, ACK, even if it is outdated
    // (server might not have received previous ACK)
    if (client.recvd_msg.priority) {
      Message ack = {.type = MSG_ACK, .seq = client.recvd_msg.seq};
      send_message_impl(&ack);
    }

    printf("Received: ");
    if (client.recvd_msg.type != MSG_ACK &&
        client.recvd_msg.seq <= client.server.last_seq_recvd) {
      printf("old data; ignored\n");
      continue;
    }

    if (client.recvd_msg.type != MSG_ACK) {
      client.server.last_seq_recvd = client.recvd_msg.seq;
    }

    switch (client.recvd_msg.type) {
    case MSG_HELLO: {
      printf("redundant hello\n");
      break;
    }
    case MSG_INPUT:
      break;
    case MSG_HEARTBEAT:
      printf("heartbeat\n");
      break;
    case MSG_ACK:
      printf("ACK for message %d\n", client.recvd_msg.seq);
      if (ack_msg(&client.server.queue, client.recvd_msg.seq) < 0) {
        // Server ACKed a message that it shouldn't have received yet
        // Something is very wrong, disconnect
        printf("Disconnected from server: invalid ACK reecived\n");
        return -1;
      }
      break;
    case MSG_GAME_STATE:
      printf("game state\n");
      game_set_state(game, client.recvd_msg.game.state);
      game->total_time = client.recvd_msg.game.total_time;
      game->delta_time = client.recvd_msg.game.delta_time;
      game->boss_timer = client.recvd_msg.game.boss_timer;
      game->boss_idx = client.recvd_msg.game.boss_idx;
      game->score = client.recvd_msg.game.score;
      game->host_player_idx = client.recvd_msg.game.host_player_idx;

      for (u32 i = 0; i < MAX_CLIENTS; i++) {
        if (game->player_type[i] == PLAYER_CLIENT &&
            client.recvd_msg.game.player_type[i] == PLAYER_NONE) {
          printf("player %u left\n", i);
          game->player_type[i] = PLAYER_NONE;
          game->players[i] = NULL;
        }
        if (game->state == GS_MAIN_MENU) {
          game->player_type[i] = client.recvd_msg.game.player_type[i];
        }
      }
      break;
    case MSG_PLAYER_STATE:
      printf("player %u state\n", client.recvd_msg.player.player_idx);
      game->players[client.recvd_msg.player.player_idx]->player =
          client.recvd_msg.player.player_data;
      game->ai_players[client.recvd_msg.player.player_idx] =
          client.recvd_msg.player.ai_data;
      break;
    case MSG_ENTITY_SYNC: {
      EntitySyncData *sync = &client.recvd_msg.entity_sync;
      printf("sync entities %u-%u\n", sync->first,
             sync->first + sync->count - 1);
      for (u32 i = 0; i < sync->count; i++) {
        printf("sync entity %u: %u\n", i + sync->first, sync->entities[i].type);
        el_set(game->world, sync->first + i, &sync->entities[i]);
      }
      break;
    }
    case MSG_MOVE:
      printf("move %u entities\n", client.recvd_msg.move.count);
      for (u32 i = 0; i < client.recvd_msg.move.count; i++) {
        EntityMoveData data = client.recvd_msg.move.entities[i];
        Entity *entity = el_get(game->world, data.idx);
        entity->position = data.position;
        entity->velocity = data.velocity;
      }
      break;
    case MSG_CHANGES:
      printf("change %u entities\n", client.recvd_msg.changes.count);
      for (u32 i = 0; i < client.recvd_msg.changes.count; i++) {
        EntityListChange c = client.recvd_msg.changes.changes[i];
        if (c.type == -1) {
          el_destroy(game->world, c.idx);
        } else {
          Entity *e;
          switch (c.type) {
          case ENT_PLAYER:
            break;
          case ENT_BULLET:
            e = bullet_create(
                c.create_data.position, c.create_data.bullet.target,
                c.create_data.bullet.type, c.create_data.bullet.damage,
                c.create_data.bullet.level, c.create_data.bullet.special_idx,
                NULL);
            break;
          case ENT_WALL:
            break;
          case ENT_ENEMY:
            e = enemy_create(0, 0, c.create_data.enemy.w, c.create_data.enemy.h,
                             c.create_data.enemy.type,
                             c.create_data.enemy.level,
                             c.create_data.enemy.sprite_type);
            break;
          case ENT_DMG_NUMBER:
            e = dmg_number_create(c.create_data.position,
                                  c.create_data.dmg_number.damage,
                                  c.create_data.dmg_number.size);
            break;
          case ENT_XP_GEM:
            e = xp_gem_create(c.create_data.position, c.create_data.xp_value);
            break;
          case ENT_POWERUP:
            e = powerup_create(c.create_data.position,
                               c.create_data.powerup_type);
            break;
          case ENT_EXPLOSION:
            e = explosion_create(c.create_data.position,
                                 c.create_data.explosion.radius,
                                 c.create_data.explosion.damage);
            break;
          case ENT_LASER:
            e = laser_create(c.create_data.position,
                             c.create_data.laser_damage);
            break;
          case ENT_ENEMY_BULLET:
            e = enemy_bullet_create(c.create_data.position,
                                    c.create_data.bullet.target,
                                    c.create_data.bullet.damage);
            break;
          }

          e->position = c.create_data.position;
          e->velocity = c.create_data.velocity;

          el_add(game->world, e);
        }
      }
      el_flush_changes(game->world);
      break;
    case MSG_UPDATE:
      printf("update %u entities\n", client.recvd_msg.updates.count);
      for (u32 i = 0; i < client.recvd_msg.updates.count; i++) {
        EntityUpdateData update = client.recvd_msg.updates.updates[i];
        Entity *ent = el_get(game->world, update.idx);
        if (ent->type == ENT_ENEMY) {
          ent->enemy = update.enemy;
        } else {
          printf("Fatal: received update %u for an entity of type %s\n", i,
                 entity_type_name[ent->type]);
          printf("Entities:\n");
          for (u32 i = 0, j = 0; i < el_size(game->world); i++) {
            Entity *e = el_get(game->world, i);
            printf("  %u: %s\n", j++, entity_type_name[e->type]);
          }
          printf("Updates:\n");
          for (u32 j = 0; j < client.recvd_msg.updates.count; j++) {
            EntityUpdateData update = client.recvd_msg.updates.updates[j];
            printf("  ent %u\n", update.idx);
          }
          assert(false);
        }
      }
      break;
    case MSG_LEVEL_UP:
      game->players[game->client.local_player_idx]->player =
          client.recvd_msg.level_up.player_state;
      game->level_up_menu = true;
      game->level_up_option = -1;
      break;
    case MSG_PLAYER_JOIN: {
      u32 idx = client.recvd_msg.player_change.player_idx;
      printf("player %u joined\n", idx);
      if (idx != game->client.local_player_idx) {
        // If there is no player in this slot create it, otherwise, take over
        // the AI player
        if (game->player_type[idx] == PLAYER_NONE) {
          Entity *new_player = player_create();
          new_player->player.xp = game->players[0]->player.xp;
          game->players[idx] = el_add(game->world, new_player);
        }
        game->player_type[idx] = PLAYER_CLIENT;
      }
      break;
    }
    case MSG_PLAYER_LEAVE: {
      break;
    }
    case MSG_END_GAME:
      game_end(game);
      break;
    case MSG_RESET:
      memcpy(game->player_type, client.recvd_msg.reset.player_type,
             MAX_CLIENTS * sizeof(PlayerType));
      game_reset(game);
      break;
    case MSG_GOODBYE:
      printf("Disconnected from server: goodbye!\n");
      return -1;
    case MSG_START_GAME:
    case MSG_ADD_AI:
      printf("invalid message type; ignored\n");
      break;
    }
  }

  // Resend queued messages until ACKed
  if (has_queued_msgs(&client.server.queue)) {
    // Resend message, do it as non-priority so it won't get re-queued
    printf("resending message %u (%u in queue)\n",
           peek_msg(&client.server.queue)->seq,
           get_queued_count(&client.server.queue));
    send_message_impl(peek_msg(&client.server.queue));
  }

  // Timeout server
  if (is_connected(&client.server) &&
      frame_time - client.server.last_seen > CLIENT_TIMEOUT) {
    printf("Disconnected from server: connection timed out\n");
    return -1;
  }

  return 0;
}

void client_disconnect() {
  Message goodbye = {.type = MSG_GOODBYE};
  send_message(&goodbye, false);

  client.server.last_seen = 0;
}

void client_shutdown() {
  if (client_is_connected()) {
    client_disconnect();
  }

  close(client.sock);
}

bool client_is_connected() { return client.server.last_seen != 0; }

i32 client_host(Game game) {
  game->client.should_start_server = false;
  i32 pid = fork();
  if (pid == -1) {
    return -1;
  }

  if (pid == 0) {
    close(client.sock);

    if (execl("./bin/arcade_shooter", "arcade_shooter", NULL) == -1) {
      perror("execv failed: ");
      assert(false);
    }
  } else {
    usleep(1000000);
    client_connect("127.0.0.1", game);
  }
  return 0;
}
