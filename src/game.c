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

static void game_setup_menu(Game game, u32 n_opts, Orientation orientation) {
  game->menu_n_options = n_opts;
  game->menu_selected_option = 0;
  game->menu_layout = orientation;
}

void game_set_state(Game game, GameState state) {
  if (state == GS_MAIN_MENU || state == GS_GAME_OVER) {
    game_setup_menu(game, 2, VERTICAL);
  } else if (state == GS_PAUSED) {
    game_setup_menu(game, 3, VERTICAL);
  } else if (state == GS_LEVEL_UP) {
    game_setup_menu(game, 3, HORIZONTAL);
  }

  game->state = state;
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

  game_set_state(game, GS_MAIN_MENU);
  game->score = 0;
}

void game_process_input(Game game) {
  if (IsGamepadAvailable(0) && game->state != GS_RUNNING) {
    if (game->menu_layout == VERTICAL) {
      if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_DOWN)) {
        game->menu_selected_option =
            (game->menu_selected_option + 1) % game->menu_n_options;
      }
      if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_UP)) {
        game->menu_selected_option = game->menu_selected_option == 0
                                         ? game->menu_n_options - 1
                                         : game->menu_selected_option - 1;
      }
    } else {
      if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_RIGHT)) {
        game->menu_selected_option =
            (game->menu_selected_option + 1) % game->menu_n_options;
      }
      if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_LEFT)) {
        game->menu_selected_option = game->menu_selected_option == 0
                                         ? game->menu_n_options - 1
                                         : game->menu_selected_option - 1;
      }
    }
  }

  if (game->state == GS_GAME_OVER)
    return;

  if (IsKeyPressed(KEY_ESCAPE) ||
      IsGamepadAvailable(0) &&
          IsGamepadButtonPressed(0, GAMEPAD_BUTTON_MIDDLE_RIGHT)) {
    if (game->state == GS_PAUSED) {
      game_set_state(game, GS_RUNNING);
    } else if (game->state == GS_RUNNING) {
      game_set_state(game, GS_PAUSED);
    }
  }

  if (game->state != GS_RUNNING)
    return;

  PlayerData *pdata = (PlayerData *)game->player->custom_data;

  // Player movement
  Vector2 v_target = {0, 0};

  if (IsGamepadAvailable(0)) {
    // Controller input
    v_target.x = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X);
    v_target.y = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y);

    if (Vector2Length(v_target) < INPUT_GAMEPAD_DEADZONE) {
      v_target = (Vector2){0, 0};
    }

    Vector2 aim_movement = {
        GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_X),
        GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_Y),
    };
    if (Vector2Length(aim_movement) >= INPUT_GAMEPAD_DEADZONE) {
      pdata->crosshair = Vector2Add(pdata->crosshair, aim_movement);
      if (pdata->crosshair.x < -FIELD_WIDTH / 2.0f)
        pdata->crosshair.x = -FIELD_WIDTH / 2.0f;
      if (pdata->crosshair.x > FIELD_WIDTH / 2.0f)
        pdata->crosshair.x = FIELD_WIDTH / 2.0f;
      if (pdata->crosshair.y < -FIELD_HEIGHT / 2.0f)
        pdata->crosshair.y = -FIELD_HEIGHT / 2.0f;
      if (pdata->crosshair.y > FIELD_HEIGHT / 2.0f)
        pdata->crosshair.y = FIELD_HEIGHT / 2.0f;
    }

    pdata->firing = IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_TRIGGER_1);
  } else {
    // Keyboard input
    if (IsKeyDown(KEY_W))
      v_target.y--;
    if (IsKeyDown(KEY_S))
      v_target.y++;
    if (IsKeyDown(KEY_A))
      v_target.x--;
    if (IsKeyDown(KEY_D))
      v_target.x++;

    v_target = Vector2Normalize(v_target);

    // Aiming
    pdata->crosshair = screen_to_game(GetMousePosition());

    // Fire!
    pdata->firing = IsKeyDown(KEY_SPACE);
  }

  pdata->direction = v_target;
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
    Entity *boss = boss_create();
    el_add(game->world, boss);
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
    ui_draw_main_menu(game);
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
  if (!IsGamepadAvailable(0)) {
    DrawCircleV(GetMousePosition(), 5, BLACK);
  }

  EndDrawing();
}

void game_end(Game game) {
  el_free(game->world);
  free(game);
}
