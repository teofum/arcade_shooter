#ifndef BULLET_H
#define BULLET_H

#include <raylib.h>

#include "entity.h"
#include "types.h"

typedef enum {
  BULLET_NONE = 0,
  BULLET_NORMAL = 0,
  BULLET_REPLICATE, // :(){:|:&};:
  BULLET_EXPLOSIVE, // zip bomb
  BULLET_SHRAPNEL,  // spam
  BULLET_LASER,     // firewall
  BULLET_HEALING,   // ???
} BulletType;

typedef struct BulletData {
  Vector2 velocity;
  f32 size;

  i32 damage;
} BulletData;

Entity *bullet_create(Vector2 position, Vector2 target, i32 damage);

#endif
