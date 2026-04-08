#ifndef UTILS_H
#define UTILS_H

#include <raylib.h>

#include "types.h"

f32 game_to_screen_x(f32 x);
f32 game_to_screen_y(f32 y);
f32 game_to_screen_scale(f32 x);

Vector2 game_to_screen(Vector2 p);
Vector2 screen_to_game(Vector2 p);

Rectangle game_to_screen_rect(Rectangle rect);

#endif
