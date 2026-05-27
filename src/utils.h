#ifndef UTILS_H
#define UTILS_H

#include <raylib.h>
#include <sys/time.h>

#include "types.h"

typedef struct Range {
  i32 min;
  i32 max;
} Range;

// Time
u64 timeval_to_ms(struct timeval *tv);
u64 now();

// RNG
f32 frand();

// Damage
i32 get_damage(i32 base_damage);
Range get_damage_range(i32 base_damage);

#endif
