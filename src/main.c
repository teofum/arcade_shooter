#include <raylib.h>
#include <raymath.h>
#include <stdio.h>

#include "assets.h"
#include "config.h"
#include "game.h"
#include "server.h"
#include "ui.h"

void init();
void cleanup();

int main() {
  init();

  Game game = game_init();
  while (!WindowShouldClose() && game->state != GS_QUIT) {
    game_update(game);
    server_update(game);

    BeginDrawing();
    ClearBackground((Color){0, 128, 128, 255});

    if (game->state == GS_MAIN_MENU) {
      ui_draw_main_menu(game);
    }
    EndDrawing();
  }
  game_end(game);

  cleanup();

  return 0;
}

void init() {
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "ArcadeShooter");
  SetTargetFPS(TARGET_FPS);

  HideCursor();
  SetExitKey(0); // Stop "esc" key from immediately quitting the game

  InitAudioDevice();

  load_assets();

  server_init();
}

void cleanup() {
  server_shutdown();
  unload_assets();

  CloseAudioDevice();
  CloseWindow();
}
