#include <math.h>
#include <raylib.h>
#include <raymath.h>

#include "physics.h"

// Simple collision detection
Collision collide_particle_rect(Vector2 p_last, Vector2 p, f32 size,
                                Rectangle rect) {
  Collision collision = {
      .direction = COL_NONE,
      .t = INFINITY,
  };

  f32 c_top = rect.y - size;
  if (p_last.y <= c_top && p.y > c_top) {
    f32 t = (c_top - p_last.y) / (p.y - p_last.y);
    if (t <= collision.t) {
      collision.direction |= COL_Y;
      collision.t = t;
    }
  }

  f32 c_left = rect.x - size;
  if (p_last.x <= c_left && p.x > c_left) {
    f32 t = (c_left - p_last.x) / (p.x - p_last.x);
    if (t <= collision.t) {
      collision.direction |= COL_X;
      collision.t = t;
    }
  }

  f32 c_bottom = rect.y + rect.height + size;
  if (p_last.y >= c_bottom && p.y < c_bottom) {
    f32 t = (c_bottom - p_last.y) / (p.y - p_last.y);
    if (t <= collision.t) {
      collision.direction |= COL_Y;
      collision.t = t;
    }
  }

  f32 c_right = rect.x + rect.width + size;
  if (p_last.x >= c_right && p.x < c_right) {
    f32 t = (c_right - p_last.x) / (p.x - p_last.x);
    if (t <= collision.t) {
      collision.direction |= COL_X;
      collision.t = t;
    }
  }

  return collision;
}
