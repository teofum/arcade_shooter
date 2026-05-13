#ifndef PLAYER_H
#define PLAYER_H

#include "entity.h"
#include "game.h"

Entity *player_create();
bool player_update(Entity *self, Game game);
void player_draw(Entity *self, Game game);

#endif
