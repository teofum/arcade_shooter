#ifndef XP_GEM_H
#define XP_GEM_H

#include <raylib.h>

#include "entity.h"
#include "types.h"

#define XP_GEM_TIERS 3

extern u32 xp_gem_values[XP_GEM_TIERS];

Entity *xp_gem_create(Vector2 position, u32 value);

#endif
