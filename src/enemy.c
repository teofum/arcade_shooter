#include <limits.h>
#include <math.h>
#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>

#include "bullet.h"
#include "config.h"
#include "enemy.h"
#include "entity.h"
#include "entity_list.h"
#include "game.h"
#include "player.h"
#include "powerup.h"
#include "utils.h"
#include "xp_gem.h"

static EnemyData *enemy_init_data(Vector2 size) {
  EnemyData *data = malloc(sizeof(EnemyData));
  data->size = size;

  data->health = data->max_health = 100;
  data->damage = 20;

  return data;
}

static void enemy_update(Entity *self, Game game) {
  EnemyData *data = (EnemyData *)self->custom_data;

  // Die
  if (data->health <= 0) {
    Vector2 gem_pos =
        Vector2Add(self->position, Vector2Scale(data->size, 0.5f));
    do {
      Entity *xp_gem = xp_gem_create(gem_pos, 1);
      el_add(game->world, xp_gem);
    } while ((f32)rand() / INT_MAX < 0.2);

    if ((f32)rand() / INT_MAX < POWERUP_SPAWN_PROB) {
      Entity *powerup = powerup_create(gem_pos, rand() % 2);
      el_add(game->world, powerup);
    }

    el_destroy(game->world, self);
    return;
  }

  // Move, pushing the player and bullets if it collides
  f32 delta_y = game->delta_time * ENEMY_SPEED;
  self->position.y += delta_y;

  PlayerData *pdata = (PlayerData *)game->player->custom_data;
  Vector2 ppos = game->player->position;

  f32 top = self->position.y - pdata->size;
  f32 bottom = self->position.y + data->size.y + pdata->size;
  f32 left = self->position.x - pdata->size;
  f32 right = self->position.x + data->size.x + pdata->size;

  if (left < ppos.x && ppos.x < right && top < ppos.y && ppos.y < bottom) {
    game->player->position.y = bottom;
  }

  EntityListIterator it = el_iter(game->world);
  Entity *entity;
  while ((entity = eli_next(&it))) {
    if (entity->type == ENT_BULLET) {
      BulletData *bdata = (BulletData *)entity->custom_data;
      Vector2 bpos = entity->position;

      f32 top = self->position.y - bdata->size;
      f32 bottom = self->position.y + data->size.y + bdata->size;
      f32 left = self->position.x - bdata->size;
      f32 right = self->position.x + data->size.x + bdata->size;

      if (left < bpos.x && bpos.x < right && top < bpos.y && bpos.y < bottom) {
        bullet_hit_enemy(entity, self, game);

        if (bdata->deferred_destroy) {
          el_destroy(game->world, entity);
        } else {
          entity->position.y = bottom;
          bdata->velocity.y = fabsf(bdata->velocity.y);
        }
      }
    }
  }

  // Destroy self on reaching the bottom of the screen
  if (self->position.y > FIELD_HEIGHT / 2.0f - data->size.y * 2.0f) {
    pdata->health -= data->damage;
    el_destroy(game->world, self);
  }
}

static void enemy_draw(Entity *enemy, Game game) {
  EnemyData *data = (EnemyData *)enemy->custom_data;

  Rectangle rect = {enemy->position.x, enemy->position.y, data->size.x,
                    data->size.y};
  rect = game_to_screen_rect(rect);

  Rectangle rect2 = rect;
  rect2.height *= (float)data->health / data->max_health;

  DrawRectangleRec(rect, RED);
  DrawRectangleRec(rect2, ORANGE);
  DrawRectangleLinesEx(rect, 1.0f, BLACK);
}

Entity *enemy_create(u32 x, u32 y, u32 w, u32 h) {
  Entity *enemy = ent_create(ENT_ENEMY);

  enemy->position = (Vector2){
      -FIELD_WIDTH / 2.0f + x * GRID_SIZE,
      -FIELD_HEIGHT / 2.0f + y * GRID_SIZE,
  };
  Vector2 size = {w * GRID_SIZE, h * GRID_SIZE};

  enemy->custom_data = enemy_init_data(size);

  enemy->update = enemy_update;
  enemy->draw = enemy_draw;

  return enemy;
}
