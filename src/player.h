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
  f32 cooldown;
} SpecialBulletSlot;

typedef enum {
  LU_UPGRADE,
  LU_NEW,
} LevelUpOptionType;

typedef struct LevelUpOption {
  LevelUpOptionType type;
  union {
    u32 bullet_idx;
    BulletType bullet_type;
  };
} LevelUpOption;

typedef enum {
  STAT_AMMO,
  STAT_DAMAGE,
  STAT_HEALTH,
  STAT_MOVEMENT,
} PlayerStats;

typedef struct PlayerData {
  f32 size;

  Vector2 velocity;
  Vector2 crosshair;

  i32 max_health;
  i32 health;
  i32 base_damage;
  f32 move_speed;

  u32 max_ammo;
  u32 ammo;
  u32 special_bullet_count;
  SpecialBulletSlot special_bullets[MAX_SPECIAL_BULLETS];

  f32 fire_cooldown;
  f32 fire_timer;

  u32 level;
  u32 xp;
  u32 to_next_level;
  LevelUpOption *level_up_options[LEVEL_UP_OPTIONS];
  bool leveled_up_stats[4];

  PowerupType active_powerup;
  f32 powerup_timer;

  // Input
  Vector2 direction;
  bool firing;
} PlayerData;

Entity *player_create();

#endif
