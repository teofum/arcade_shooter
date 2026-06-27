#include <assert.h>
#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "types.h"
#include "ui.h"

#define BUTTON_PADDING 5

// Constants
static Color overlay_bg = {0, 0, 0, 128};
static Rectangle full_screen = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};

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

  if (align_x == END)
    x = -x;
  if (align_y == END)
    y = -y;

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

bool ui_button(const char *text, f32 font_size, Vector2 position,
               bool highlight) {
  return ui_button_ex(text, font_size, position, highlight, (Vector2){0, 0},
                      START, START);
}

bool ui_button_ex(const char *text, f32 font_size, Vector2 position,
                  bool highlight, Vector2 size, Alignment align_x,
                  Alignment align_y) {
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
  bool released = hovered && IsMouseButtonReleased(MOUSE_BUTTON_LEFT);

  Color color = (Color){0xc0, 0xc0, 0xc0, 255};
  Color color2 = (Color){0xe0, 0xe0, 0xe0, 255};
  ui_begin_frame_ex(rect, clicked ? WHITE : BLACK, BLANK, (Vector2){0, 0});
  {
    if (highlight) {
      DrawRectangleRec(ui_align(0, 0, size.x + 6, size.y + 6, CENTER, CENTER),
                       RED);

      hovered = true;
      clicked = IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN);
      released = IsGamepadButtonReleased(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN);
    }

    Rectangle inner_rect = ui_align_v((Vector2){0, 0}, size, START, START);
    f32 x0 = inner_rect.x, w = inner_rect.width;
    f32 y0 = inner_rect.y, h = inner_rect.height;

    Rectangle text_rect = ui_align(0, 0, text_width, font_size, CENTER, CENTER);

    DrawRectangle(x0, y0, w - 1, h - 1, clicked ? BLACK : WHITE);
    DrawRectangle(x0 + 1, y0 + 1, w - 2, h - 2, clicked ? color2 : DARKGRAY);
    DrawRectangle(x0 + 1, y0 + 1, w - 3, h - 3, clicked ? DARKGRAY : color2);
    DrawRectangle(x0 + 2, y0 + 2, w - 4, h - 4, color);

    DrawText(text, text_rect.x, text_rect.y, font_size, BLACK);
  }
  ui_end_frame();

  return released;
}

void ui_text(const char *text, f32 font_size, Color color, Vector2 position,
             Alignment align_x, Alignment align_y) {
  f32 text_width = MeasureText(text, font_size);
  Rectangle text_rect =
      ui_align(position.x, position.y, text_width, font_size, align_x, align_y);
  DrawText(text, text_rect.x, text_rect.y, font_size, color);
}

void ui_draw_bar(f32 x, f32 y, f32 w, f32 h, f32 full, const char *text,
                 Color bg_color, Color bar_color, Alignment align_x,
                 Alignment align_y) {
  f32 fill_w = w * full;
  Rectangle bar_rect = ui_align(x, y, w, h, align_x, align_y);

  ui_begin_frame_ex(bar_rect, bg_color, BLACK, (Vector2){0, 0});
  {
    Rectangle fill_rect = ui_align(0, 0, fill_w, h, START, START);
    ui_begin_frame(fill_rect, bar_color);
    {
      ui_text(text, h, WHITE, (Vector2){5, 0}, START, START);
    }
    ui_end_frame();
  }
  ui_end_frame();
}

void ui_text_input(char *text, u32 len, f32 size, f32 x, f32 y, f32 w,
                   bool *focused, Alignment align_x, Alignment align_y) {
  Rectangle rect = ui_align(x, y, w, size + 20, align_x, align_y);

  bool hovered = CheckCollisionPointRec(GetMousePosition(), rect);
  bool clicked = hovered && IsMouseButtonDown(MOUSE_BUTTON_LEFT);
  bool clicked_outside = !hovered && IsMouseButtonDown(MOUSE_BUTTON_LEFT);

  if (clicked && focused != NULL) {
    *focused = true;
  } else if (clicked_outside && focused != NULL) {
    *focused = false;
  }

  if (*focused) {
    char c = 0;
    while ((c = GetCharPressed())) {
      u32 i = strlen(text);
      if (i < len - 1) {
        text[i] = c;
        text[i + 1] = 0;
      }
    }
    if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) {
      u32 i = strlen(text);
      if (i > 0) {
        text[i - 1] = 0;
      }
    }
  }

  ui_begin_frame_ex(rect, BLACK, *focused ? RED : WHITE, (Vector2){10, 10});
  {
    ui_text(text, size, WHITE, (Vector2){0, 0}, START, CENTER);
  }
  ui_end_frame();
}
