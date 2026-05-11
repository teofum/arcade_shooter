#include <raylib.h>
#include <stdlib.h>

#include "entity.h"
#include "game.h"
#include "wall.h"

static WallData *wall_init_data(Entity *self, Rectangle bounds) {
  WallData *data = &self->wall;
  data->bounds = bounds;

  return data;
}

static void wall_update(Entity *wall, Game game) {
  // Do nothing. It's a wall.
}

static void wall_draw(Entity *self, Game game) {
  WallData *data = &self->wall;

  // Draw wall
  DrawRectangleRec(data->bounds, (Color){0, 64, 64, 255});
}

Entity *wall_create(Rectangle bounds) {
  Entity *wall = ent_create(ENT_WALL);

  wall_init_data(wall, bounds);

  wall->update = wall_update;
  wall->draw = wall_draw;

  return wall;
}
