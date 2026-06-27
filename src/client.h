#ifndef CLIENT_H
#define CLIENT_H

#include <arpa/inet.h>
#include <raylib.h>

#include "game.h"
#include "types.h"

#define CLIENT_HB_INTERVAL 1000
#define CLIENT_TIMEOUT 10000

i32 client_init();

i32 client_connect(const char *address, Game game);

i32 client_update(Game game);

void client_disconnect();

void client_shutdown();

bool client_is_connected();

#endif
