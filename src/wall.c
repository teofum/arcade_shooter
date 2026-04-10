#include <raylib.h>
#include <stdlib.h>

#include "entity.h"
#include "game.h"
#include "utils.h"
#include "wall.h"

static WallData *wall_init_data(Rectangle bounds) {
  WallData *data = malloc(sizeof(WallData));
  data->bounds = bounds;

  return data;
}

static void wall_update(Entity *wall, Game game) {
  // Do nothing. It's a wall.
}

static void wall_draw(Entity *wall, Game game) {
  WallData *data = (WallData *)wall->custom_data;

  // Draw player
  DrawRectangleRec(game_to_screen_rect(data->bounds), GREEN);
}

Entity *wall_create(Rectangle bounds) {
  Entity *wall = ent_create(ENT_WALL);

  wall->custom_data = wall_init_data(bounds);

  wall->update = wall_update;
  wall->draw = wall_draw;

  return wall;
}
