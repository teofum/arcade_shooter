#include <arpa/inet.h>
#include <fcntl.h>
#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <unistd.h>

#include "assets.h"
#include "client.h"
#include "config.h"
#include "game.h"
#include "network_shared.h"
#include "ui.h"

void init();
void cleanup();

int main() {
  init();

  Game game = game_init();
  game->client.menu_state = MS_TITLE;
  while (!WindowShouldClose() && game->state != GS_QUIT) {
    while (!client_is_connected() && !WindowShouldClose() &&
           game->state != GS_QUIT) {
      ui_draw_main_menu(game);
      if (game->client.should_start_server) {
        client_host(game);
      }
    }
    while (client_is_connected() && !WindowShouldClose() &&
           game->state != GS_QUIT) {
      game_process_input(game);

      game_update_client(game);
      if (client_update(game) < 0)
        break;

      game_draw(game);
    }
  }
  game_end(game);

  cleanup();

  return 0;
}

void init() {
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "ArcadeShooterClient");
  SetTargetFPS(TARGET_FPS);

  HideCursor();
  SetExitKey(0); // Stop "esc" key from immediately quitting the game
  InitAudioDevice();

  load_assets();

  client_init();
}

void cleanup() {
  client_shutdown();
  unload_assets();

  CloseAudioDevice();
  CloseWindow();
}
