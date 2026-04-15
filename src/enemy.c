#include <math.h>
#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>

#include "bullet.h"
#include "config.h"
#include "enemy.h"
#include "enemy_bullet.h"
#include "entity.h"
#include "entity_list.h"
#include "explosion.h"
#include "game.h"
#include "player.h"
#include "powerup.h"
#include "utils.h"
#include "xp_gem.h"

static i32 base_health[] = {
    [ENEMY_NORMAL] = 100,
    [ENEMY_SHOOTER] = 50,
};
static Color enemy_colors[][2] = {
    [ENEMY_NORMAL] = {BROWN, DARKBROWN},
    [ENEMY_SHOOTER] = {BLUE, DARKBLUE},
};

static const char *level_text[] = {"I", "II", "III", "IV", "V"};
static f32 avg_xp_drop[] = {1.5f, 5.0f, 15.0f, 45.0f, 150.0f};

static EnemyData *enemy_init_data(u32 w, u32 h, EnemyType type, u32 level) {
  EnemyData *data = malloc(sizeof(EnemyData));

  Vector2 size = {w * GRID_SIZE, h * GRID_SIZE};
  data->size = size;
  data->stat_scaling = w * h;

  data->type = type;
  data->level = level;
  data->health = data->max_health =
      base_health[type] * level * data->stat_scaling;

  data->damage = 20;
  data->ranged_damage = 5 * level;

  data->fire_timer = data->fire_cooldown = 5.5f - level * 0.5f;

  // Miniboss
  if (data->stat_scaling >= 4) {
    data->damage = 9999;
    data->fire_timer = data->fire_cooldown = 0.5f;
    data->stat_scaling *= 4; // 4x XP drop
  }

  return data;
}

static void enemy_update(Entity *self, Game game) {
  EnemyData *data = (EnemyData *)self->custom_data;
  Vector2 center = Vector2Add(self->position, Vector2Scale(data->size, 0.5f));

  // Die
  if (data->health <= 0) {

    f32 xp_drop_p = 1 - 1 / (avg_xp_drop[data->level - 1] * data->stat_scaling);
    u32 xp_drop = 0;
    do {
      xp_drop++;
    } while (frand() < xp_drop_p);
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

    if (frand() < POWERUP_SPAWN_PROB) {
      Entity *powerup = powerup_create(center, rand() % 2);
      el_add(game->world, powerup);
    }

    // kaboom
    Entity *explosion = explosion_create(center, data->size.x * 0.6f, 0);
    el_add(game->world, explosion);

    game->score += 10 * data->level * data->stat_scaling;

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

  // If it's a shooty enemy, shoot
  if (data->type == ENEMY_SHOOTER) {
    if (data->fire_timer <= 0) {
      Entity *bullet = enemy_bullet_create(center, game->player->position,
                                           data->ranged_damage);
      el_add(game->world, bullet);

      data->fire_timer = data->fire_cooldown;
    } else {
      data->fire_timer -= game->delta_time;
    }
  }

  // Destroy self on reaching the bottom of the screen
  if (self->position.y >
      FIELD_HEIGHT / 2.0f - data->size.y - ((f32)FIELD_WIDTH / FIELD_COLS)) {
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

  DrawRectangleRec(rect, enemy_colors[data->type][1]);
  DrawRectangleRec(rect2, enemy_colors[data->type][0]);
  DrawRectangleLinesEx(rect, 1.0f, BLACK);
  DrawText(level_text[data->level - 1], rect.x + 10, rect.y + 10, 10, WHITE);
}

Entity *enemy_create(u32 x, u32 y, u32 w, u32 h, EnemyType type, u32 level) {
  Entity *enemy = ent_create(ENT_ENEMY);

  enemy->position = (Vector2){
      -FIELD_WIDTH / 2.0f + x * GRID_SIZE,
      -FIELD_HEIGHT / 2.0f + y * GRID_SIZE,
  };
  enemy->custom_data = enemy_init_data(w, h, type, level);

  enemy->update = enemy_update;
  enemy->draw = enemy_draw;

  return enemy;
}
