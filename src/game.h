#ifndef GAME_H
#define GAME_H

#include <raylib.h>

#include "config.h"
#include "entity.h"
#include "entity_list.h"
#include "types.h"

typedef enum {
  GS_MAIN_MENU,
  GS_RUNNING,
  GS_GAME_OVER,
  GS_WIN,
  GS_QUIT,
} GameState;

typedef enum {
  MS_TITLE,
  MS_CONNECT,
} MenuState;

typedef enum {
  CS_READY,
  CS_CONNECTING,
  CS_FAILED,
} ConnectionState;

typedef enum {
  VERTICAL,
  HORIZONTAL,
} Orientation;

typedef struct InputData {
  Vector2 direction;
  Vector2 crosshair;
  bool firing;
} InputData;

typedef enum {
  PLAYER_NONE,
  PLAYER_CLIENT,
  PLAYER_AI,
} PlayerType;

typedef struct Game {
  EntityList world;

  PlayerType player_type[MAX_CLIENTS];
  Entity *players[MAX_CLIENTS];
  i32 host_player_idx;

  union {
    struct {
      InputData input[MAX_CLIENTS];
    } server;
    struct {
      InputData input;
      u32 local_player_idx;
      MenuState menu_state;
      ConnectionState conn_state;
      bool should_start_server;
      bool should_start_game;
      bool should_add_ai;
    } client;
  };

  Camera2D camera;
  f32 camera_shake;

  u64 start_time;
  f32 total_time;
  f32 delta_time;

  u32 next_wave_size;
  f32 enemy_spawn_timer;
  f32 enemy_spawn_p;

  f32 boss_timer;
  u32 boss_idx;

  GameState state;
  u32 score;

  bool pause_menu;
  bool level_up_menu;
  i32 level_up_option;

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
void game_update_client(Game game);

void game_remove_player(Game game, u32 idx);

void game_draw(Game game);

void game_end(Game game);

#endif
