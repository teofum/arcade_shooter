#ifndef WALL_H
#define WALL_H

#include <raylib.h>

#include "entity.h"
#include "game.h"

typedef struct WallData {
  Rectangle bounds;
} WallData;

Entity *wall_create(Rectangle bounds);

void wall_update(Entity *wall, Game game);

void wall_draw(Entity *wall, Game game);

#endif
