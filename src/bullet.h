#ifndef BULLET_H
#define BULLET_H

#include <raylib.h>

#include "entity.h"
#include "game.h"
#include "types.h"

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

extern const char *bullet_type_names[6];
extern Color bullet_type_colors[6];

typedef struct BulletData {
  Vector2 velocity;
  f32 size;

  BulletType type;
  u32 level;
  i32 damage;
  u32 special_idx;

  bool deferred_destroy;
} BulletData;

Entity *bullet_create(Vector2 position, Vector2 target, BulletType type,
                      u32 level, i32 damage, u32 special_idx);

bool bullet_hit_enemy(Entity *self, Entity *enemy, Game game);

const char *get_bullet_description(Game game, BulletType type, u32 level);

#endif
