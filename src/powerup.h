#ifndef POWERUP_H
#define POWERUP_H

#include <raylib.h>

#include "entity.h"

typedef enum {
  POWER_FAST,
  POWER_DMG,
  POWER_NONE = -1,
} PowerupType;

typedef struct PowerupData {
  PowerupType type;

  Vector2 velocity;
} PowerupData;

Entity *powerup_create(Vector2 position, PowerupType type);

#endif
