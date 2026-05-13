#ifndef EXPLOSION_H
#define EXPLOSION_H

#include <raylib.h>

#include "entity.h"
#include "game.h"
#include "types.h"

Entity *explosion_create(Vector2 position, f32 radius, i32 damage);
bool explosion_update(Entity *self, Game game);
void explosion_draw(Entity *self, Game game);

#endif
