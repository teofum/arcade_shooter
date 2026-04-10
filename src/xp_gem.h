#ifndef XP_GEM_H
#define XP_GEM_H

#include <raylib.h>

#include "entity.h"
#include "types.h"

typedef struct XpGemData {
  u32 value;

  Vector2 velocity;
} XpGemData;

Entity *xp_gem_create(Vector2 position, u32 value);

#endif
