#include <math.h>
#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>

#include "bullet.h"
#include "config.h"
#include "entity.h"
#include "entity_list.h"
#include "player.h"

static PlayerData *player_init_data() {
  PlayerData *data = malloc(sizeof(PlayerData));

  data->velocity = (Vector2){0, 0};
  data->direction = (Vector2){0, 0};
  data->crosshair = (Vector2){0, 0};

  data->ammo = data->max_ammo = 50;
  data->fire_cooldown = 0.1f;
  data->fire_timer = 0.0f;
  data->firing = false;

  return data;
}

Entity *player_create() {
  Entity *player = ent_create(ENT_PLAYER);

  player->position = (Vector2){WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f};
  player->custom_data = player_init_data();
  player->update = player_update;
  player->draw = player_draw;

  return player;
}

void player_update(Entity *player, Game game) {
  PlayerData *data = (PlayerData *)player->custom_data;

  // Update position and velocity
  Vector2 target_velocity = Vector2Scale(data->direction, PLAYER_SPEED);
  data->velocity = Vector2Lerp(data->velocity, target_velocity, PLAYER_ACCEL);

  player->position = Vector2Add(player->position, data->velocity);

  // Spawn a bullet
  if (data->firing) {
    if (data->fire_timer == 0.0f && data->ammo > 0) {
      Entity *bullet = bullet_create(player->position, data->crosshair);
      el_add(game->world, bullet);

      data->fire_timer = data->fire_cooldown;
      data->ammo--;
    }
  }

  // Update firing timer
  data->fire_timer = fmaxf(data->fire_timer - game->delta_time, 0.0f);
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
