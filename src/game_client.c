#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "entity.h"
#include "entity_list.h"
#include "game.h"
#include "player.h"
#include "ui.h"

void game_draw(Game game) {
  BeginDrawing();
  ClearBackground((Color){0, 128, 128, 255});

  if (game->state == GS_MAIN_MENU) {
    ui_draw_lobby_screen(game);
  } else {
    BeginMode2D(game->camera);
    {
      for (u32 i = 0, j = 0; i < el_size(game->world); i++) {
        Entity *e = el_get(game->world, i);
        ent_draw(e, game);
      }

      // Draw AI debug
      if (game->client.show_ai_debug_ui) {
        for (u32 i = 0; i < MAX_CLIENTS; i++) {
          if (game->player_type[i] == PLAYER_AI &&
              game->client.show_ai_debug_ui) {
            AiPlayer *ai = &game->ai_players[i];
            Color color = player_colors[i];
            Color color_t = color;
            color_t.a = 50;

            Vector2 p = game->players[i]->position;

            // Current position
            DrawCircle(p.x, p.y, 1, color);
            DrawCircleLines(p.x, p.y, 1, BLACK);

            // Target position and radii
            DrawCircle(ai->target_pos.x, ai->target_pos.y, AI_MOVE_START_D,
                       color_t);
            DrawCircleLines(ai->target_pos.x, ai->target_pos.y, AI_MOVE_START_D,
                            color);
            DrawCircle(ai->target_pos.x, ai->target_pos.y, AI_MOVE_STOP_D,
                       color_t);
            DrawCircleLines(ai->target_pos.x, ai->target_pos.y, AI_MOVE_STOP_D,
                            color);
            DrawCircle(ai->target_pos.x, ai->target_pos.y, 1, color);
            DrawCircleLines(ai->target_pos.x, ai->target_pos.y, 1, BLACK);

            // Crosshair
            Vector2 c = ai->crosshair;
            DrawLineEx((Vector2){c.x - 3, c.y - 3}, (Vector2){c.x + 3, c.y + 3},
                       1.2f, BLACK);
            DrawLineEx((Vector2){c.x - 3, c.y + 3}, (Vector2){c.x + 3, c.y - 3},
                       1.2f, BLACK);
            DrawLineEx((Vector2){c.x - 2.8f, c.y - 2.8f},
                       (Vector2){c.x + 2.8f, c.y + 2.8f}, 0.6f, color);
            DrawLineEx((Vector2){c.x - 2.8f, c.y + 2.8f},
                       (Vector2){c.x + 2.8f, c.y - 2.8f}, 0.6f, color);
          }
        }
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
