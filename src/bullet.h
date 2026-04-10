#ifndef BULLET_H
#define BULLET_H

#include <raylib.h>

#include "entity.h"
#include "types.h"

typedef struct BulletData {
  Vector2 velocity;
  f32 size;

  i32 damage;
} BulletData;

Entity *bullet_create(Vector2 position, Vector2 target);

#endif
