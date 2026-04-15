#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>

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

  game->boss_idx = 0;
  game->boss_timer = BOSS_TIME;

  game->state = GS_MAIN_MENU;
  game->score = 0;
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

static void game_spawn_wave(Game game) {
  for (int i = 0; i < game->next_wave_size; i++) {
    for (int j = 0; j < FIELD_COLS; j++) {
      f32 r = frand();
      if (r < game->enemy_spawn_p) {
        EnemyType type = (game->enemy_spawn_p > 0.3f && frand() < 0.1f)
                             ? ENEMY_SHOOTER
                             : ENEMY_NORMAL;
        u32 level = 1 + (game->enemy_spawn_p - r) * 5;

        u32 w = 1;
        if (j < FIELD_COLS - 1 && frand() < 0.02f) {
          w = 2;
          j++;
        }

        Entity *enemy = enemy_create(j, i, w, 1, type, level);
        el_add(game->world, enemy);
      }
    }
  }
}

static void game_spawn_boss(Game game) {
  switch (game->boss_idx) {
  case 0: {
    // Miniboss 1: Big Chungus
    Entity *enemy = enemy_create(FIELD_COLS / 2 - 2, 0, 4, 4, ENEMY_NORMAL, 2);
    el_add(game->world, enemy);
    break;
  }
  case 1: {
    // Miniboss 2: Big Iron
    Entity *enemy = enemy_create(FIELD_COLS / 2 - 1, 0, 2, 2, ENEMY_SHOOTER, 5);
    el_add(game->world, enemy);

    for (u32 j = FIELD_COLS / 2 - 2; j <= FIELD_COLS / 2 + 1; j++) {
      enemy = enemy_create(j, 2, 1, 1, ENEMY_NORMAL, 5);
      el_add(game->world, enemy);
    }
    for (u32 i = 0; i < 2; i++) {
      enemy = enemy_create(FIELD_COLS / 2 - 2, i, 1, 1, ENEMY_NORMAL, 3);
      el_add(game->world, enemy);
      enemy = enemy_create(FIELD_COLS / 2 + 1, i, 1, 1, ENEMY_NORMAL, 3);
      el_add(game->world, enemy);
    }
    break;
  }
  case 2: {
    // Final boss
    // TODO
    break;
  }
  }

  game->boss_idx += 1;
  game->boss_timer = BOSS_TIME;
}

static void game_spawn_enemies(Game game) {
  if (game->enemy_spawn_timer <= 0.0f) {
    if (game->next_wave_size == 0) {
      game_spawn_boss(game);
    } else {
      game_spawn_wave(game);

      // Increase diffculty
      if (game->enemy_spawn_p < 0.9) {
        game->enemy_spawn_p += 0.01 * game->next_wave_size;
      }
    }

    if (game->boss_timer <= 0.0f) {
      game->next_wave_size = 0;
      game->enemy_spawn_timer = ROW_TIME * 4;
    } else {
      game->next_wave_size = rand() % 3 + 1;
      game->enemy_spawn_timer = ROW_TIME * game->next_wave_size;
    }
  }

  game->enemy_spawn_timer -= game->delta_time;
  game->boss_timer -= game->delta_time;
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

  if (game->boss_idx < N_BOSSES)
    game_spawn_enemies(game);
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
      ui_draw_level_up_screen(game);
    } else if (game->state == GS_PAUSED) {
      ui_draw_pause_screen(game);
    } else if (game->state == GS_GAME_OVER) {
      ui_draw_game_over_screen(game);
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
