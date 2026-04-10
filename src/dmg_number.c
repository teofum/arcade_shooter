#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "dmg_number.h"
#include "entity.h"
#include "entity_list.h"
#include "game.h"
#include "utils.h"

static DmgNumberData *dmg_number_init_data(i32 damage, f32 size) {
  DmgNumberData *data = malloc(sizeof(DmgNumberData));

  sprintf(data->string, "%d", damage);
  data->size = size;
  data->timer = DMG_NUMBER_TTL;
  data->speed = DMG_NUMBER_SPEED;

  return data;
}

static void dmg_number_update(Entity *dmg_number, Game game) {
  DmgNumberData *data = (DmgNumberData *)dmg_number->custom_data;

  if (data->timer <= 0.0f) {
    el_destroy(game->world, dmg_number);
  } else {
    data->timer -= game->delta_time;
    dmg_number->position.y -= data->speed * game->delta_time;
    data->speed += DMG_NUMBER_ACCEL * game->delta_time;
  }
}

static void dmg_number_draw(Entity *dmg_number, Game game) {
  DmgNumberData *data = (DmgNumberData *)dmg_number->custom_data;

  // Draw number
  f32 w = MeasureText(data->string, data->size);
  f32 h = data->size;
  DrawText(data->string, game_to_screen_x(dmg_number->position.x) - w / 2,
           game_to_screen_y(dmg_number->position.y) - h / 2, data->size, BLACK);
}

Entity *dmg_number_create(Vector2 position, i32 dmg, f32 size) {
  Entity *dmg_number = ent_create(ENT_DMG_NUMBER);

  dmg_number->position = position;
  dmg_number->custom_data = dmg_number_init_data(dmg, size);
  dmg_number->update = dmg_number_update;
  dmg_number->draw = dmg_number_draw;

  return dmg_number;
}
