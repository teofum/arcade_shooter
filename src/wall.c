#include <raylib.h>
#include <stdlib.h>

#include "entity.h"
#include "game.h"
#include "wall.h"

void wall_draw(Entity *self, Game game) {
  Wall *data = &self->wall;

  // Draw wall
  DrawRectangleRec(data->bounds, (Color){0, 64, 64, 255});
}

Entity *wall_create(Rectangle bounds) {
  Entity *wall = ent_create(ENT_WALL);
  wall->wall = (Wall){.bounds = bounds};

  return wall;
}
