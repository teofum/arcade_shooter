#include <stdlib.h>

#include "config.h"
#include "utils.h"

#define SCALE ((float)WINDOW_HEIGHT / FIELD_HEIGHT)
#define OFFSET_X ((float)WINDOW_WIDTH / 2)
#define OFFSET_Y ((float)WINDOW_HEIGHT / 2)

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
