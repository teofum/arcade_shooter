#include <limits.h>
#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>

#include "config.h"
#include "entity.h"
#include "entity_list.h"
#include "game.h"
#include "player.h"
#include "powerup.h"
#include "types.h"
#include "utils.h"

static PowerupData *powerup_init_data(PowerupType type) {
  PowerupData *data = malloc(sizeof(PowerupData));
  data->type = type;

  f32 vx = (f32)rand() / INT_MAX * 2.0f - 1.0f;
  f32 vy = (f32)rand() / INT_MAX * 2.0f - 1.0f;
  f32 speed = (f32)rand() / INT_MAX * 0.6f + 0.7f;

  data->velocity = Vector2Scale(Vector2Normalize((Vector2){vx, vy}), speed);

  return data;
}

static void powerup_update(Entity *powerup, Game game) {
  PowerupData *data = (PowerupData *)powerup->custom_data;

  f32 delta_y = game->delta_time * ENEMY_SPEED;
  powerup->position.y += delta_y;

  if (powerup->position.y > FIELD_HEIGHT / 2.0f + 10) {
    el_destroy(game->world, powerup);
    return;
  }

  // Accelerate towards player and move
  Vector2 target_velocity = {0, 0};
  Vector2 player_pos = game->player->position;
  f32 distance = Vector2Distance(player_pos, powerup->position);

  if (distance < XP_PICKUP_RANGE) {
    PlayerData *pdata = (PlayerData *)game->player->custom_data;
    pdata->active_powerup = data->type;
    pdata->powerup_timer = POWERUP_DURATION;

    el_destroy(game->world, powerup);
    return;
  } else if (distance < XP_MAGNET_RANGE) {
    target_velocity = Vector2Subtract(player_pos, powerup->position);
    target_velocity =
        Vector2Scale(target_velocity, XP_MAGNET_POWER / (distance * distance));
  }

  data->velocity = Vector2Lerp(data->velocity, target_velocity, 0.1f);
  powerup->position = Vector2Add(powerup->position, data->velocity);
}

static void powerup_draw(Entity *powerup, Game game) {
  PowerupData *data = (PowerupData *)powerup->custom_data;

  DrawCircle(game_to_screen_x(powerup->position.x),
             game_to_screen_y(powerup->position.y), game_to_screen_scale(2.0f),
             data->type == POWER_FAST ? GREEN : ORANGE);
}

Entity *powerup_create(Vector2 position, PowerupType type) {
  Entity *powerup = ent_create(ENT_POWERUP);

  powerup->position = position;
  powerup->custom_data = powerup_init_data(type);

  powerup->update = powerup_update;
  powerup->draw = powerup_draw;

  return powerup;
}
