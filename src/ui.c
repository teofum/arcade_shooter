#include <assert.h>
#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>

#include "assets.h"
#include "bullet.h"
#include "config.h"
#include "entity.h"
#include "game.h"
#include "powerup.h"
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

static void ui_draw_bar(f32 x, f32 y, f32 w, f32 h, f32 full, const char *text,
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

void ui_draw_main_menu(Game game) {
  ui_begin_frame((Rectangle){0, 0, WINDOW_WIDTH, WINDOW_HEIGHT}, WHITE);
  {
    DrawTexturePro(assets.title_bg, (Rectangle){0, 0, 2160, 1440}, full_screen,
                   (Vector2){0, 0}, 0, WHITE);

    ui_text("Desktop\nDefender", 60, BLACK,
            (Vector2){WINDOW_WIDTH / 2.0f + 100, -148}, START, CENTER);
    ui_text("Desktop\nDefender", 60, RED,
            (Vector2){WINDOW_WIDTH / 2.0f + 100, -150}, START, CENTER);

    bool gp = IsGamepadAvailable(0);
    u32 m = game->menu_selected_option;

    if (ui_button_ex("Start", 20, (Vector2){WINDOW_WIDTH / 2.0f + 100, 0},
                     gp && m == 0, (Vector2){200, 0}, START, CENTER)) {
      game_reset(game);
    }
    if (ui_button_ex("Quit", 20, (Vector2){WINDOW_WIDTH / 2.0f + 100, 40},
                     gp && m == 1, (Vector2){200, 0}, START, CENTER)) {
      game_set_state(game, GS_QUIT);
    }
  }
  ui_end_frame();
}

static void ui_draw_health_bar(Player *pdata) {
  static char health_text[10];
  sprintf(health_text, "%3d/%3d", pdata->health, pdata->max_health);

  ui_draw_bar(0, 20, 200, 20, (f32)pdata->health / pdata->max_health,
              health_text, DARKGRAY, RED, START, END);
}

static void ui_draw_xp_bar(Player *pdata) {
  static char level_text[10];
  sprintf(level_text, "Lv. %d", pdata->level);

  ui_draw_bar(0, 0, 200, 10, (f32)pdata->xp / pdata->to_next_level, level_text,
              DARKGRAY, MAGENTA, START, END);
}

static void ui_draw_ammo_counter(Player *pdata) {
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

static void ui_draw_special_ammo(Player *pdata) {
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

      Sprite *sprite = &assets.bullets[sb->type];
      Rectangle dest = {x, y, 16, 16};

      DrawTexturePro(sprite->texture, get_frame_rect(sprite, 0), dest,
                     (Vector2){8, 8}, 0, WHITE);
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
  {
    DrawRectangleRec(ui_align(0, 0, 20, 400 * progress, START, END), RED);

    for (int i = 1; i <= 3; i++) {
      f32 w = i == 3 ? 32 : 28;
      f32 h = i == 3 ? 24 : 20;
      Rectangle r = ui_align(0, 400 * i / 3.0f - 10, w, h, CENTER, END);

      DrawRectangleRec(r, game->boss_idx >= i ? RED : DARKGRAY);
      DrawRectangleLines(r.x, r.y, r.width, r.height, BLACK);
    }
  }
  ui_end_frame();
}

void ui_draw_powerup(Player *pdata) {
  if (pdata->active_powerup == POWER_NONE)
    return;

  ui_draw_bar(0, 20, 200, 15, pdata->powerup_timer / POWERUP_DURATION, "",
              DARKGRAY, powerup_colors[pdata->active_powerup], START, START);

  ui_begin_frame(ui_align(0, 0, 200, 15, START, START), BLANK);
  {
    ui_text(powerup_names[pdata->active_powerup], 15, WHITE, (Vector2){0, 0},
            START, END);
    ui_text(powerup_descriptions[pdata->active_powerup], 10, WHITE,
            (Vector2){0, 0}, END, END);
  }
  ui_end_frame();
}

static void ui_draw_player_crosshair(Player *pdata) {
  Vector2 pos = pdata->crosshair;

  DrawRectangle(pos.x - 7, pos.y, 15, 1, WHITE);
  DrawRectangle(pos.x, pos.y - 7, 1, 15, WHITE);
}

void ui_draw_game_ui(Game game) {
  Player *pdata = &game->players[0]->player;

  ui_begin_frame_ex(full_screen, BLANK, BLANK, (Vector2){20, 20});
  {
    static char score_str[30];
    sprintf(score_str, "Score: %6u", game->score);
    ui_text(score_str, 20, WHITE, (Vector2){0, 0}, END, START);

    ui_draw_health_bar(pdata);
    ui_draw_xp_bar(pdata);
    ui_draw_ammo_counter(pdata);
    ui_draw_special_ammo(pdata);
    ui_draw_powerup(pdata);

    ui_draw_progress(game);

    ui_draw_player_crosshair(pdata);
  }
  ui_end_frame();
}

static void ui_draw_level_up_option(Game game, LevelUpOption *option, u32 i) {
  Player *pdata = &game->players[0]->player;
  static char text[30];

  f32 button_x = (i + 1) * 220;

  bool gp = IsGamepadAvailable(0);
  u32 m = game->menu_selected_option;
  bool clicked;

  ui_begin_frame(ui_align(button_x, 0, 200, 300, START, START), BLANK);
  {
    clicked = ui_button_ex("", 0, (Vector2){0, 0}, gp && m == i,
                           (Vector2){200, 300}, START, START);

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

    ui_text(bullet_type_names[type], 20, BLACK, (Vector2){0, 10}, CENTER,
            START);
    ui_text(text, 15, BLACK, (Vector2){0, 40}, CENTER, START);

    Rectangle dest = ui_align(0, 70, 64, 64, CENTER, START);
    Sprite *sprite = &assets.bullets[type];

    DrawTexturePro(sprite->texture, get_frame_rect(sprite, 0), dest,
                   (Vector2){0, 0}, 0, WHITE);

    ui_text(get_bullet_description(game->players[0], type, level), 10, BLACK,
            (Vector2){0, 150}, CENTER, START);
  }
  ui_end_frame();

  if (clicked) {
    game->level_up_option = i;
    game->level_up_menu = false;
  }
}

void ui_draw_level_up_screen(Game game) {
  Player *pdata = &game->players[0]->player;

  static char level_up_str[30];
  sprintf(level_up_str, "Lv. %d -> %d", pdata->level - 1, pdata->level);

  ui_begin_frame(full_screen, overlay_bg);
  {
    // Level up title
    ui_begin_frame_ex(
        ui_align_ex(0, -200, WINDOW_WIDTH, 120, START, CENTER, START, END),
        overlay_bg, BLANK, (Vector2){10, 10});
    {
      ui_text("Level up!", 60, WHITE, (Vector2){0, 0}, CENTER, START);
      ui_text(level_up_str, 30, WHITE, (Vector2){0, 70}, CENTER, START);
    }
    ui_end_frame();

    ui_begin_frame(
        ui_align_ex(0, -160, 860, 400, CENTER, CENTER, CENTER, START), BLANK);
    {
      // Increased stats
      ui_begin_frame_ex(ui_align(0, 0, 200, 300, START, START), overlay_bg,
                        BLANK, (Vector2){10, 10});
      {
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
          sprintf(level_up_str, "Move speed: %.0f -> %.0f",
                  pdata->move_speed - 5, pdata->move_speed);
          ui_text(level_up_str, 15, WHITE, cursor, START, START);
          cursor.y += 20;
        }
      }
      ui_end_frame();

      // Level up options
      for (u32 i = 0; i < LEVEL_UP_OPTIONS; i++) {
        LevelUpOption *option = &pdata->level_up_options[i];
        if (option->type == LU_NONE)
          break;

        ui_draw_level_up_option(game, option, i);
      }
    }
    ui_end_frame();
  }
  ui_end_frame();
}

void ui_draw_pause_screen(Game game) {
  bool gp = IsGamepadAvailable(0);
  u32 m = game->menu_selected_option;

  ui_begin_frame(full_screen, overlay_bg);
  {
    ui_text("Paused", 60, WHITE, (Vector2){0, -60}, CENTER, CENTER);

    if (ui_button_ex("Resume", 20, (Vector2){0, 20}, gp && m == 0,
                     (Vector2){200, 0}, CENTER, CENTER)) {
      game->pause_menu = false;
    }
    if (ui_button_ex("Main menu", 20, (Vector2){0, 60}, gp && m == 1,
                     (Vector2){200, 0}, CENTER, CENTER)) {
      game_set_state(game, GS_MAIN_MENU);
    }
    if (ui_button_ex("Quit", 20, (Vector2){0, 100}, gp && m == 2,
                     (Vector2){200, 0}, CENTER, CENTER)) {
      game_set_state(game, GS_QUIT);
    }
  }
  ui_end_frame();
}

void ui_draw_game_over_screen(Game game) {
  static char score_str[30];
  sprintf(score_str, "Score: %u", game->score);

  bool gp = IsGamepadAvailable(0);
  u32 m = game->menu_selected_option;

  ui_begin_frame(full_screen, overlay_bg);
  {
    ui_text("You Died", 60, WHITE, (Vector2){0, -60}, CENTER, CENTER);
    ui_text(score_str, 20, WHITE, (Vector2){0, -20}, CENTER, CENTER);

    if (ui_button_ex("Main menu", 20, (Vector2){0, 20}, gp && m == 0,
                     (Vector2){200, 0}, CENTER, CENTER)) {
      game_set_state(game, GS_MAIN_MENU);
    }
    if (ui_button_ex("Quit", 20, (Vector2){0, 60}, gp && m == 1,
                     (Vector2){200, 0}, CENTER, CENTER)) {
      game->state = GS_QUIT;
    }
  }
  ui_end_frame();
}

void ui_draw_win_screen(Game game) {
  static char score_str[30];
  sprintf(score_str, "Score: %u", game->score);

  bool gp = IsGamepadAvailable(0);
  u32 m = game->menu_selected_option;

  ui_begin_frame(full_screen, overlay_bg);
  {
    ui_text("Victory!", 60, WHITE, (Vector2){0, -60}, CENTER, CENTER);
    ui_text("Malware Vanquished", 20, WHITE, (Vector2){0, -15}, CENTER, CENTER);
    ui_text(score_str, 20, WHITE, (Vector2){0, 15}, CENTER, CENTER);

    if (ui_button_ex("Main menu", 20, (Vector2){0, 45}, gp && m == 0,
                     (Vector2){200, 0}, CENTER, CENTER)) {
      game_set_state(game, GS_MAIN_MENU);
    }
    if (ui_button_ex("Quit", 20, (Vector2){0, 85}, gp && m == 1,
                     (Vector2){200, 0}, CENTER, CENTER)) {
      game->state = GS_QUIT;
    }
  }
  ui_end_frame();
}
