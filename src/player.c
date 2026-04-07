#include "player.h"
#include "raylib.h"

void player_update(Entity *player, Game game) { player->position.x++; }

void player_draw(Entity *player, Game game) {
  DrawCircle(player->position.x, player->position.y, 10, RED);
}
