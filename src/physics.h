#ifndef PHYSICS_H
#define PHYSICS_H

#include <raylib.h>

#include "types.h"

typedef enum {
  COL_NONE = 0,
  COL_X = 1,
  COL_Y = 2,
} CollisionDirection;

typedef struct Collision {
  CollisionDirection direction;
  f32 t;
} Collision;

Collision collide_particle_rect(Vector2 p_last, Vector2 p, f32 size,
                                Rectangle rect);

#endif
