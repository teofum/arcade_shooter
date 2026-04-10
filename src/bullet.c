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
#include "game.h"
#include "physics.h"
#include "player.h"
#include "types.h"
#include "utils.h"
#include "wall.h"

static BulletData *bullet_init_data(Vector2 initial_velocity) {
  BulletData *data = malloc(sizeof(BulletData));
  data->velocity = initial_velocity;
  data->size = 1.5f;

  data->damage = 10;

  return data;
}

static void bullet_update(Entity *bullet, Game game) {
  BulletData *data = (BulletData *)bullet->custom_data;

  Vector2 delta_pos = Vector2Scale(data->velocity, game->delta_time);
  Vector2 next_pos = Vector2Add(bullet->position, delta_pos);

  // Check collisions
  Collision collision = {
      .direction = COL_NONE,
      .t = INFINITY,
  };
  EntityListIterator it = el_iter(game->world);
  Entity *e;
  while ((e = eli_next(&it))) {
    if (e->type == ENT_WALL) {
      WallData *wdata = (WallData *)e->custom_data;

      Collision c = collide_particle_rect(bullet->position, next_pos,
                                          data->size, wdata->bounds);
      collision.direction |= c.direction;
      collision.t = fminf(collision.t, c.t);
    } else if (e->type == ENT_ENEMY) {
      EnemyData *edata = (EnemyData *)e->custom_data;

      Rectangle bounds = {e->position.x, e->position.y, edata->size.x,
                          edata->size.y};
      Collision c =
          collide_particle_rect(bullet->position, next_pos, data->size, bounds);
      collision.direction |= c.direction;
      collision.t = fminf(collision.t, c.t);

      // Damage enemy and spawn a dopamine inducing number
      if (c.direction != COL_NONE) {
        edata->health -= data->damage;

        Vector2 dmg_pos =
            Vector2Add(e->position, Vector2Scale(edata->size, 0.5f));
        Entity *dmg_number =
            dmg_number_create(dmg_pos, data->damage, DMG_NUMBER_SIZE);
        el_add(game->world, dmg_number);
      }
    }
  }

  // If there was a collision change the trajectory
  if (collision.direction != COL_NONE) {
    // 1. Move in the original direction up to the point of collision
    delta_pos = Vector2Scale(delta_pos, collision.t);
    next_pos = Vector2Add(bullet->position, delta_pos);

    // 2. Modify velocity
    if (collision.direction & COL_X)
      data->velocity.x *= -1;
    if (collision.direction & COL_Y)
      data->velocity.y *= -1;

    // 3. Move the rest of the way
    delta_pos =
        Vector2Scale(data->velocity, game->delta_time * (1 - collision.t));
    next_pos = Vector2Add(next_pos, delta_pos);
  }

  // Update position
  bullet->position = next_pos;

  // Destroy the bullet when it reaches the bottom of the screen
  if (bullet->position.y >= 100) {
    PlayerData *pdata = (PlayerData *)game->player->custom_data;
    pdata->ammo++;
    el_destroy(game->world, bullet);
  }
}

static void bullet_draw(Entity *bullet, Game game) {
  BulletData *data = (BulletData *)bullet->custom_data;

  // Draw bullet
  Vector2 screen_pos = game_to_screen(bullet->position);
  f32 screen_size = game_to_screen_scale(data->size);

  DrawCircle(screen_pos.x, screen_pos.y, screen_size, BLUE);
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
