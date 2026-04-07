#include <raylib.h>
#include <raymath.h>
#include <stdio.h>

void frame();

float angle = 0.0f;

int main() {
  InitWindow(800, 600, "Raylib Example");

  SetTargetFPS(120);
  while (!WindowShouldClose()) {
    angle = GetTime();
    frame();
  }

  CloseWindow();

  return 0;
}

Vector2 center = {400, 300};
Vector2 offset = {200, 0};

void frame() {
  BeginDrawing();
  ClearBackground(BLACK);
  DrawText("Hello world", 20, 20, 20, WHITE);

  Vector2 v1 = Vector2Add(center, Vector2Rotate(offset, angle));
  Vector2 v2 = Vector2Add(center, Vector2Rotate(offset, angle - 2 * PI / 3));
  Vector2 v3 = Vector2Add(center, Vector2Rotate(offset, angle - 4 * PI / 3));

  DrawTriangle(v1, v2, v3, BLUE);

  EndDrawing();
}
