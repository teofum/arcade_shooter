#include <limits.h>
#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>

#include "bullet.h"
#include "config.h"
#include "enemy.h"
#include "entity.h"
#include "entity_list.h"
#include "game.h"
#include "player.h"
#include "ui.h"
#include "utils.h"
#include "wall.h"

Game game_init() {
  Game game = malloc(sizeof(struct Game));
  game->world = NULL;

  game_reset(game);

  return game;
}

void game_reset(Game game) {
  // Destroy old world if it exists
  if (game->world) {
    el_free(game->world);
  }

  // Create world
  game->world = el_create();

  game->player = player_create();
  el_add(game->world, game->player);

  Entity *left_wall =
      wall_create((Rectangle){-1000 - FIELD_WIDTH / 2.0f, -100, 1000, 200});
  Entity *right_wall =
      wall_create((Rectangle){FIELD_WIDTH / 2.0f, -100, 1000, 200});
  Entity *top_wall =
      wall_create((Rectangle){-FIELD_WIDTH / 2.0f, -200, FIELD_WIDTH, 100});

  el_add(game->world, left_wall);
  el_add(game->world, right_wall);
  el_add(game->world, top_wall);

  // Init timers
  game->total_time = GetTime();
  game->delta_time = 0.0f;

  game->enemy_spawn_timer = 0.0f;
  game->enemy_spawn_p = 0.2f;
  game->next_wave_size = 2;

  game->state = GS_MAIN_MENU;
}

void game_process_input(Game game) {
  if (game->state == GS_GAME_OVER)
    return;

  if (IsKeyPressed(KEY_ESCAPE)) {
    if (game->state == GS_PAUSED) {
      game->state = GS_RUNNING;
    } else {
      game->state = GS_PAUSED;
    }
  }

  if (game->state != GS_RUNNING)
    return;

  // Player movement
  Vector2 v_target = {0, 0};

  if (IsKeyDown(KEY_W))
    v_target.y--;
  if (IsKeyDown(KEY_S))
    v_target.y++;
  if (IsKeyDown(KEY_A))
    v_target.x--;
  if (IsKeyDown(KEY_D))
    v_target.x++;

  PlayerData *pdata = (PlayerData *)game->player->custom_data;
  pdata->direction = Vector2Normalize(v_target);

  // Aiming
  pdata->crosshair = screen_to_game(GetMousePosition());

  // Fire!
  pdata->firing = IsKeyDown(KEY_SPACE);
}

void game_update(Game game) {
  // Update timers
  f32 now = GetTime();
  game->delta_time = now - game->total_time;
  game->total_time = now;

  if (game->state != GS_RUNNING)
    return;

  // Update entities
  EntityListIterator it = el_iter(game->world);
  Entity *e;
  while ((e = eli_next(&it))) {
    if (e->update != NULL) {
      e->update(e, game);
    }
  }

  // Spawn enemies
  if (game->enemy_spawn_timer <= 0.0f) {
    for (int i = 0; i < game->next_wave_size; i++) {
      for (int j = 0; j < FIELD_COLS; j++) {
        f32 r = (f32)rand() / RAND_MAX;
        if (r < game->enemy_spawn_p) {
          u32 level = 1 + (game->enemy_spawn_p - r) * 5;
          Entity *enemy = enemy_create(j, i, 1, 1, level);
          el_add(game->world, enemy);
        }
      }
    }

    if (game->enemy_spawn_p < 0.9) {
      game->enemy_spawn_p += 0.01;
    }

    game->next_wave_size = rand() % 3 + 1;
    game->enemy_spawn_timer = ROW_TIME * game->next_wave_size;
  } else {
    game->enemy_spawn_timer -= game->delta_time;
  }
}

