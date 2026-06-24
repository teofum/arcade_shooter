#include <assert.h>
#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>

#include "assets.h"
#include "config.h"
#include "entity.h"
#include "game.h"
#include "types.h"
#include "ui.h"

#define BUTTON_PADDING 5

// Constants
static Color overlay_bg = {0, 0, 0, 128};
static Rectangle full_screen = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};

void ui_draw_server_menu(Game game) {
  BeginDrawing();
  ClearBackground((Color){0, 128, 128, 255});

  ui_begin_frame(full_screen, WHITE);
  {
    DrawTexturePro(assets.title_bg, (Rectangle){0, 0, 2160, 1440}, full_screen,
                   (Vector2){0, 0}, 0, WHITE);

    ui_text("Desktop\nDefender", 60, BLACK,
            (Vector2){WINDOW_WIDTH / 2.0f + 100, -148}, START, CENTER);
    ui_text("Desktop\nDefender", 60, RED,
            (Vector2){WINDOW_WIDTH / 2.0f + 100, -150}, START, CENTER);

    bool gp = IsGamepadAvailable(0);
    u32 m = game->menu_selected_option;

    if (ui_button_ex("Start game", 20, (Vector2){WINDOW_WIDTH / 2.0f + 100, 0},
                     gp && m == 0, (Vector2){200, 0}, START, CENTER)) {
      game_reset(game);
    }
    if (ui_button_ex("Quit", 20, (Vector2){WINDOW_WIDTH / 2.0f + 100, 40},
                     gp && m == 1, (Vector2){200, 0}, START, CENTER)) {
      game_set_state(game, GS_QUIT);
    }
  }
  ui_end_frame();

  EndDrawing();
}
