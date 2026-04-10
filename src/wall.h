#ifndef WALL_H
#define WALL_H

#include <raylib.h>

#include "entity.h"

typedef struct WallData {
  Rectangle bounds;
} WallData;

Entity *wall_create(Rectangle bounds);

#endif
