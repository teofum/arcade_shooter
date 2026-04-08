#ifndef BULLET_H
#define BULLET_H

#include <raylib.h>

#include "entity.h"
#include "game.h"
#include "types.h"

typedef struct BulletData {
  Vector2 velocity;
  f32 size;
} BulletData;

Entity *bullet_create(Vector2 position, Vector2 target);

void bullet_update(Entity *bullet, Game game);

void bullet_draw(Entity *bullet, Game game);

#endif
