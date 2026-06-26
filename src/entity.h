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

extern const char *entity_type_name[];

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

typedef struct Bullet {
  Vector2 velocity;
  f32 size;

  BulletType type;
  u32 level;
  i32 damage;

  struct Entity *player;
  u32 special_idx;

  bool deferred_destroy;
} Bullet;

typedef enum {
  POWER_FAST,
  POWER_DMG,
  POWER_NONE = -1,
} PowerupType;

typedef struct Powerup {
  PowerupType type;

  Vector2 velocity;
} Powerup;

typedef struct SpecialBulletSlot {
  BulletType type;
  f32 cooldown;
  u8 level;
  bool fired;
} SpecialBulletSlot;

typedef enum {
  LU_NONE,
  LU_UPGRADE,
  LU_NEW,
} LevelUpOptionType;

typedef struct LevelUpOption {
  LevelUpOptionType type;
  union {
    BulletType bullet_type;
    u32 bullet_idx;
  };
} LevelUpOption;

typedef enum {
  STAT_AMMO,
  STAT_DAMAGE,
  STAT_HEALTH,
  STAT_MOVEMENT,
} PlayerStats;

typedef struct Player {
  f32 size;

  Vector2 velocity;
  Vector2 crosshair;

  f32 move_speed;
  i16 max_health;
  i16 health;
  i16 base_damage;

  u8 max_ammo;
  u8 ammo;
  u8 special_bullet_count;
  u8 level;
  bool leveled_up;
  bool firing;

  SpecialBulletSlot special_bullets[MAX_SPECIAL_BULLETS];

  f32 fire_cooldown;
  f32 fire_timer;

  u32 xp;
  u32 to_next_level;
  LevelUpOption level_up_options[LEVEL_UP_OPTIONS];
  bool leveled_up_stats[4];

  PowerupType active_powerup;
  f32 powerup_timer;

  Vector2 direction;
  i32 orientation;
} Player;

typedef struct Wall {
  Rectangle bounds;
} Wall;

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

typedef struct Enemy {
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
} Enemy;

typedef struct DmgNumber {
  char string[10];
  f32 size;
  i32 damage;
  f32 timer;
  f32 speed;
} DmgNumber;

typedef struct XpGem {
  u32 value;

  Vector2 velocity;
} XpGem;

typedef struct Explosion {
  f32 radius;
  i32 damage;

  f32 ttl;
} Explosion;

typedef struct Laser {
  i32 damage;

  f32 ttl;
} Laser;

typedef struct EnemyBullet {
  Vector2 velocity;
  f32 size;

  i32 damage;

  bool deferred_destroy;
} EnemyBullet;

struct Entity;
struct Game;

typedef bool (*EntityUpdateFunction)(struct Entity *entity, struct Game *game);
typedef void (*EntityDrawFunction)(struct Entity *entity, struct Game *game);

typedef struct Entity {
  EntityType type;

  // Common fields
  Vector2 position;

  union {
    Player player;
    Bullet bullet;
    Wall wall;
    Enemy enemy;
    DmgNumber dmg_number;
    XpGem xp_gem;
    Powerup powerup;
    Explosion explosion;
    Laser laser;
    EnemyBullet enemy_bullet;
  };
} Entity;

typedef struct BulletCreateData {
  Vector2 target;
  BulletType type;
  u32 level;
  i32 damage;
  u32 special_idx;
} BulletCreateData;

typedef struct EnemyCreateData {
  EnemyType type;
  u32 w;
  u32 h;
  u32 level;
} EnemyCreateData;

typedef struct ExplosionCreateData {
  f32 radius;
  i32 damage;
} ExplosionCreateData;

typedef struct DmgNumberCreateData {
  f32 size;
  i32 damage;
} DmgNumberCreateData;

typedef struct EntityCreateData {
  Vector2 position;

  union {
    BulletCreateData bullet;
    EnemyCreateData enemy;
    u32 xp_value;
    PowerupType powerup_type;
    ExplosionCreateData explosion;
    i32 laser_damage;
    DmgNumberCreateData dmg_number;
  };
} EntityCreateData;

Entity *ent_create(EntityType type);

bool ent_update(Entity *entity, struct Game *game);
bool ent_update_client(Entity *entity, struct Game *game);
void ent_draw(Entity *entity, struct Game *game);

EntityCreateData ent_get_create_data(Entity *entity);

#endif
