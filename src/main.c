#include <raylib.h>
#include <raymath.h>
#include <stdio.h>

#include "config.h"

void init();
void frame();
void cleanup();

int main() {
  init();

  while (!WindowShouldClose()) {
    frame();
  }

  cleanup();

  return 0;
}

Vector2 center = {400, 300};
Vector2 offset = {200, 0};

float angle = 0.0f;

void init() {
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "ArcadeShooter");
  SetTargetFPS(TARGET_FPS);
}

void frame() {
  angle = GetTime();

  BeginDrawing();
  ClearBackground(BLACK);
  DrawText("Hello world", 20, 20, 20, WHITE);

  Vector2 v1 = Vector2Add(center, Vector2Rotate(offset, angle));
  Vector2 v2 = Vector2Add(center, Vector2Rotate(offset, angle - 2 * PI / 3));
  Vector2 v3 = Vector2Add(center, Vector2Rotate(offset, angle - 4 * PI / 3));

  DrawTriangle(v1, v2, v3, BLUE);

  EndDrawing();
}

void cleanup() { CloseWindow(); }
