#include <math.h>
#include <stdlib.h>

#include "config.h"
#include "raylib.h"
#include "raymath.h"
#include "utils.h"

#define SCALE ((float)WINDOW_HEIGHT / FIELD_HEIGHT)
#define OFFSET_X ((float)WINDOW_WIDTH / 2)
#define OFFSET_Y ((float)WINDOW_HEIGHT / 2)

/*
 * Time
 */
u64 timeval_to_ms(struct timeval *tv) {
  return tv->tv_sec * 1000 + tv->tv_usec / 1000;
}

u64 now() {
  static struct timeval tv;
  gettimeofday(&tv, NULL);
  return timeval_to_ms(&tv);
}

/*
 * RNG
 */
f32 frand() { return (f32)rand() / RAND_MAX; }

/*
 * Damage calculation
 */
i32 get_damage(i32 base_damage) {
  i32 variation = base_damage > 10 ? base_damage / 5 : 2;
  return base_damage - variation / 2 + rand() % (variation + 1);
}

Range get_damage_range(i32 base_damage) {
  i32 variation = base_damage > 10 ? base_damage / 5 : 2;
  i32 min = base_damage - variation / 2;

  return (Range){min, min + variation};
}

PlayerPosition get_closest_player(Game game, Vector2 pos) {
  PlayerPosition closest = {.position = {0}, .idx = -1};
  f32 closest_d = INFINITY;

  for (u32 i = 0; i < MAX_CLIENTS; i++) {
    if (game->players_enabled[i]) {
      Vector2 player_pos = game->players[i]->position;
      f32 d = Vector2DistanceSqr(pos, player_pos);
      if (d < closest_d) {
        closest_d = d;
        closest = (PlayerPosition){.position = player_pos, .idx = i};
      }
    }
  }

  return closest;
}
