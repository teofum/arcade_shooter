#include <assert.h>
#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>

#include "bullet.h"
#include "config.h"
#include "game.h"
#include "player.h"
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

static void ui_draw_bar(f32 x, f32 y, f32 w, f32 h, f32 full, const char *text,
                        Color bg_color, Color bar_color, Alignment align_x,
                        Alignment align_y) {
  f32 fill_w = w * full;
  Rectangle bar_rect = ui_align(x, y, w, h, align_x, align_y);

  ui_begin_frame_ex(bar_rect, bg_color, BLACK, (Vector2){0, 0});

  Rectangle fill_rect = ui_align(0, 0, fill_w, h, START, START);
  ui_begin_frame(fill_rect, bar_color);
  ui_text(text, h, WHITE, (Vector2){5, 0}, START, START);
  ui_end_frame();

  ui_end_frame();
}

static void ui_draw_health_bar(PlayerData *pdata) {
  static char health_text[10];
  sprintf(health_text, "%3d/%3d", pdata->health, pdata->max_health);

  ui_draw_bar(0, 20, 200, 20, (f32)pdata->health / pdata->max_health,
              health_text, DARKGRAY, RED, START, END);
}

static void ui_draw_xp_bar(PlayerData *pdata) {
  static char level_text[10];
  sprintf(level_text, "Lv. %d", pdata->level);

  ui_draw_bar(0, 0, 200, 10, (f32)pdata->xp / pdata->to_next_level, level_text,
              DARKGRAY, MAGENTA, START, END);
}

static void ui_draw_ammo_counter(PlayerData *pdata) {
  f32 size = 5;
  f32 x0 = 20 + size;
  f32 x = x0;
  f32 y = WINDOW_HEIGHT - 70 - size;

  for (i32 i = 0; i < pdata->max_ammo; i++) {
    DrawCircle(x, y, size, i < pdata->ammo ? WHITE : DARKGRAY);

    x += size * 2 + 5;
    if (i % 8 == 7) {
      x = x0;
      y -= size * 2 + 5;
    }
  }
}

static void ui_draw_special_ammo(PlayerData *pdata) {
  f32 size = 10;
  f32 x0 = 150 + size;
  f32 x = x0;
  f32 y = WINDOW_HEIGHT - 70 - size;

  for (i32 i = 0; i < pdata->special_bullet_count; i++) {
    SpecialBulletSlot *sb = &pdata->special_bullets[i];

    DrawCircle(x, y, size, DARKGRAY);
    if (!sb->fired) {
      f32 angle = (1 - sb->cooldown / 3.0f) * 360;
      DrawCircleSector((Vector2){x, y}, size, 0, angle, 20,
                       bullet_type_colors[sb->type]);
    }

    x += size * 2 + 5;
  }
}

static void ui_draw_progress(Game game) {
  f32 end_time = BOSS_TIME * N_BOSSES;
  f32 progress =
      (BOSS_TIME * game->boss_idx + BOSS_TIME - game->boss_timer) / end_time;

  ui_begin_frame_ex(ui_align(0, 0, 20, 400, END, CENTER), DARKGRAY, BLACK,
                    (Vector2){0, 0});

  DrawRectangleRec(ui_align(0, 0, 20, 400 * progress, START, END), RED);

  for (int i = 1; i <= 3; i++) {
    f32 w = i == 3 ? 32 : 28;
    f32 h = i == 3 ? 24 : 20;
    Rectangle r = ui_align(0, 400 * i / 3.0f - 10, w, h, CENTER, END);

    DrawRectangleRec(r, game->boss_idx >= i ? RED : DARKGRAY);
    DrawRectangleLines(r.x, r.y, r.width, r.height, BLACK);
  }

  ui_end_frame();
}

void ui_draw_game_ui(Game game) {
  PlayerData *pdata = (PlayerData *)game->player->custom_data;

  ui_begin_frame_ex(full_screen, BLANK, BLANK, (Vector2){20, 20});

  static char score_str[30];
  sprintf(score_str, "Score: %6u", game->score);
  ui_text(score_str, 20, WHITE, (Vector2){0, 0}, END, START);

  ui_draw_health_bar(pdata);
  ui_draw_xp_bar(pdata);
  ui_draw_ammo_counter(pdata);
  ui_draw_special_ammo(pdata);

  ui_draw_progress(game);

  ui_end_frame();
}

