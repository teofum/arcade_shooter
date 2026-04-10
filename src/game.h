#ifndef GAME_H
#define GAME_H

#include "entity.h"
#include "entity_list.h"
#include "types.h"

typedef enum {
  GS_RUNNING,
  GS_PAUSED,
  GS_GAME_OVER,
  GS_QUIT,
} GameState;

typedef struct Game {
  EntityList world;
  Entity *player;

  f32 total_time;
  f32 delta_time;

  u32 next_wave_size;
  f32 enemy_spawn_timer;
  f32 enemy_spawn_p;

  GameState state;
} *Game;

Game game_init();

void game_process_input(Game game);

void game_update(Game game);

void game_draw(Game game);

#endif
