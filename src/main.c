#include <raylib.h>
#include <raymath.h>
#include <stdio.h>

#include "assets.h"
#include "config.h"
#include "game.h"

void init();
void frame();
void cleanup();

int main() {
  init();

  Game game = game_init();
  while (!WindowShouldClose() && game->state != GS_QUIT) {
    game_process_input(game);
    game_update(game);
    game_draw(game);
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
}

void cleanup() {
  unload_assets();

  CloseAudioDevice();
  CloseWindow();
}
