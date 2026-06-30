#include <unistd.h>

#include "config.h"
#include "game.h"
#include "server.h"
#include "utils.h"

void init();
void cleanup();

int main() {
  init();

  Game game = game_init();
  game->host_player_idx = -1;

  while (game->state != GS_QUIT) {
    u64 frame_start = now();

    game_update(game);
    server_update(game);

    u64 frame_time = now() - frame_start;
    if (frame_time < SERVER_TICK_MS) {
      usleep(1000 * (SERVER_TICK_MS - frame_time));
    }
  }
  game_end(game);

  cleanup();

  return 0;
}

void init() { server_init(); }

void cleanup() { server_shutdown(); }
