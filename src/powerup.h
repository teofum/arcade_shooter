#ifndef POWERUP_H
#define POWERUP_H

#include <raylib.h>

#include "entity.h"
#include "game.h"

extern const char *powerup_names[2];
extern const char *powerup_descriptions[2];
extern Color powerup_colors[2];

Entity *powerup_create(Vector2 position, PowerupType type);
bool powerup_update(Entity *self, Game game);
void powerup_draw(Entity *self, Game game);

#endif
