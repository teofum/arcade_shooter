#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>

#include "assets.h"
#include "config.h"
#include "enemy_bullet.h"
#include "entity.h"
#include "game.h"
#include "types.h"

/*============================================================================*
 * Enemy bullet initialization                                                *
 *============================================================================*/
static EnemyBullet enemy_bullet_init_data(i32 damage) {
  return (EnemyBullet){
      .size = 2.0f,
      .damage = damage,
      .deferred_destroy = false,
  };
}

/*============================================================================*
 * Enemy bullet update function                                               *
 *============================================================================*/
bool enemy_bullet_update(Entity *self, Game game) {
  EnemyBullet *data = &self->enemy_bullet;

  if (data->deferred_destroy) {
    return true;
  }

  // Update position
  Vector2 delta_pos = Vector2Scale(self->velocity, game->delta_time);
  self->position = Vector2Add(self->position, delta_pos);

  // Destroy the bullet when it goes out of bounds
  static Rectangle screen_bounds = {-FIELD_WIDTH / 2.0f, -FIELD_HEIGHT / 2.0f,
                                    FIELD_WIDTH, FIELD_HEIGHT};
  if (!CheckCollisionCircleRec(self->position, data->size, screen_bounds)) {
    return true;
  }

  return false;
}

bool enemy_bullet_update_client(Entity *self, Game game) {
  EnemyBullet *data = &self->enemy_bullet;

  // Update position
  Vector2 delta_pos = Vector2Scale(self->velocity, game->delta_time);
  self->position = Vector2Add(self->position, delta_pos);

  return false;
}

/*============================================================================*
 * Enemy bullet draw function                                                 *
 *============================================================================*/
void enemy_bullet_draw(Entity *self, Game game) {
  EnemyBullet *data = &self->enemy_bullet;

  // Draw bullet
  Vector2 screen_pos = self->position;
  f32 screen_size = data->size;

  Sprite *sprite = &assets.enemy_bullet;
  Rectangle dest = {screen_pos.x, screen_pos.y, screen_size * 2,
                    screen_size * 2};

  DrawTexturePro(sprite->texture, get_frame_rect(sprite, 0), dest,
                 (Vector2){screen_size, screen_size}, 0, WHITE);
}

/*============================================================================*
 * Enemy bullet constructor                                                   *
 *============================================================================*/
Entity *enemy_bullet_create(Vector2 position, Vector2 target, i32 damage) {
  Entity *bullet = ent_create(ENT_ENEMY_BULLET);

  Vector2 aim = Vector2Subtract(target, position);
  Vector2 velocity = Vector2Scale(Vector2Normalize(aim), ENEMY_BULLET_SPEED);

  bullet->position = position;
  bullet->velocity = velocity;
  bullet->enemy_bullet = enemy_bullet_init_data(damage);

  return bullet;
}
