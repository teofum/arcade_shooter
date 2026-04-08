#ifndef ENEMY_H
#define ENEMY_H

#include <raylib.h>

#include "entity.h"
#include "game.h"
#include "types.h"

typedef struct EnemyData {
  Vector2 size;

  i32 max_health;
  i32 health;
  i32 damage;
} EnemyData;

Entity *enemy_create(u32 x, u32 y, u32 w, u32 h);

void enemy_update(Entity *enemy, Game game);

void enemy_draw(Entity *enemy, Game game);

#endif
