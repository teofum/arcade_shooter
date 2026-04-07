#ifndef PLAYER_H
#define PLAYER_H

#include "entity.h"
#include "game.h"

void player_update(Entity *player, Game game);

void player_draw(Entity *player, Game game);

#endif
