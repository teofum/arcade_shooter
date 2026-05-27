#ifndef SERVER_H
#define SERVER_H

#include <arpa/inet.h>
#include <raylib.h>
#include <sys/time.h>

#include "game.h"
#include "types.h"

#define MAX_CLIENTS 4

#define SERVER_HB_INTERVAL 1000
#define SERVER_TIMEOUT 10000

i32 server_init();
void server_shutdown();

void server_start_game(Game game);
void server_update(Game game);
void server_end_game(Game game);

#endif
