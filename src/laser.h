#ifndef LASER_H
#define LASER_H

#include <raylib.h>

#include "entity.h"
#include "types.h"

typedef struct LaserData {
  i32 damage;

  f32 ttl;
} LaserData;

Entity *laser_create(Vector2 position, i32 damage);

#endif
