#ifndef ENEMY_BULLET_H
#define ENEMY_BULLET_H

#include <raylib.h>

#include "entity.h"
#include "game.h"
#include "types.h"

Entity *enemy_bullet_create(Vector2 position, Vector2 target, i32 damage);
bool enemy_bullet_update(Entity *self, Game game);
void enemy_bullet_draw(Entity *self, Game game);

#endif
