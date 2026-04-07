#include <raylib.h>
#include <raymath.h>
#include <stdio.h>

#include "config.h"
#include "game.h"

void init();
void frame();
void cleanup();

int main() {
  init();

  Game game = game_init();
  while (!WindowShouldClose()) {
    game_process_input(game);
    game_update(game);
    game_draw(game);
  }

  cleanup();

  return 0;
}

void init() {
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "ArcadeShooter");
  SetTargetFPS(TARGET_FPS);

  HideCursor();
}

void cleanup() { CloseWindow(); }
