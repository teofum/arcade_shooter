#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>

#include "assets.h"
#include "config.h"
#include "entity.h"
#include "game.h"
#include "types.h"
#include "utils.h"
#include "xp_gem.h"

u32 xp_gem_values[XP_GEM_TIERS] = {25, 5, 1};

static XpGem xp_gem_init_data(u32 value) {
  f32 vx = frand() * 2.0f - 1.0f;
  f32 vy = frand() * 2.0f - 1.0f;
  f32 speed = frand() * 0.6f + 0.7f;

  return (XpGem){
      .value = value,
      .velocity = Vector2Scale(Vector2Normalize((Vector2){vx, vy}), speed),
  };
}

bool xp_gem_update(Entity *self, Game game) {
  XpGem *data = &self->xp_gem;

  f32 delta_y = game->delta_time * ENEMY_SPEED;
  self->position.y += delta_y;

  if (self->position.y > FIELD_HEIGHT / 2.0f + 10) {
    return true;
  }

  // Accelerate towards player and move
  Vector2 target_velocity = {0, 0};
  Vector2 player_pos = game->players[0]->position;
  f32 distance = Vector2Distance(player_pos, self->position);

  if (distance < XP_PICKUP_RANGE) {
    for (u32 i = 0; i < MAX_CLIENTS; i++) {
      if (game->players_enabled[i]) {
        Player *pdata = &game->players[i]->player;
        pdata->xp += data->value;
      }
    }

    return true;
  } else if (distance < XP_MAGNET_RANGE) {
    target_velocity = Vector2Subtract(player_pos, self->position);
    target_velocity =
        Vector2Scale(target_velocity, XP_MAGNET_POWER / (distance * distance));
  }

  data->velocity = Vector2Lerp(data->velocity, target_velocity, 0.1f);
  self->position = Vector2Add(self->position, data->velocity);

  return false;
}

void xp_gem_draw(Entity *self, Game game) {
  XpGem *data = &self->xp_gem;

  u32 sprite_idx = data->value >= 25 ? 2 : data->value >= 5 ? 1 : 0;
  Sprite *sprite = &assets.xp_gems[sprite_idx];

  Vector2 p = self->position;
  f32 size = 16 / game->camera.zoom;
  Rectangle dest = (Rectangle){p.x, p.y, size, size};

  Vector2 origin = {size / 2, size / 2};

  // Draw XP item
  DrawTexturePro(sprite->texture, get_frame_rect(sprite, 0), dest, origin, 0,
                 WHITE);
}

Entity *xp_gem_create(Vector2 position, u32 value) {
  Entity *xp_gem = ent_create(ENT_XP_GEM);

  xp_gem->position = position;
  xp_gem->xp_gem = xp_gem_init_data(value);

  return xp_gem;
}
