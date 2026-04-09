#include <limits.h>
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

  EntityListIterator it = el_iter(game->world);
  Entity *e;
  while ((e = eli_next(&it))) {
    if (e->update != NULL) {
      e->update(e, game);
    }
  }

  if ((float)rand() / INT_MAX < 0.002f) {
    // Spawn enemy
    Entity *enemy = enemy_create(rand() % FIELD_COLS, rand() % 3, 1, 1);
    el_add(game->world, enemy);
  }
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

  // Health bar (TODO: actual ui)
  PlayerData *pdata = (PlayerData *)game->player->custom_data;
  DrawRectangle(10, 10, pdata->max_health, 20, BLACK);
  DrawRectangle(10, 10, pdata->health, 20, BLUE);
  DrawRectangleLines(10, 10, pdata->max_health, 20, BLACK);

  if (game->game_over) {
    DrawText("you died", WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, 60, BLACK);
  }

  EndDrawing();
}
