#ifndef PLAYER_H
#define PLAYER_H

#include "bullet.h"
#include "config.h"
#include "entity.h"
#include "powerup.h"
#include "types.h"

typedef struct SpecialBulletSlot {
  BulletType type;
  u32 level;
  bool fired;
} SpecialBulletSlot;

typedef struct PlayerData {
  f32 size;

  Vector2 velocity;
  Vector2 crosshair;

  i32 max_health;
  i32 health;

  u32 max_ammo;
  u32 ammo;
  u32 special_bullet_count;
  SpecialBulletSlot special_bullets[MAX_SPECIAL_BULLETS];

  f32 fire_cooldown;
  f32 fire_timer;

  u32 level;
  u32 xp;
  u32 to_next_level;

  PowerupType active_powerup;
  f32 powerup_timer;

  // Input
  Vector2 direction;
  bool firing;
} PlayerData;

Entity *player_create();

#endif
