#include <math.h>
#include <raylib.h>
#include <raymath.h>

#include "enemy.h"
#include "entity.h"
#include "game.h"
#include "physics.h"
#include "types.h"
#include "wall.h"

/*
 * Simple collision detection
 */
Collision collide_particle_rect(Vector2 p_last, Vector2 p, f32 size,
                                Rectangle rect) {
  Collision collision = {
      .direction = COL_NONE,
      .t = INFINITY,
  };

  f32 c_top = rect.y - size;
  f32 c_left = rect.x - size;
  f32 c_bottom = rect.y + rect.height + size;
  f32 c_right = rect.x + rect.width + size;

  if (p_last.y <= c_top && p.y > c_top) {
    f32 t = (c_top - p_last.y) / (p.y - p_last.y);
    f32 x = p_last.x * (1 - t) + p.x * t;

    if (c_left <= x && x <= c_right && t <= collision.t) {
      collision.direction |= COL_Y;
      collision.t = t;
    }
  }

  if (p_last.x <= c_left && p.x > c_left) {
    f32 t = (c_left - p_last.x) / (p.x - p_last.x);
    f32 y = p_last.y * (1 - t) + p.y * t;

    if (c_top <= y && y <= c_bottom && t <= collision.t) {
      collision.direction |= COL_X;
      collision.t = t;
    }
  }

  if (p_last.y >= c_bottom && p.y < c_bottom) {
    f32 t = (c_bottom - p_last.y) / (p.y - p_last.y);
    f32 x = p_last.x * (1 - t) + p.x * t;

    if (c_left <= x && x <= c_right && t <= collision.t) {
      collision.direction |= COL_Y;
      collision.t = t;
    }
  }

  if (p_last.x >= c_right && p.x < c_right) {
    f32 t = (c_right - p_last.x) / (p.x - p_last.x);
    f32 y = p_last.y * (1 - t) + p.y * t;

    if (c_top <= y && y <= c_bottom && t <= collision.t) {
      collision.direction |= COL_X;
      collision.t = t;
    }
  }

  return collision;
}

/*
 * Get collision bounds for a wall or enemy. Assumes entity is one of these
 * two types.
 */
static Rectangle get_collision_bounds(Entity *entity) {
  if (entity->type == ENT_WALL) {
    WallData *wdata = (WallData *)entity->custom_data;
    return wdata->bounds;
  } else {
    EnemyData *edata = (EnemyData *)entity->custom_data;
    return (Rectangle){entity->position.x, entity->position.y, edata->size.x,
                       edata->size.y};
  }
}

/*
 * Check collisions with walls/enemies, optionally triggering something on hit
 */
Collision check_collisions(Entity *self, Game game, Vector2 next_pos, f32 size,
                           CollisionCallback on_enemy_hit) {
  // Check for collisions with walls
  Collision collision = {
      .direction = COL_NONE,
      .t = INFINITY,
  };
  EntityListIterator it = el_iter(game->world);
  Entity *entity;
  while ((entity = eli_next(&it))) {
    if (entity->type == ENT_WALL || entity->type == ENT_ENEMY) {
      Rectangle rect = get_collision_bounds(entity);

      Collision c = collide_particle_rect(self->position, next_pos, size, rect);
      collision.direction |= c.direction;
      collision.t = fminf(collision.t, c.t);

      if (c.direction != COL_NONE && entity->type == ENT_ENEMY &&
          on_enemy_hit != NULL) {
        if (on_enemy_hit(self, entity, game))
          break;
      }
    }
  }

  return collision;
}

Vector2 apply_collision(Vector2 pos, Vector2 delta_pos, Vector2 *velocity,
                        f32 elasticity, Collision collision, Game game) {
  // 1. Move in the original direction up to the point of collision
  delta_pos = Vector2Scale(delta_pos, collision.t);
  pos = Vector2Add(pos, delta_pos);

  // 2. Modify velocity (fully plastic collision, ie wallslide)
  if (collision.direction & COL_X)
    velocity->x *= -elasticity;
  if (collision.direction & COL_Y)
    velocity->y *= -elasticity;

  // 3. Move the rest of the way
  delta_pos = Vector2Scale(*velocity, game->delta_time * (1 - collision.t));
  return Vector2Add(pos, delta_pos);
}
