#ifndef EXPLOSION_H
#define EXPLOSION_H

#include <raylib.h>

#include "entity.h"
#include "types.h"

typedef struct ExplosionData {
  f32 radius;
  i32 damage;

  f32 ttl;
} ExplosionData;

Entity *explosion_create(Vector2 position, f32 radius, i32 damage);

#endif
