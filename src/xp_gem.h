#ifndef XP_GEM_H
#define XP_GEM_H

#include <raylib.h>

#include "entity.h"
#include "game.h"
#include "types.h"

#define XP_GEM_TIERS 3

extern u32 xp_gem_values[XP_GEM_TIERS];

Entity *xp_gem_create(Vector2 position, u32 value);
bool xp_gem_update(Entity *self, Game game);
bool xp_gem_update_client(Entity *self, Game game);
void xp_gem_draw(Entity *self, Game game);

#endif
