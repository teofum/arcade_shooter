#include <math.h>
#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>

#include "assets.h"
#include "config.h"
#include "enemy.h"
#include "entity.h"
#include "entity_list.h"
#include "game.h"
#include "player.h"
#include "ui.h"
#include "utils.h"
#include "wall.h"

#define BASE_OFFSET (Vector2){WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f}

Game game_init() {
  Game game = malloc(sizeof(struct Game));
  *game = (struct Game){
      .world = NULL,
      .camera =
          (Camera2D){
              .offset = BASE_OFFSET,
              .zoom = (f32)WINDOW_HEIGHT / FIELD_HEIGHT,
          },
      .players_enabled = {0},
  };

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
  } else if (state == GS_RUNNING) {
    game_setup_menu(game, 3, VERTICAL);
  }

  if (ENABLE_BGM) {
    if (state == GS_MAIN_MENU || state == GS_GAME_OVER) {
      StopMusicStream(game->bgm);
    } else if (state == GS_RUNNING) {
      if (!IsMusicStreamPlaying(game->bgm)) {
        PlayMusicStream(game->bgm);
      }
    }
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

  for (u32 i = 0; i < MAX_CLIENTS; i++) {
    if (game->players_enabled[i]) {
      Entity *player = player_create();
      game->players[i] = el_add(game->world, player);
    }
  }

  Entity *left_wall =
      wall_create((Rectangle){-1000 - FIELD_WIDTH / 2.0f, -100, 1000, 200});
  Entity *right_wall =
      wall_create((Rectangle){FIELD_WIDTH / 2.0f, -100, 1000, 200});
  Entity *top_wall =
      wall_create((Rectangle){-FIELD_WIDTH / 2.0f, -200, FIELD_WIDTH, 100});

  el_add(game->world, left_wall);
  el_add(game->world, right_wall);
  el_add(game->world, top_wall);

  // Ensure server doesn't send creation of player and walls
  el_flush_changes(game->world);

  // Init timers
  game->total_time = GetTime();
  game->delta_time = 0.0f;

  game->enemy_spawn_timer = 0.0f;
  game->enemy_spawn_p = 0.2f;
  game->next_wave_size = 2;

  game->boss_idx = 0;
  game->boss_timer = BOSS_TIME;

  game_set_state(game, GS_RUNNING);
  game->score = 0;

  game->pause_menu = false;
  game->level_up_menu = false;
  game->level_up_option = -1;

  game->bgm = assets.bgm;
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
    if (game->state == GS_RUNNING) {
      game->pause_menu = !game->pause_menu;
    }
  }

  if (game->state != GS_RUNNING)
    return;

  InputData *i = &game->client.input;

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
      i->crosshair = Vector2Add(i->crosshair, aim_movement);
      if (i->crosshair.x < -FIELD_WIDTH / 2.0f)
        i->crosshair.x = -FIELD_WIDTH / 2.0f;
      if (i->crosshair.x > FIELD_WIDTH / 2.0f)
        i->crosshair.x = FIELD_WIDTH / 2.0f;
      if (i->crosshair.y < -FIELD_HEIGHT / 2.0f)
        i->crosshair.y = -FIELD_HEIGHT / 2.0f;
      if (i->crosshair.y > FIELD_HEIGHT / 2.0f)
        i->crosshair.y = FIELD_HEIGHT / 2.0f;
    }

    i->firing = IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_TRIGGER_1);
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
    Matrix m = GetCameraMatrix2D(game->camera);
    m = MatrixInvert(m);
    i->crosshair = Vector2Transform(GetMousePosition(), m);

    // Fire!
    i->firing = IsKeyDown(KEY_SPACE);
  }

  i->direction = v_target;
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
    Entity *enemy =
        enemy_create(FIELD_COLS / 2 - 2, 0, 4, 4, ENEMY_MINIBOSS_1, 2);
    el_add(game->world, enemy);
    break;
  }
  case 1: {
    // Miniboss 2: Big Iron
    Entity *enemy =
        enemy_create(FIELD_COLS / 2 - 1, 0, 2, 2, ENEMY_MINIBOSS_2, 5);
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

    // Boss music
    StopMusicStream(game->bgm);
    game->bgm = assets.bgm_boss;
    PlayMusicStream(game->bgm);

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

void game_update_camera(Game game) {
  static f32 frequencies[] = {50, 50, 80, 80, 120, 120};
  static f32 phases[] = {0, 0.3, 0.7, 0.1, 0.9, 0.5};

  if (game->camera_shake > 0.0f) {
    game->camera.offset = BASE_OFFSET;
    for (u32 i = 0; i < 3; i++) {
      game->camera.offset = Vector2Add(
          game->camera.offset,
          (Vector2){game->camera_shake / (i + 1) *
                        sinf(game->total_time * frequencies[i / 2] +
                             2 * PI * phases[i / 2]),
                    game->camera_shake / (i + 1) *
                        cosf(game->total_time * frequencies[i / 2 + 1] +
                             2 * PI * phases[i / 2 + 1])});
    }

    game->camera_shake -= game->delta_time * CAMERA_SHAKE_DECAY;
    if (game->camera_shake <= 0.0f) {
      game->camera.offset = BASE_OFFSET;
    }
  }
}

void game_update(Game game) {
  // Update BGM stream
  UpdateMusicStream(game->bgm);

  // Update timers
  f32 now = GetTime();
  game->delta_time = now - game->total_time;
  game->total_time = now;

  if (game->state == GS_MAIN_MENU)
    return;

  // Update input
  for (u32 i = 0; i < MAX_CLIENTS; i++) {
    if (game->players_enabled[i]) {
      game->players[i]->player.direction = game->server.input[i].direction;
      game->players[i]->player.crosshair = game->server.input[i].crosshair;
      game->players[i]->player.firing = game->server.input[i].firing;
    }
  }

  // Update entities
  EntityListIterator it = el_iter(game->world);
  Entity *e;
  while ((e = eli_next(it))) {
    if (ent_update(e, game)) {
      eli_destroy_current(it);
    }
  }

  if (game->boss_idx < N_BOSSES)
    game_spawn_enemies(game);

  game_update_camera(game);
}

void game_update_client(Game game) {
  // Update BGM stream
  UpdateMusicStream(game->bgm);

  if (game->state == GS_MAIN_MENU)
    return;

  // Update entities
  EntityListIterator it = el_iter(game->world);
  Entity *e;
  while ((e = eli_next(it))) {
    ent_update_client(e, game);
  }

  game_update_camera(game);
}

void game_remove_player(Game game, u32 idx) {
  if (game->players_enabled[idx]) {
    game->players_enabled[idx] = false;
    el_destroy(game->world, el_indexof(game->world, game->players[idx]));
    game->players[idx] = NULL;
  }
}

void game_draw(Game game) {
  BeginDrawing();
  ClearBackground((Color){0, 128, 128, 255});

  if (game->state == GS_MAIN_MENU) {
    ui_draw_main_menu(game);
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

void game_end(Game game) {
  el_free(game->world);
  free(game);
}
