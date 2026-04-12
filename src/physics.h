#ifndef PHYSICS_H
#define PHYSICS_H

#include <raylib.h>

#include "entity.h"
#include "game.h"
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

typedef bool (*CollisionCallback)(Entity *self, Entity *enemy, Game game);

Collision collide_particle_rect(Vector2 p_last, Vector2 p, f32 size,
                                Rectangle rect);

Collision check_collisions(Entity *self, Game game, Vector2 next_pos, f32 size,
                           CollisionCallback on_enemy_hit);

Vector2 apply_collision(Vector2 pos, Vector2 delta_pos, Vector2 *velocity,
                        f32 elasticity, Collision collision, Game game);

#endif
