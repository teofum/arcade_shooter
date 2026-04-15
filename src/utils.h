#ifndef UTILS_H
#define UTILS_H

#include <raylib.h>

#include "types.h"

typedef struct Range {
  i32 min;
  i32 max;
} Range;

f32 game_to_screen_x(f32 x);
f32 game_to_screen_y(f32 y);
f32 game_to_screen_scale(f32 x);

Vector2 game_to_screen(Vector2 p);
Vector2 screen_to_game(Vector2 p);

Rectangle game_to_screen_rect(Rectangle rect);

i32 get_damage(i32 base_damage);
Range get_damage_range(i32 base_damage);

#endif
