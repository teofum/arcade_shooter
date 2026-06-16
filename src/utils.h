#ifndef UTILS_H
#define UTILS_H

#include <raylib.h>
#include <sys/time.h>

#include "game.h"
#include "types.h"

#define min(a, b) ((a) < (b) ? (a) : (b))

typedef struct Range {
  i32 min;
  i32 max;
} Range;

typedef struct PlayerPosition {
  Vector2 position;
  u32 idx;
} PlayerPosition;

// Time
u64 timeval_to_ms(struct timeval *tv);
u64 now();

// RNG
f32 frand();

// Damage
i32 get_damage(i32 base_damage);
Range get_damage_range(i32 base_damage);

// Player finding
PlayerPosition get_closest_player(Game game, Vector2 pos);

#endif
