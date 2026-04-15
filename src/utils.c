#include <stdlib.h>

#include "config.h"
#include "utils.h"

#define SCALE ((float)WINDOW_HEIGHT / FIELD_HEIGHT)
#define OFFSET_X ((float)WINDOW_WIDTH / 2)
#define OFFSET_Y ((float)WINDOW_HEIGHT / 2)

f32 frand() { return (f32)rand() / RAND_MAX; }

f32 game_to_screen_x(f32 x) { return x * SCALE + OFFSET_X; }

f32 game_to_screen_y(f32 y) { return y * SCALE + OFFSET_Y; }

f32 game_to_screen_scale(f32 x) { return x * SCALE; }

Vector2 game_to_screen(Vector2 p) {
  return (Vector2){p.x * SCALE + OFFSET_X, p.y * SCALE + OFFSET_Y};
}

Vector2 screen_to_game(Vector2 p) {
  return (Vector2){(p.x - OFFSET_X) / SCALE, (p.y - OFFSET_Y) / SCALE};
}

Rectangle game_to_screen_rect(Rectangle rect) {
  return (Rectangle){
      rect.x * SCALE + OFFSET_X,
      rect.y * SCALE + OFFSET_Y,
      rect.width * SCALE,
      rect.height * SCALE,
  };
}

/*
 * Damage calculation
 */
i32 get_damage(i32 base_damage) {
  i32 variation = base_damage > 10 ? base_damage / 5 : 2;
  return base_damage - variation / 2 + rand() % (variation + 1);
}

Range get_damage_range(i32 base_damage) {
  i32 variation = base_damage > 10 ? base_damage / 5 : 2;
  i32 min = base_damage - variation / 2;

  return (Range){min, min + variation};
}
