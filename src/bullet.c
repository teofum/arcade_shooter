#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>

#include "bullet.h"
#include "entity_list.h"
#include "types.h"

BulletData *bullet_init_data(Vector2 initial_velocity) {
  BulletData *data = malloc(sizeof(BulletData));
  data->velocity = initial_velocity;

  return data;
}

void bullet_update(Entity *bullet, Game game) {
  BulletData *data = (BulletData *)bullet->custom_data;

  bullet->position = Vector2Add(bullet->position, data->velocity);

  // Destory the bullet once it gets too far from the player. For testing!
  f32 distance = Vector2Distance(bullet->position, game->player->position);
  if (distance >= 300) {
    el_destroy(game->world, bullet);
  }
}

void bullet_draw(Entity *bullet, Game game) {
  BulletData *data = (BulletData *)bullet->custom_data;

  // Draw player
  DrawCircle(bullet->position.x, bullet->position.y, 5, BLUE);
}
