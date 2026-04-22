#ifndef GAME_H
#define GAME_H

#include "entity.h"
#include "entity_list.h"
#include "raylib.h"
#include "types.h"

typedef enum {
  GS_MAIN_MENU,
  GS_RUNNING,
  GS_LEVEL_UP,
  GS_PAUSED,
  GS_GAME_OVER,
  GS_WIN,
  GS_QUIT,
} GameState;

typedef enum {
  VERTICAL,
  HORIZONTAL,
} Orientation;

typedef struct Game {
  EntityList world;
  Entity *player;

  f32 total_time;
  f32 delta_time;

  u32 next_wave_size;
  f32 enemy_spawn_timer;
  f32 enemy_spawn_p;

  f32 boss_timer;
  u32 boss_idx;

  GameState state;
  u32 score;

  u32 menu_selected_option;
  u32 menu_n_options;
  Orientation menu_layout;

  Music bgm;
} *Game;

Game game_init();

void game_set_state(Game game, GameState state);

void game_reset(Game game);

void game_process_input(Game game);

void game_update(Game game);

void game_draw(Game game);

void game_end(Game game);

#endif
