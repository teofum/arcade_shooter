#ifndef UI_H
#define UI_H

#include <raylib.h>

#include "game.h"
#include "player.h"

typedef enum {
  START = 0,
  CENTER = 1,
  END = 2,
} Alignment;

// Frames
void ui_begin_frame(Rectangle rect, Color bg_color);
void ui_begin_frame_ex(Rectangle rect, Color bg_color, Color border_color,
                       Vector2 padding);

void ui_end_frame();

// UI alignment helpers
Rectangle ui_align(f32 x, f32 y, f32 w, f32 h, Alignment align_x,
                   Alignment align_y);
Rectangle ui_align_v(Vector2 position, Vector2 size, Alignment align_x,
                     Alignment align_y);
Rectangle ui_align_r(Rectangle rect, Alignment align_x, Alignment align_y);

Rectangle ui_align_ex(f32 x, f32 y, f32 w, f32 h, Alignment align_x,
                      Alignment align_y, Alignment origin_x,
                      Alignment origin_y);
Rectangle ui_align_ex_v(Vector2 position, Vector2 size, Alignment align_x,
                        Alignment align_y, Alignment origin_x,
                        Alignment origin_y);
Rectangle ui_align_ex_r(Rectangle rect, Alignment align_x, Alignment align_y,
                        Alignment origin_x, Alignment origin_y);

// UI components
bool ui_button(const char *text, f32 font_size, Vector2 position);

bool ui_button_ex(const char *text, f32 font_size, Vector2 position,
                  Vector2 size, Alignment align_x, Alignment align_y);

void ui_text(const char *text, f32 font_size, Color color, Vector2 position,
             Alignment align_x, Alignment align_y);

void ui_draw_health_bar(PlayerData *pdata);
void ui_draw_xp_bar(PlayerData *pdata);
void ui_draw_ammo_counter(PlayerData *pdata);
void ui_draw_game_ui(Game game);

#endif
