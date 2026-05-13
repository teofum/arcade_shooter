#ifndef WALL_H
#define WALL_H

#include <raylib.h>

#include "entity.h"
#include "game.h"

Entity *wall_create(Rectangle bounds);
void wall_draw(Entity *self, Game game);

#endif
