#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>

#include "entity.h"
#include "entity_list.h"
#include "game.h"
#include "ui.h"

void game_draw(Game game) {
  BeginDrawing();
  ClearBackground((Color){0, 128, 128, 255});

  if (game->state == GS_MAIN_MENU) {
    ui_draw_lobby_screen(game);
  } else {
    BeginMode2D(game->camera);
    {
      EntityListIterator it = el_iter(game->world);
      Entity *e;
      while ((e = eli_next(it))) {
        ent_draw(e, game);
      }
    }
    EndMode2D();

    ui_draw_game_ui(game);

    if (game->level_up_menu) {
      ui_draw_level_up_screen(game);
    } else if (game->pause_menu) {
      ui_draw_pause_screen(game);
    } else if (game->state == GS_GAME_OVER) {
      ui_draw_game_over_screen(game);
    } else if (game->state == GS_WIN) {
      ui_draw_win_screen(game);
    }
  }

  // TODO better mouse cursor
  if (!IsGamepadAvailable(0)) {
    DrawCircleV(GetMousePosition(), 5, BLACK);
  }

  EndDrawing();
}
