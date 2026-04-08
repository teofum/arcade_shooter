#ifndef ENEMY_H
#define ENEMY_H

#include <raylib.h>

#include "entity.h"
#include "game.h"

typedef struct EnemyData {
  Rectangle bounds;
} EnemyData;

Entity *enemy_create(Vector2 position);

void enemy_update(Entity *enemy, Game game);

void enemy_draw(Entity *enemy, Game game);

#endif
