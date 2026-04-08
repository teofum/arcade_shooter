#ifndef PLAYER_H
#define PLAYER_H

#include "entity.h"
#include "game.h"

typedef struct PlayerData {
  Vector2 velocity;
  Vector2 crosshair;
  bool fire;
} PlayerData;

PlayerData *player_init_data();

void player_update(Entity *player, Game game);

void player_draw(Entity *player, Game game);

#endif
