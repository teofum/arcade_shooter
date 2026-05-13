#ifndef DMG_NUMBER_H
#define DMG_NUMBER_H

#include <raylib.h>

#include "entity.h"
#include "game.h"
#include "types.h"

Entity *dmg_number_create(Vector2 position, i32 dmg, f32 size);
bool dmg_number_update(Entity *self, Game game);
void dmg_number_draw(Entity *self, Game game);

#endif
