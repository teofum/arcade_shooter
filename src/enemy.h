#ifndef ENEMY_H
#define ENEMY_H

#include <raylib.h>

#include "entity.h"
#include "game.h"
#include "types.h"

Entity *enemy_create(u32 x, u32 y, u32 w, u32 h, EnemyType type, u32 level,
                     i32 sprite_type);
Entity *boss_create();
bool enemy_update(Entity *self, Game game);
bool enemy_update_client(Entity *self, Game game);
void enemy_draw(Entity *self, Game game);

bool enemy_is_boss(Entity *self);

#endif