static void ui_draw_level_up_option(Game game, LevelUpOption *option, u32 i) {
  PlayerData *pdata = (PlayerData *)game->player->custom_data;
  static char text[30];

  f32 button_x = (i + 1) * 220;

  ui_begin_frame(ui_align(button_x, 0, 200, 300, START, START), BLANK);

  bool clicked =
      ui_button_ex("", 0, (Vector2){0, 0}, (Vector2){200, 300}, START, START);

  BulletType type;
  u32 level;

  if (option->type == LU_NEW) {
    sprintf(text, "New");
    type = option->bullet_type;
    level = 1;
  } else {
    SpecialBulletSlot *bullet = &pdata->special_bullets[option->bullet_idx];
    sprintf(text, "Lv. %d -> %d", bullet->level, bullet->level + 1);
    type = bullet->type;
    level = bullet->level + 1;
  }

  ui_text(bullet_type_names[type], 20, BLACK, (Vector2){0, 10}, CENTER, START);
  ui_text(text, 15, BLACK, (Vector2){0, 40}, CENTER, START);

  DrawRectangleRec(ui_align(0, 70, 60, 60, CENTER, START),
                   bullet_type_colors[type]);

  ui_text(get_bullet_description(game, type, level), 10, BLACK,
          (Vector2){0, 150}, CENTER, START);

  ui_end_frame();

  if (clicked) {
    if (option->type == LU_NEW) {
      SpecialBulletSlot *bullet =
          &pdata->special_bullets[pdata->special_bullet_count];

      bullet->fired = false;
      bullet->level = 1;
      bullet->type = option->bullet_type;
      pdata->special_bullet_count++;
      game->state = GS_RUNNING;
    } else {
      SpecialBulletSlot *bullet = &pdata->special_bullets[option->bullet_idx];

      bullet->level++;
      game->state = GS_RUNNING;
    }
  }
}

void ui_draw_level_up_screen(Game game) {
  PlayerData *pdata = (PlayerData *)game->player->custom_data;

  static char level_up_str[30];
  sprintf(level_up_str, "Lv. %d -> %d", pdata->level - 1, pdata->level);

  ui_begin_frame(full_screen, overlay_bg);

  // Level up title
  ui_begin_frame_ex(
      ui_align_ex(0, -200, WINDOW_WIDTH, 120, START, CENTER, START, END),
      overlay_bg, BLANK, (Vector2){10, 10});
  ui_text("Level up!", 60, WHITE, (Vector2){0, 0}, CENTER, START);
  ui_text(level_up_str, 30, WHITE, (Vector2){0, 70}, CENTER, START);
  ui_end_frame();

  ui_begin_frame(ui_align_ex(0, -160, 860, 400, CENTER, CENTER, CENTER, START),
                 BLANK);

  // Increased stats
  ui_begin_frame_ex(ui_align(0, 0, 200, 300, START, START), overlay_bg, BLANK,
                    (Vector2){10, 10});
  Vector2 cursor = {0, 0};
  if (pdata->leveled_up_stats[STAT_AMMO]) {
    sprintf(level_up_str, "Ammo: %d -> %d", pdata->max_ammo - 1,
            pdata->max_ammo);
    ui_text(level_up_str, 15, WHITE, cursor, START, START);
    cursor.y += 20;
  }
  if (pdata->leveled_up_stats[STAT_DAMAGE]) {
    sprintf(level_up_str, "Damage: %d -> %d", pdata->base_damage - 1,
            pdata->base_damage);
    ui_text(level_up_str, 15, WHITE, cursor, START, START);
    cursor.y += 20;
  }
  if (pdata->leveled_up_stats[STAT_HEALTH]) {
    sprintf(level_up_str, "Health: %d -> %d", pdata->max_health - 10,
            pdata->max_health);
    ui_text(level_up_str, 15, WHITE, cursor, START, START);
    cursor.y += 20;
  }
  if (pdata->leveled_up_stats[STAT_MOVEMENT]) {
    sprintf(level_up_str, "Move speed: %.0f -> %.0f", pdata->move_speed - 5,
            pdata->move_speed);
    ui_text(level_up_str, 15, WHITE, cursor, START, START);
    cursor.y += 20;
  }
  ui_end_frame();

  // Level up options
  for (u32 i = 0; i < LEVEL_UP_OPTIONS; i++) {
    LevelUpOption *option = pdata->level_up_options[i];
    if (!option)
      break;

    ui_draw_level_up_option(game, option, i);
  }

  ui_end_frame();

  ui_end_frame();
}

void ui_draw_pause_screen(Game game) {
  ui_begin_frame(full_screen, overlay_bg);

  ui_text("Paused", 60, WHITE, (Vector2){0, -60}, CENTER, CENTER);

  if (ui_button_ex("Resume", 20, (Vector2){0, 20}, (Vector2){200, 0}, CENTER,
                   CENTER)) {
    game->state = GS_RUNNING;
  }
  if (ui_button_ex("Main menu", 20, (Vector2){0, 60}, (Vector2){200, 0}, CENTER,
                   CENTER)) {
    game_reset(game);
  }
  if (ui_button_ex("Quit", 20, (Vector2){0, 100}, (Vector2){200, 0}, CENTER,
                   CENTER)) {
    game->state = GS_QUIT;
  }

  ui_end_frame();
}

void ui_draw_game_over_screen(Game game) {
  static char score_str[30];
  sprintf(score_str, "Score: %u", game->score);

  ui_begin_frame(full_screen, overlay_bg);

  ui_text("You Died", 60, WHITE, (Vector2){0, -60}, CENTER, CENTER);
  ui_text(score_str, 20, WHITE, (Vector2){0, -20}, CENTER, CENTER);

  if (ui_button_ex("Main menu", 20, (Vector2){0, 20}, (Vector2){200, 0}, CENTER,
                   CENTER)) {
    game_reset(game);
  }
  if (ui_button_ex("Quit", 20, (Vector2){0, 60}, (Vector2){200, 0}, CENTER,
                   CENTER)) {
    game->state = GS_QUIT;
  }

  ui_end_frame();
}
