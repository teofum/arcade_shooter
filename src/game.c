#include <limits.h>
#include <math.h>
#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>

#include "config.h"
#include "enemy.h"
#include "entity.h"
#include "entity_list.h"
#include "game.h"
#include "player.h"
#include "utils.h"
#include "wall.h"

Game game_init() {
  Game game = malloc(sizeof(struct Game));

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

  game->game_over = false;

  return game;
}

void game_process_input(Game game) {
  if (game->game_over)
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
        if ((float)rand() / INT_MAX < game->enemy_spawn_p) {
          Entity *enemy = enemy_create(j, i, 1, 1);
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

// UI TODO move these to some other file?
static void ui_draw_health_bar(PlayerData *pdata) {
  f32 w = fminf(pdata->max_health, 400);
  f32 h = 20;
  f32 x = 20;
  f32 y = WINDOW_HEIGHT - 20 - h;

  f32 relative_health = (f32)pdata->health / pdata->max_health;

  DrawRectangle(x, y, w, h, DARKGRAY);
  DrawRectangle(x, y, relative_health * w, h, RED);
  DrawRectangleLines(x, y, w, h, BLACK);
}

static void ui_draw_ammo_counter(PlayerData *pdata) {
  f32 size = 5;
  f32 x0 = 20 + size;
  f32 x = x0;
  f32 y = WINDOW_HEIGHT - 50 - size;

  for (i32 i = 0; i < pdata->max_ammo; i++) {
    DrawCircle(x, y, size, i < pdata->ammo ? BLUE : DARKGRAY);

    x += size * 2 + 5;
    if (i % 8 == 7) {
      x = x0;
      y += size * 2 + 5;
    }
  }
}

static void ui_draw_game_ui(Game game) {
  PlayerData *pdata = (PlayerData *)game->player->custom_data;

  ui_draw_health_bar(pdata);
  ui_draw_ammo_counter(pdata);
}

void game_draw(Game game) {
  BeginDrawing();
  ClearBackground(WHITE);

  EntityListIterator it = el_iter(game->world);
  Entity *e;
  while ((e = eli_next(&it))) {
    if (e->draw != NULL) {
      e->draw(e, game);
    }
  }

  ui_draw_game_ui(game);

  if (game->game_over) {
    DrawText("you died", WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, 60, BLACK);
  }

  EndDrawing();
}
