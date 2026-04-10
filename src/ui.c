#include <assert.h>
#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "game.h"
#include "player.h"
#include "types.h"
#include "ui.h"

#define BUTTON_PADDING 5

// Internal UI drawing state
typedef struct UiFrame {
  Rectangle bounds;
  struct UiFrame *parent;
} UiFrame;

static UiFrame *current_frame = NULL;

// UI frames
void ui_begin_frame(Rectangle rect, Color bg_color) {
  ui_begin_frame_ex(rect, bg_color, BLANK, (Vector2){0, 0});
}

void ui_begin_frame_ex(Rectangle rect, Color bg_color, Color border_color,
                       Vector2 padding) {
  UiFrame *new_frame = malloc(sizeof(UiFrame));

  DrawRectangleRec(rect, bg_color);
  DrawRectangleLines(rect.x, rect.y, rect.width, rect.height, border_color);

  rect.x += padding.x;
  rect.y += padding.y;
  rect.width -= padding.x * 2;
  rect.height -= padding.y * 2;

  new_frame->bounds = rect;
  new_frame->parent = current_frame;
  current_frame = new_frame;
}

void ui_end_frame() {
  assert(current_frame != NULL);

  UiFrame *parent = current_frame->parent;
  free(current_frame);

  current_frame = parent;
}

// UI alignment helpers
Rectangle ui_align(f32 x, f32 y, f32 w, f32 h, Alignment align_x,
                   Alignment align_y) {
  return ui_align_ex(x, y, w, h, align_x, align_y, align_x, align_y);
}

Rectangle ui_align_v(Vector2 position, Vector2 size, Alignment align_x,
                     Alignment align_y) {
  return ui_align_ex_v(position, size, align_x, align_y, align_x, align_y);
}

Rectangle ui_align_r(Rectangle rect, Alignment align_x, Alignment align_y) {
  return ui_align_ex_r(rect, align_x, align_y, align_x, align_y);
}

Rectangle ui_align_ex(f32 x, f32 y, f32 w, f32 h, Alignment align_x,
                      Alignment align_y, Alignment origin_x,
                      Alignment origin_y) {
  assert(current_frame != NULL);

  Rectangle fb = current_frame->bounds;

  // Abuse enum values
  x += fb.x + fb.width * align_x / 2 - w * origin_x / 2;
  y += fb.y + fb.height * align_y / 2 - h * origin_y / 2;

  return (Rectangle){x, y, w, h};
}

Rectangle ui_align_ex_v(Vector2 position, Vector2 size, Alignment align_x,
                        Alignment align_y, Alignment origin_x,
                        Alignment origin_y) {
  return ui_align_ex(position.x, position.y, size.x, size.y, align_x, align_y,
                     origin_x, origin_y);
}

Rectangle ui_align_ex_r(Rectangle rect, Alignment align_x, Alignment align_y,
                        Alignment origin_x, Alignment origin_y) {
  return ui_align_ex(rect.x, rect.y, rect.width, rect.height, align_x, align_y,
                     origin_x, origin_y);
}

bool ui_button(const char *text, f32 font_size, Vector2 position) {
  return ui_button_ex(text, font_size, position, (Vector2){0, 0}, START, START);
}

bool ui_button_ex(const char *text, f32 font_size, Vector2 position,
                  Vector2 size, Alignment align_x, Alignment align_y) {
  f32 text_width = MeasureText(text, font_size);

  // Autosize
  if (size.x == 0) {
    size.x = text_width + BUTTON_PADDING * 2;
  }
  if (size.y == 0) {
    size.y = font_size + BUTTON_PADDING * 2;
  }

  Rectangle rect = ui_align_v(position, size, align_x, align_y);

  bool hovered = CheckCollisionPointRec(GetMousePosition(), rect);
  bool clicked = hovered && IsMouseButtonDown(MOUSE_BUTTON_LEFT);

  Color color = ColorLerp(WHITE, BLUE, clicked ? 0.4 : hovered ? 0.2 : 0);
  ui_begin_frame_ex(rect, color, BLACK,
                    (Vector2){BUTTON_PADDING, BUTTON_PADDING});
  Rectangle text_rect = ui_align(0, 0, text_width, font_size, CENTER, CENTER);
  DrawText(text, text_rect.x, text_rect.y, font_size, BLACK);
  ui_end_frame();

  return hovered && IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
}

void ui_text(const char *text, f32 font_size, Color color, Vector2 position,
             Alignment align_x, Alignment align_y) {
  f32 text_width = MeasureText(text, font_size);
  Rectangle text_rect =
      ui_align(position.x, position.y, text_width, font_size, align_x, align_y);
  DrawText(text, text_rect.x, text_rect.y, font_size, color);
}

void ui_draw_health_bar(PlayerData *pdata) {
  static char health_text[10];

  f32 w = 200;
  f32 h = 20;
  f32 x = 20;
  f32 y = WINDOW_HEIGHT - 40 - h;

  f32 relative_health = (f32)pdata->health / pdata->max_health;

  DrawRectangle(x, y, w, h, DARKGRAY);
  DrawRectangle(x, y, relative_health * w, h, RED);
  DrawRectangleLines(x, y, w, h, BLACK);

  sprintf(health_text, "%3d/%3d", pdata->health, pdata->max_health);
  DrawText(health_text, x + 5, y, 20, WHITE);
}

void ui_draw_xp_bar(PlayerData *pdata) {
  static char level_text[10];

  f32 w = 200;
  f32 h = 10;
  f32 x = 20;
  f32 y = WINDOW_HEIGHT - 20 - h;

  f32 relative_xp = (f32)pdata->xp / pdata->to_next_level;

  DrawRectangle(x, y, w, h, DARKGRAY);
  DrawRectangle(x, y, relative_xp * w, h, MAGENTA);
  DrawRectangleLines(x, y, w, h, BLACK);

  sprintf(level_text, "Lv. %d", pdata->level);
  DrawText(level_text, x + 5, y, 10, WHITE);
}

void ui_draw_ammo_counter(PlayerData *pdata) {
  f32 size = 5;
  f32 x0 = 20 + size;
  f32 x = x0;
  f32 y = WINDOW_HEIGHT - 70 - size;

  for (i32 i = 0; i < pdata->max_ammo; i++) {
    DrawCircle(x, y, size, i < pdata->ammo ? BLUE : DARKGRAY);

    x += size * 2 + 5;
    if (i % 8 == 7) {
      x = x0;
      y -= size * 2 + 5;
    }
  }
}

void ui_draw_game_ui(Game game) {
  PlayerData *pdata = (PlayerData *)game->player->custom_data;

  ui_draw_health_bar(pdata);
  ui_draw_xp_bar(pdata);
  ui_draw_ammo_counter(pdata);
}
