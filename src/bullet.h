#ifndef BULLET_H
#define BULLET_H

#include "entity.h"
#include "game.h"

typedef struct BulletData {
  Vector2 velocity;
} BulletData;

BulletData *bullet_init_data(Vector2 initial_velocity);

void bullet_update(Entity *bullet, Game game);

void bullet_draw(Entity *bullet, Game game);

#endif
