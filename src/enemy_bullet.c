#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>

#include "assets.h"
#include "config.h"
#include "dmg_number.h"
#include "enemy_bullet.h"
#include "entity.h"
#include "entity_list.h"
#include "game.h"
#include "player.h"
#include "types.h"
#include "utils.h"

/*============================================================================*
 * Enemy bullet initialization                                                *
 *============================================================================*/
static EnemyBulletData *enemy_bullet_init_data(Vector2 initial_velocity,
                                               i32 damage) {
  EnemyBulletData *data = malloc(sizeof(EnemyBulletData));
  data->velocity = initial_velocity;
  data->size = 2.0f;

  data->damage = damage;
  return data;
}

/*============================================================================*
 * Enemy bullet update helpers                                                *
 *============================================================================*/

/*
 * Hit callback
 */
static void bullet_hit_player(Entity *self, Game game) {
  EnemyBulletData *data = (EnemyBulletData *)self->custom_data;
  PlayerData *pdata = (PlayerData *)game->player->custom_data;

  i32 damage = get_damage(data->damage);
  pdata->health -= damage;

  Entity *dmg_number =
      dmg_number_create(self->position, damage, DMG_NUMBER_SIZE);
  el_add(game->world, dmg_number);
}

/*============================================================================*
 * Enemy bullet update function                                               *
 *============================================================================*/
static void bullet_update(Entity *self, Game game) {
  EnemyBulletData *data = (EnemyBulletData *)self->custom_data;

  // Update position
  Vector2 delta_pos = Vector2Scale(data->velocity, game->delta_time);
  self->position = Vector2Add(self->position, delta_pos);

  // Destroy the bullet when it goes out of bounds
  static Rectangle screen_bounds = {-FIELD_WIDTH / 2.0f, -FIELD_HEIGHT / 2.0f,
                                    FIELD_WIDTH, FIELD_HEIGHT};
  if (!CheckCollisionCircleRec(self->position, data->size, screen_bounds)) {
    el_destroy(game->world, self);
  }
}

/*============================================================================*
 * Enemy bullet draw function                                                 *
 *============================================================================*/
static void bullet_draw(Entity *bullet, Game game) {
  EnemyBulletData *data = (EnemyBulletData *)bullet->custom_data;

  // Draw bullet
  Vector2 screen_pos = game_to_screen(bullet->position);
  f32 screen_size = game_to_screen_scale(data->size);

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

  bullet->position = position;

  Vector2 aim = Vector2Subtract(target, position);
  Vector2 velocity = Vector2Scale(Vector2Normalize(aim), ENEMY_BULLET_SPEED);
  bullet->custom_data = enemy_bullet_init_data(velocity, damage);

  bullet->update = bullet_update;
  bullet->draw = bullet_draw;

  return bullet;
}
