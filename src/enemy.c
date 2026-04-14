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
#include "explosion.h"
#include "game.h"
#include "player.h"
#include "powerup.h"
#include "utils.h"
#include "xp_gem.h"

static EnemyData *enemy_init_data(Vector2 size, u32 level) {
  EnemyData *data = malloc(sizeof(EnemyData));
  data->size = size;

  data->level = level;
  data->health = data->max_health = 100 * level;
  data->damage = 20;

  return data;
}

static const char *level_text[] = {"I", "II", "III", "IV", "V"};
static f32 avg_xp_drop[] = {1.5f, 5.0f, 15.0f, 45.0f, 150.0f};

static void enemy_update(Entity *self, Game game) {
  EnemyData *data = (EnemyData *)self->custom_data;

  // Die
  if (data->health <= 0) {
    Vector2 center = Vector2Add(self->position, Vector2Scale(data->size, 0.5f));

    f32 xp_drop_p = 1 - 1 / avg_xp_drop[data->level - 1];
    u32 xp_drop = 0;
    do {
      xp_drop++;
    } while ((f32)rand() / INT_MAX < xp_drop_p);
    while (xp_drop >= 25) {
      Entity *xp_gem = xp_gem_create(center, 25);
      el_add(game->world, xp_gem);
      xp_drop -= 25;
    }
    while (xp_drop >= 5) {
      Entity *xp_gem = xp_gem_create(center, 5);
      el_add(game->world, xp_gem);
      xp_drop -= 5;
    }
    while (xp_drop > 0) {
      Entity *xp_gem = xp_gem_create(center, 1);
      el_add(game->world, xp_gem);
      xp_drop--;
    }

    if ((f32)rand() / INT_MAX < POWERUP_SPAWN_PROB) {
      Entity *powerup = powerup_create(center, rand() % 2);
      el_add(game->world, powerup);
    }

    // kaboom
    Entity *explosion = explosion_create(center, data->size.x * 0.6f, 0);
    el_add(game->world, explosion);

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

  DrawRectangleRec(rect, DARKBROWN);
  DrawRectangleRec(rect2, BROWN);
  DrawRectangleLinesEx(rect, 1.0f, BLACK);
  DrawText(level_text[data->level - 1], rect.x + 10, rect.y + 10, 10, WHITE);
}

Entity *enemy_create(u32 x, u32 y, u32 w, u32 h, u32 level) {
  Entity *enemy = ent_create(ENT_ENEMY);

  enemy->position = (Vector2){
      -FIELD_WIDTH / 2.0f + x * GRID_SIZE,
      -FIELD_HEIGHT / 2.0f + y * GRID_SIZE,
  };
  Vector2 size = {w * GRID_SIZE, h * GRID_SIZE};

  enemy->custom_data = enemy_init_data(size, level);

  enemy->update = enemy_update;
  enemy->draw = enemy_draw;

  return enemy;
}
