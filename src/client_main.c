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

void init();
void cleanup();

int main() {
  init();

  Game game = game_init();
  while (!WindowShouldClose()) {
    if (client_update(game) < 0)
      break;

    game_update_client(game);
    game_draw(game);
  }
  game_end(game);

  cleanup();

  return 0;
}

void init() {
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "ArcadeShooterClient");
  SetTargetFPS(TARGET_FPS);

  // HideCursor();
  // SetExitKey(0); // Stop "esc" key from immediately quitting the game

  load_assets();

  client_init();
  client_connect("127.0.0.1");
}

void cleanup() {
  client_shutdown();
  unload_assets();

  CloseWindow();
}
