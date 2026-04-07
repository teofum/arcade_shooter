#ifndef GAME_H
#define GAME_H

#include "entity.h"
#include "entity_list.h"

typedef struct Game {
  EntityList world;
  Entity *player;
} *Game;

Game game_init();

void game_process_input(Game game);

void game_update(Game game);

void game_draw(Game game);

#endif
