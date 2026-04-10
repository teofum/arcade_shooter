#ifndef DMG_NUMBER_H
#define DMG_NUMBER_H

#include <raylib.h>

#include "entity.h"
#include "types.h"

typedef struct DmgNumberData {
  char string[10];
  f32 size;
  f32 timer;
  f32 speed;
} DmgNumberData;

Entity *dmg_number_create(Vector2 position, i32 dmg, f32 size);

#endif
