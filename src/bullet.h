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
  BULLET_SECONDARY = -1,
} BulletType;

typedef struct BulletData {
  Vector2 velocity;
  f32 size;

  BulletType type;
  u32 level;
  i32 damage;
  u32 special_idx;
} BulletData;

Entity *bullet_create(Vector2 position, Vector2 target, BulletType type,
                      u32 level, i32 damage, u32 special_idx);

#endif
