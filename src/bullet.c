#include <limits.h>
#include <math.h>
#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>

#include "bullet.h"
#include "config.h"
#include "dmg_number.h"
#include "enemy.h"
#include "entity.h"
#include "entity_list.h"
#include "explosion.h"
#include "game.h"
#include "laser.h"
#include "physics.h"
#include "player.h"
#include "types.h"
#include "utils.h"

const char *bullet_type_names[] = {
    "Normal bullet", "Replicating", "Explosive", "Shrapnel", "Laser", "Healing",
};

/*============================================================================*
 * Bullet initialization                                                      *
 *============================================================================*/
static BulletData *bullet_init_data(Vector2 initial_velocity, BulletType type,
                                    u32 level, i32 damage, u32 special_idx) {
  BulletData *data = malloc(sizeof(BulletData));
  data->velocity = initial_velocity;
  data->size = type > BULLET_NORMAL ? 2.5f : 1.5f;

  data->type = type;
  data->level = level;
  data->damage = damage;
  data->special_idx = special_idx;

  data->deferred_destroy = false;

  return data;
}

/*============================================================================*
 * Bullet update helpers                                                      *
 *============================================================================*/

/*
 * Special bullet hit functions
 */
bool hit_replicate(Entity *self, Entity *enemy, Game game) {
  BulletData *data = (BulletData *)self->custom_data;

  f32 spawn_p = 0.25f + 0.1f * data->level;
  if ((f32)rand() / INT_MAX < spawn_p) {
    f32 vx = (f32)rand() / INT_MAX * 2.0f - 1.0f;
    f32 vy = (f32)rand() / INT_MAX * 2.0f - 1.0f;

    Vector2 direction = Vector2Normalize((Vector2){vx, vy});
    Vector2 target = Vector2Add(self->position, direction);

    Entity *new_bullet = bullet_create(self->position, target, BULLET_SECONDARY,
                                       1, data->damage, 0);
    el_add(game->world, new_bullet);
  }

  return false;
}

bool hit_explosive(Entity *self, Entity *enemy, Game game) {
  BulletData *data = (BulletData *)self->custom_data;

  f32 radius = 25 + data->level * 5;
  f32 damage = data->damage * (2 + data->level * 0.25f);
  Entity *explosion = explosion_create(self->position, radius, damage);
  el_add(game->world, explosion);

  PlayerData *pdata = (PlayerData *)game->player->custom_data;
  pdata->special_bullets[data->special_idx].fired = false;
  pdata->special_bullets[data->special_idx].cooldown = 3.0f;
  data->deferred_destroy = true;

  return true;
}

bool hit_shrapnel(Entity *self, Entity *enemy, Game game) {
  BulletData *data = (BulletData *)self->custom_data;

  for (u32 i = 0; i < data->level + 2; i++) {
    f32 vx = (f32)rand() / INT_MAX * 2.0f - 1.0f;
    f32 vy = (f32)rand() / INT_MAX * 2.0f - 1.0f;

    Vector2 direction = Vector2Normalize((Vector2){vx, vy});
    Vector2 target = Vector2Add(self->position, direction);

    Entity *new_bullet = bullet_create(self->position, target, BULLET_SECONDARY,
                                       1, data->damage, 0);
    el_add(game->world, new_bullet);
  }

  PlayerData *pdata = (PlayerData *)game->player->custom_data;
  pdata->special_bullets[data->special_idx].fired = false;
  pdata->special_bullets[data->special_idx].cooldown = 3.0f;
  data->deferred_destroy = true;

  return true;
}

bool hit_laser(Entity *self, Entity *enemy, Game game) {
  BulletData *data = (BulletData *)self->custom_data;

  f32 damage = data->damage * 0.5f;
  Entity *laser = laser_create(self->position, damage);
  el_add(game->world, laser);

  return false;
}

