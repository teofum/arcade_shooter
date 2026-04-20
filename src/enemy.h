#ifndef ENEMY_H
#define ENEMY_H

#include <raylib.h>

#include "entity.h"
#include "types.h"

typedef enum {
  ENEMY_NORMAL,
  ENEMY_SHOOTER,
  ENEMY_BOSS,
} EnemyType;

typedef enum {
  BOSS_ENTER,
  BOSS_SHOOT_ARC,
  BOSS_MOVE_LEFT_1,
  BOSS_SHOOT_HOMING_1,
  BOSS_MOVE_RIGHT_1,
  BOSS_SHOOT_SPIRAL,
  BOSS_MOVE_RIGHT_2,
  BOSS_SHOOT_HOMING_2,
  BOSS_MOVE_LEFT_2,
} BossState;

typedef struct EnemyData {
  EnemyType type;
  Vector2 size;

  union {
    u32 level;
    BossState boss_state;
  };

  i32 max_health;
  i32 health;
  i32 damage;
  i32 ranged_damage;

  union {
    u32 stat_scaling;
    u32 boss_bullet_counter;
  };

  f32 fire_cooldown;
  f32 fire_timer;

  u32 sprite_type;
  f32 dmg_flash_timer;
} EnemyData;

Entity *enemy_create(u32 x, u32 y, u32 w, u32 h, EnemyType type, u32 level);
Entity *boss_create();

#endif
