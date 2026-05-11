#ifndef ENTITY_H
#define ENTITY_H

#include <raylib.h>

#include "config.h"
#include "types.h"

typedef enum {
  ENT_PLAYER,
  ENT_BULLET,
  ENT_WALL,
  ENT_ENEMY,
  ENT_DMG_NUMBER,
  ENT_XP_GEM,
  ENT_POWERUP,
  ENT_EXPLOSION,
  ENT_LASER,
  ENT_ENEMY_BULLET,
} EntityType;

typedef enum {
  BULLET_NONE = 0,
  BULLET_NORMAL = 0,
  BULLET_REPLICATE, // :(){:|:&};:
  BULLET_EXPLOSIVE, // minesweeper
  BULLET_SHRAPNEL,  // recycle bin
  BULLET_LASER,     // firewall
  BULLET_HEALING,   // defrag
  BULLET_SECONDARY = -1,
} BulletType;

typedef struct BulletData {
  Vector2 velocity;
  f32 size;

  BulletType type;
  u32 level;
  i32 damage;
  u32 special_idx;

  bool deferred_destroy;
} BulletData;

typedef enum {
  POWER_FAST,
  POWER_DMG,
  POWER_NONE = -1,
} PowerupType;

typedef struct PowerupData {
  PowerupType type;

  Vector2 velocity;
} PowerupData;

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

  // Rendering
  i32 orientation;
} PlayerData;

typedef struct WallData {
  Rectangle bounds;
} WallData;

typedef enum {
  ENEMY_NORMAL,
  ENEMY_SHOOTER,
  ENEMY_BOSS,
  ENEMY_MINIBOSS_1,
  ENEMY_MINIBOSS_2,
} EnemyType;

typedef enum {
  BOSS_DEAD,
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

typedef struct DmgNumberData {
  char string[10];
  f32 size;
  i32 damage;
  f32 timer;
  f32 speed;
} DmgNumberData;

typedef struct XpGemData {
  u32 value;

  Vector2 velocity;
} XpGemData;

typedef struct ExplosionData {
  f32 radius;
  i32 damage;

  f32 ttl;
} ExplosionData;

typedef struct LaserData {
  i32 damage;

  f32 ttl;
} LaserData;

typedef struct EnemyBulletData {
  Vector2 velocity;
  f32 size;

  i32 damage;

  bool deferred_destroy;
} EnemyBulletData;

struct Entity;
struct Game;

typedef void (*EntityUpdateFunction)(struct Entity *entity, struct Game *game);
typedef void (*EntityDrawFunction)(struct Entity *entity, struct Game *game);

typedef struct Entity {
  EntityType type;

  Vector2 position;

  EntityUpdateFunction update;
  EntityDrawFunction draw;

  union {
    PlayerData player;
    BulletData bullet;
    WallData wall;
    EnemyData enemy;
    DmgNumberData dmg_number;
    XpGemData xp_gem;
    PowerupData powerup;
    ExplosionData explosion;
    LaserData laser;
    EnemyBulletData enemy_bullet;
  };
} Entity;

Entity *ent_create(EntityType type);

#endif
