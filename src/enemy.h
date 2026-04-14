#ifndef ENEMY_H
#define ENEMY_H

#include <raylib.h>

#include "entity.h"
#include "types.h"

typedef enum {
  ENEMY_NORMAL,
  ENEMY_SHOOTER,
} EnemyType;

typedef struct EnemyData {
  EnemyType type;
  Vector2 size;

  u32 level;
  i32 max_health;
  i32 health;
  i32 damage;
  i32 ranged_damage;
  u32 stat_scaling;

  f32 fire_cooldown;
  f32 fire_timer;
} EnemyData;

Entity *enemy_create(u32 x, u32 y, u32 w, u32 h, EnemyType type, u32 level);

#endif