void game_draw(Game game) {
  BeginDrawing();
  ClearBackground((Color){0, 128, 128, 255});

  if (game->state == GS_MAIN_MENU) {
    ui_begin_frame((Rectangle){0, 0, WINDOW_WIDTH, WINDOW_HEIGHT}, WHITE);

    ui_text("arcade shooter", 60, BLUE, (Vector2){0, -60}, CENTER, CENTER);

    if (ui_button_ex("Start", 20, (Vector2){0, 20}, (Vector2){200, 0}, CENTER,
                     CENTER)) {
      game->state = GS_RUNNING;
    }
    if (ui_button_ex("Quit", 20, (Vector2){0, 60}, (Vector2){200, 0}, CENTER,
                     CENTER)) {
      game->state = GS_QUIT;
    }

    ui_end_frame();
  } else {
    EntityListIterator it = el_iter(game->world);
    Entity *e;
    while ((e = eli_next(&it))) {
      if (e->draw != NULL) {
        e->draw(e, game);
      }
    }

    ui_draw_game_ui(game);

    if (game->state == GS_LEVEL_UP) {
      // TODO clean up this logic
      static char level_up_str[30];
      PlayerData *pdata = (PlayerData *)game->player->custom_data;
      sprintf(level_up_str, "%d -> %d", pdata->level - 1, pdata->level);

      ui_begin_frame((Rectangle){0, 0, WINDOW_WIDTH, WINDOW_HEIGHT},
                     (Color){0, 0, 0, 128});

      ui_text("Level up!", 60, WHITE, (Vector2){0, -60}, CENTER, CENTER);
      ui_text(level_up_str, 20, WHITE, (Vector2){0, -10}, CENTER, CENTER);

      Vector2 cursor = {0, 20};
      if (pdata->leveled_up_stats[STAT_AMMO]) {
        sprintf(level_up_str, "Ammo: %d -> %d", pdata->max_ammo - 1,
                pdata->max_ammo);
        ui_text(level_up_str, 20, WHITE, cursor, CENTER, CENTER);
        cursor.y += 30;
      }
      if (pdata->leveled_up_stats[STAT_DAMAGE]) {
        sprintf(level_up_str, "Damage: %d -> %d", pdata->base_damage - 1,
                pdata->base_damage);
        ui_text(level_up_str, 20, WHITE, cursor, CENTER, CENTER);
        cursor.y += 30;
      }
      if (pdata->leveled_up_stats[STAT_HEALTH]) {
        sprintf(level_up_str, "Health: %d -> %d", pdata->max_health - 10,
                pdata->max_health);
        ui_text(level_up_str, 20, WHITE, cursor, CENTER, CENTER);
        cursor.y += 30;
      }
      if (pdata->leveled_up_stats[STAT_MOVEMENT]) {
        sprintf(level_up_str, "Move speed: %.0f -> %.0f", pdata->move_speed - 5,
                pdata->move_speed);
        ui_text(level_up_str, 20, WHITE, cursor, CENTER, CENTER);
        cursor.y += 30;
      }

      for (u32 i = 0; i < LEVEL_UP_OPTIONS; i++) {
        LevelUpOption *option = pdata->level_up_options[i];
        if (!option)
          break;

        static char button_text[30];
        if (option->type == LU_NEW) {
          sprintf(button_text, "New: %s",
                  bullet_type_names[option->bullet_type]);

          if (ui_button_ex(button_text, 20, cursor, (Vector2){300, 0}, CENTER,
                           CENTER)) {
            SpecialBulletSlot *bullet =
                &pdata->special_bullets[pdata->special_bullet_count];

            bullet->fired = false;
            bullet->level = 1;
            bullet->type = option->bullet_type;
            pdata->special_bullet_count++;

            game->state = GS_RUNNING;
          }
        } else {
          SpecialBulletSlot *bullet =
              &pdata->special_bullets[option->bullet_idx];

          sprintf(button_text, "%s: Lv. %d -> Lv. %d",
                  bullet_type_names[bullet->type], bullet->level,
                  bullet->level + 1);

          if (ui_button_ex(button_text, 20, cursor, (Vector2){300, 0}, CENTER,
                           CENTER)) {
            bullet->level++;
            game->state = GS_RUNNING;
          }
        }

        cursor.y += 40;
      }

      ui_end_frame();
    } else if (game->state == GS_PAUSED) {
      ui_begin_frame((Rectangle){0, 0, WINDOW_WIDTH, WINDOW_HEIGHT},
                     (Color){0, 0, 0, 128});

      ui_text("Paused", 60, WHITE, (Vector2){0, -60}, CENTER, CENTER);

      if (ui_button_ex("Resume", 20, (Vector2){0, 20}, (Vector2){200, 0},
                       CENTER, CENTER)) {
        game->state = GS_RUNNING;
      }
      if (ui_button_ex("Main menu", 20, (Vector2){0, 60}, (Vector2){200, 0},
                       CENTER, CENTER)) {
        game_reset(game);
      }
      if (ui_button_ex("Quit", 20, (Vector2){0, 100}, (Vector2){200, 0}, CENTER,
                       CENTER)) {
        game->state = GS_QUIT;
      }

      ui_end_frame();
    } else if (game->state == GS_GAME_OVER) {
      ui_begin_frame((Rectangle){0, 0, WINDOW_WIDTH, WINDOW_HEIGHT},
                     (Color){0, 0, 0, 128});

      ui_text("You Died", 60, WHITE, (Vector2){0, -60}, CENTER, CENTER);

      if (ui_button_ex("Main menu", 20, (Vector2){0, 20}, (Vector2){200, 0},
                       CENTER, CENTER)) {
        game_reset(game);
      }
      if (ui_button_ex("Quit", 20, (Vector2){0, 60}, (Vector2){200, 0}, CENTER,
                       CENTER)) {
        game->state = GS_QUIT;
      }

      ui_end_frame();
    }
  }

  // TODO better mouse cursor
  DrawCircleV(GetMousePosition(), 5, BLACK);

  EndDrawing();
}

void game_end(Game game) {
  el_free(game->world);
  free(game);
}
