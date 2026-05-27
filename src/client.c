#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>

#include "client.h"
#include "entity_list.h"
#include "game.h"
#include "network_shared.h"
#include "types.h"
#include "utils.h"

typedef struct ClientState {
  i32 sock;
  u64 last_heartbeat_sent;
  u32 last_seq;

  Message recvd_msg;
  struct sockaddr_in serv_addr;
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
      .recvd_msg = {0},
  };

  return 0;
}

void send_message(Message *msg) {
  sendto(client.sock, msg, sizeof(Message), 0,
         (struct sockaddr *)&client.serv_addr, sizeof(client.serv_addr));
}

bool recv_message() {
  i32 recvd_bytes = recv(client.sock, &client.recvd_msg, sizeof(Message), 0);
  return recvd_bytes > 0;
}

i32 client_connect(const char *address) {
  client.serv_addr = (struct sockaddr_in){
      .sin_family = AF_INET,
      .sin_port = htons(SERVER_PORT),
  };
  inet_pton(AF_INET, address, &client.serv_addr.sin_addr);

  printf("Connecting to server at %s...\n", address);

  Message greeting = {.type = MSG_HELLO};
  send_message(&greeting);

  printf("Connected\n");

  return 0;
}

i32 client_update(Game game) {
  u64 frame_time = now();

  // Send heartbeat
  if (frame_time - client.last_heartbeat_sent > CLIENT_HB_INTERVAL) {
    Message heartbeat = {.type = MSG_HEARTBEAT};
    send_message(&heartbeat);
    client.last_heartbeat_sent = frame_time;
  }

  while (recv_message()) {
    printf("Received: ");

    if (client.recvd_msg.seq <= client.last_seq) {
      printf("old data; ignored\n");
      continue;
    } else {
      client.last_seq = client.recvd_msg.seq;
    }

    switch (client.recvd_msg.type) {
    case MSG_HEARTBEAT:
      printf("heartbeat\n");
      break;
    case MSG_GAME_STATE:
      printf("game state %d\n", client.recvd_msg.game_state);
      game->state = client.recvd_msg.game_state;
      break;
    case MSG_MOVE:
      for (u32 i = 0; i < client.recvd_msg.move.count; i++) {
        EntityMoveData ent = client.recvd_msg.move.entities[i];
        printf("move [%u] x=%f, y=%f\n", ent.idx, ent.position.x,
               ent.position.y);
        el_get(game->world, ent.idx)->position = ent.position;
      }
      break;
    case MSG_GOODBYE:
      printf("Goodbye!\n");
      return -1;
    default:
      printf("\n");
      break;
    }
  }

  return 0;
}

void client_disconnect() {
  Message goodbye = {.type = MSG_GOODBYE};
  send_message(&goodbye);
}

void client_shutdown() {
  client_disconnect();

  close(client.sock);
}
