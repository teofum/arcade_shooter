#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>

#include "assets.h"
#include "config.h"
#include "entity.h"
#include "entity_list.h"
#include "game.h"
#include "player.h"
#include "powerup.h"
#include "types.h"
#include "utils.h"

const char *powerup_names[] = {"Overclock", "War Drum"};
const char *powerup_descriptions[] = {"Double movement speed", "Double damage"};
Color powerup_colors[] = {GREEN, ORANGE};

static PowerupData *powerup_init_data(Entity *self, PowerupType type) {
  PowerupData *data = &self->powerup;
  data->type = type;

  f32 vx = frand() * 2.0f - 1.0f;
  f32 vy = frand() * 2.0f - 1.0f;
  f32 speed = frand() * 0.6f + 0.7f;

  data->velocity = Vector2Scale(Vector2Normalize((Vector2){vx, vy}), speed);

  return data;
}

static void powerup_update(Entity *self, Game game) {
  PowerupData *data = &self->powerup;

  f32 delta_y = game->delta_time * ENEMY_SPEED;
  self->position.y += delta_y;

  if (self->position.y > FIELD_HEIGHT / 2.0f + 10) {
    el_destroy(game->world, self);
    return;
  }

  // Accelerate towards player and move
  Vector2 target_velocity = {0, 0};
  Vector2 player_pos = game->player->position;
  f32 distance = Vector2Distance(player_pos, self->position);

  if (distance < XP_PICKUP_RANGE) {
    PlayerData *pdata = &game->player->player;
    pdata->active_powerup = data->type;
    pdata->powerup_timer = POWERUP_DURATION;

    el_destroy(game->world, self);
    return;
  } else if (distance < XP_MAGNET_RANGE) {
    target_velocity = Vector2Subtract(player_pos, self->position);
    target_velocity =
        Vector2Scale(target_velocity, XP_MAGNET_POWER / (distance * distance));
  }

  data->velocity = Vector2Lerp(data->velocity, target_velocity, 0.1f);
  self->position = Vector2Add(self->position, data->velocity);
}

static void powerup_draw(Entity *self, Game game) {
  PowerupData *data = &self->powerup;

  Sprite *sprite = &assets.powerups[data->type];

  Vector2 p = self->position;
  f32 size = 16 / game->camera.zoom;
  Rectangle dest = (Rectangle){p.x, p.y, size, size};

  Vector2 origin = {size / 2, size / 2};

  DrawTexturePro(sprite->texture, get_frame_rect(sprite, 0), dest, origin, 0,
                 WHITE);
}

Entity *powerup_create(Vector2 position, PowerupType type) {
  Entity *powerup = ent_create(ENT_POWERUP);

  powerup->position = position;
  powerup_init_data(powerup, type);

  powerup->update = powerup_update;
  powerup->draw = powerup_draw;

  return powerup;
}
