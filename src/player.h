#ifndef PLAYER_H
#define PLAYER_H

#include "entity.h"
#include "game.h"
#include "types.h"

typedef struct PlayerData {
  f32 size;

  Vector2 velocity;
  Vector2 crosshair;

  i32 max_health;
  i32 health;

  u32 max_ammo;
  u32 ammo;

  f32 fire_cooldown;
  f32 fire_timer;

  // Input
  Vector2 direction;
  bool firing;
} PlayerData;

Entity *player_create();

void player_update(Entity *player, Game game);

void player_draw(Entity *player, Game game);

#endif
