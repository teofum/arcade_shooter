#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>

#include "bullet.h"
#include "config.h"
#include "entity.h"
#include "entity_list.h"
#include "player.h"
#include "types.h"

static BulletData *bullet_init_data(Vector2 initial_velocity) {
  BulletData *data = malloc(sizeof(BulletData));
  data->velocity = initial_velocity;

  return data;
}

Entity *bullet_create(Vector2 position, Vector2 target) {
  Entity *bullet = ent_create(ENT_BULLET);

  bullet->position = position;

  Vector2 aim = Vector2Subtract(target, position);
  Vector2 velocity = Vector2Scale(Vector2Normalize(aim), BULLET_SPEED);
  bullet->custom_data = bullet_init_data(velocity);

  bullet->update = bullet_update;
  bullet->draw = bullet_draw;

  return bullet;
}

void bullet_update(Entity *bullet, Game game) {
  BulletData *data = (BulletData *)bullet->custom_data;

  bullet->position = Vector2Add(bullet->position, data->velocity);

  // Destory the bullet once it gets too far from the player. For testing!
  f32 distance = Vector2Distance(bullet->position, game->player->position);
  if (distance >= 500) {
    PlayerData *pdata = (PlayerData *)game->player->custom_data;
    pdata->ammo++;
    el_destroy(game->world, bullet);
  }
}

void bullet_draw(Entity *bullet, Game game) {
  BulletData *data = (BulletData *)bullet->custom_data;

  // Draw player
  DrawCircle(bullet->position.x, bullet->position.y, 5, BLUE);
}