bool hit_healing(Entity *self, Entity *enemy, Game game) {
  BulletData *data = (BulletData *)self->custom_data;
  PlayerData *pdata = (PlayerData *)game->player->custom_data;

  f32 heal_p = 0.04 + 0.02f * data->level;
  if ((f32)rand() / INT_MAX < heal_p && pdata->health < pdata->max_health) {
    i32 heal_amount = 1 + data->level / 4;
    i32 player_damage = pdata->max_health - pdata->health;
    if (heal_amount > player_damage)
      heal_amount = player_damage;

    pdata->health += heal_amount;
    Entity *dmg_number = dmg_number_create(game->player->position, -heal_amount,
                                           DMG_NUMBER_SIZE);
    el_add(game->world, dmg_number);
  }

  return false;
}

CollisionCallback special_bullet_hit[6] = {
    [BULLET_NORMAL] = NULL,
    [BULLET_REPLICATE] = hit_replicate,
    [BULLET_EXPLOSIVE] = hit_explosive,
    [BULLET_SHRAPNEL] = hit_shrapnel,
    [BULLET_LASER] = hit_laser,
    [BULLET_HEALING] = hit_healing,
};

/*
 * Hit callback
 */
bool bullet_hit_enemy(Entity *self, Entity *enemy, Game game) {
  BulletData *data = (BulletData *)self->custom_data;
  EnemyData *edata = (EnemyData *)enemy->custom_data;

  edata->health -= data->damage;

  Entity *dmg_number =
      dmg_number_create(self->position, data->damage, DMG_NUMBER_SIZE);
  el_add(game->world, dmg_number);

  if (data->type > 0) {
    return special_bullet_hit[data->type](self, enemy, game);
  } else {
    return false;
  }
}

/*============================================================================*
 * Bullet update function                                                     *
 *============================================================================*/
static void bullet_update(Entity *self, Game game) {
  BulletData *data = (BulletData *)self->custom_data;

  // Predict next position
  Vector2 delta_pos = Vector2Scale(data->velocity, game->delta_time);
  Vector2 next_pos = Vector2Add(self->position, delta_pos);

  // Check collisions
  Collision collision =
      check_collisions(self, game, next_pos, data->size, bullet_hit_enemy);

  // Some bullets destroy themselves on hitting an enemy
  if (data->deferred_destroy) {
    el_destroy(game->world, self);
    return;
  }

  // If there was a collision change the trajectory
  if (collision.direction != COL_NONE) {
    next_pos = apply_collision(self->position, delta_pos, &data->velocity, 1,
                               collision, game);
  }

  // Update position
  self->position = next_pos;

  // Destroy the bullet when it reaches the bottom of the screen
  if (self->position.y >= FIELD_HEIGHT / 2.0f + data->size) {
    PlayerData *pdata = (PlayerData *)game->player->custom_data;

    if (data->type == BULLET_NORMAL) {
      pdata->ammo++;
    } else if (data->type != BULLET_SECONDARY) {
      pdata->special_bullets[data->special_idx].fired = false;
    }

    el_destroy(game->world, self);
  }
}

/*============================================================================*
 * Player draw function                                                       *
 *============================================================================*/
static void bullet_draw(Entity *bullet, Game game) {
  BulletData *data = (BulletData *)bullet->custom_data;

  // Draw bullet
  Vector2 screen_pos = game_to_screen(bullet->position);
  f32 screen_size = game_to_screen_scale(data->size);

  DrawCircle(screen_pos.x, screen_pos.y, screen_size, BLUE);
}

/*============================================================================*
 * Player constructor                                                         *
 *============================================================================*/
Entity *bullet_create(Vector2 position, Vector2 target, BulletType type,
                      u32 level, i32 damage, u32 special_idx) {
  Entity *bullet = ent_create(ENT_BULLET);

  bullet->position = position;

  Vector2 aim = Vector2Subtract(target, position);
  Vector2 velocity = Vector2Scale(Vector2Normalize(aim), BULLET_SPEED);
  bullet->custom_data =
      bullet_init_data(velocity, type, level, damage, special_idx);

  bullet->update = bullet_update;
  bullet->draw = bullet_draw;

  return bullet;
}
