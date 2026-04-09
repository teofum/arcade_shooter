#ifndef GAME_H
#define GAME_H

#include "entity.h"
#include "entity_list.h"
#include "types.h"

typedef struct Game {
  EntityList world;
  Entity *player;

  f32 total_time;
  f32 delta_time;

  u32 next_wave_size;
  f32 enemy_spawn_timer;
  f32 enemy_spawn_p;

  bool game_over;
} *Game;

Game game_init();

void game_process_input(Game game);

void game_update(Game game);

void game_draw(Game game);

#endif
