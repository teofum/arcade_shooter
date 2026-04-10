#include <limits.h>
#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>

#include "config.h"
#include "entity.h"
#include "entity_list.h"
#include "game.h"
#include "player.h"
#include "types.h"
#include "utils.h"
#include "xp_gem.h"

static XpGemData *xp_gem_init_data(u32 value) {
  XpGemData *data = malloc(sizeof(XpGemData));
  data->value = value;

  f32 vx = (f32)rand() / INT_MAX * 2.0f - 1.0f;
  f32 vy = (f32)rand() / INT_MAX * 2.0f - 1.0f;

  data->velocity = Vector2Scale(Vector2Normalize((Vector2){vx, vy}), 1.0);

  return data;
}

static void xp_gem_update(Entity *xp_gem, Game game) {
  XpGemData *data = (XpGemData *)xp_gem->custom_data;

  f32 delta_y = game->delta_time * ENEMY_SPEED;
  xp_gem->position.y += delta_y;

  if (xp_gem->position.y > FIELD_HEIGHT / 2.0f + 10) {
    el_destroy(game->world, xp_gem);
    return;
  }

  // Accelerate towards player and move
  Vector2 target_velocity = {0, 0};
  Vector2 player_pos = game->player->position;
  f32 distance = Vector2Distance(player_pos, xp_gem->position);

  if (distance < XP_PICKUP_RANGE) {
    PlayerData *pdata = (PlayerData *)game->player->custom_data;
    pdata->xp += data->value;

    el_destroy(game->world, xp_gem);
    return;
  } else if (distance < XP_MAGNET_RANGE) {
    target_velocity = Vector2Subtract(player_pos, xp_gem->position);
    target_velocity =
        Vector2Scale(target_velocity, XP_MAGNET_POWER / (distance * distance));
  }

  data->velocity = Vector2Lerp(data->velocity, target_velocity, 0.1f);
  xp_gem->position = Vector2Add(xp_gem->position, data->velocity);
}

static void xp_gem_draw(Entity *xp_gem, Game game) {
  XpGemData *data = (XpGemData *)xp_gem->custom_data;

  DrawCircle(game_to_screen_x(xp_gem->position.x),
             game_to_screen_y(xp_gem->position.y), game_to_screen_scale(1.0f),
             MAGENTA);
}

Entity *xp_gem_create(Vector2 position, u32 value) {
  Entity *xp_gem = ent_create(ENT_XP_GEM);

  xp_gem->position = position;
  xp_gem->custom_data = xp_gem_init_data(value);

  xp_gem->update = xp_gem_update;
  xp_gem->draw = xp_gem_draw;

  return xp_gem;
}
