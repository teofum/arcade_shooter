#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>

#include "player.h"

PlayerData *player_init_data() {
  PlayerData *data = malloc(sizeof(PlayerData));

  data->velocity = (Vector2){0, 0};
  data->crosshair = (Vector2){0, 0};

  return data;
}

void player_update(Entity *player, Game game) {
  PlayerData *data = (PlayerData *)player->custom_data;

  player->position = Vector2Add(player->position, data->velocity);
}

void player_draw(Entity *player, Game game) {
  PlayerData *data = (PlayerData *)player->custom_data;

  // Draw player
  DrawCircle(player->position.x, player->position.y, 15, RED);

  // Draw targeting crosshairs
  DrawRectangle(data->crosshair.x - 5, data->crosshair.y - 1, 11, 3, BLACK);
  DrawRectangle(data->crosshair.x - 1, data->crosshair.y - 5, 3, 11, BLACK);

  // Draw debug aim line
  Vector2 aim = Vector2Subtract(data->crosshair, player->position);
  aim = Vector2Scale(Vector2Normalize(aim), 40);
  aim = Vector2Add(aim, player->position);

  DrawLine(player->position.x, player->position.y, aim.x, aim.y, BLUE);
}
