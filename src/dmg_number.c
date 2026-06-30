#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "dmg_number.h"
#include "entity.h"
#include "game.h"

static DmgNumber dmg_number_init_data(i32 damage, f32 size) {
  DmgNumber data = {
      .size = size,
      .damage = damage,
      .timer = DMG_NUMBER_TTL,
      .speed = DMG_NUMBER_SPEED,
  };
  snprintf(data.string, 10, damage < 0 ? "+%d" : "%d", abs(damage));

  return data;
}

bool dmg_number_update(Entity *self, Game game) {
  DmgNumber *data = &self->dmg_number;

  if (data->timer <= 0.0f) {
    return true;
  } else {
    data->timer -= game->delta_time;
    self->position.y -= data->speed * game->delta_time;
    data->speed += DMG_NUMBER_ACCEL * game->delta_time;
  }

  return false;
}

bool dmg_number_update_client(Entity *self, Game game) {
  DmgNumber *data = &self->dmg_number;

  if (data->timer > 0.0f) {
    self->position.y -= data->speed * game->delta_time;
    data->speed += DMG_NUMBER_ACCEL * game->delta_time;
  }

  return false;
}

void dmg_number_draw(Entity *self, Game game) {
  DmgNumber *data = &self->dmg_number;

  // Draw number
  // Temporarily disable the camera and manually scale the text coords,
  // because drawing text with the camera causes odd behavior
  EndMode2D();
  {
    f32 w = MeasureText(data->string, data->size);
    f32 h = data->size;
    Matrix m = GetCameraMatrix2D(game->camera);
    Vector2 pos = Vector2Transform(self->position, m);

    DrawText(data->string, pos.x - w / 2, pos.y - h / 2, h,
             data->damage > 0 ? RED : GREEN);
  }
  BeginMode2D(game->camera);
}

Entity *dmg_number_create(Vector2 position, i32 dmg, f32 size) {
  Entity *dmg_number = ent_create(ENT_DMG_NUMBER);

  dmg_number->position = position;
  dmg_number->dmg_number = dmg_number_init_data(dmg, size);

  return dmg_number;
}
