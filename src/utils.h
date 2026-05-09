#ifndef UTILS_H
#define UTILS_H

#include <raylib.h>

#include "types.h"

typedef struct Range {
  i32 min;
  i32 max;
} Range;

// RNG
f32 frand();

// Damage
i32 get_damage(i32 base_damage);
Range get_damage_range(i32 base_damage);

#endif
