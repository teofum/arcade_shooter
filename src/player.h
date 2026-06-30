#ifndef PLAYER_H
#define PLAYER_H

#include "config.h"
#include "entity.h"
#include "game.h"

extern Color player_colors[MAX_CLIENTS];

Entity *player_create();
bool player_update(Entity *self, Game game);
bool player_update_client(Entity *self, Game game);
void player_draw(Entity *self, Game game);

#endif
