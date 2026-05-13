#ifndef LASER_H
#define LASER_H

#include <raylib.h>

#include "entity.h"
#include "game.h"
#include "types.h"

Entity *laser_create(Vector2 position, i32 damage);
bool laser_update(Entity *self, Game game);
void laser_draw(Entity *self, Game game);

#endif
