#include <raylib.h>
#include <stdlib.h>

#include "config.h"
#include "dmg_number.h"
#include "enemy.h"
#include "entity_list.h"
#include "explosion.h"
#include "game.h"
#include "raymath.h"
#include "utils.h"

static ExplosionData *explosion_init_data(f32 radius, i32 damage) {
  ExplosionData *data = malloc(sizeof(ExplosionData));
  data->radius = radius;
  data->damage = damage;
  data->ttl = EXPLOSION_TTL;

  return data;
}

static void explosion_update(Entity *self, Game game) {
  ExplosionData *data = (ExplosionData *)self->custom_data;
  f32 current_radius = data->radius * (data->ttl / EXPLOSION_TTL);

  data->ttl -= game->delta_time;

  if (data->ttl <= 0) {
    // Damage enemies and remove from game
    if (data->damage > 0) {
      EntityListIterator it = el_iter(game->world);
      Entity *entity;
      while ((entity = eli_next(&it))) {
        if (entity->type == ENT_ENEMY) {
          EnemyData *edata = (EnemyData *)entity->custom_data;
          Rectangle bounds = {entity->position.x, entity->position.y,
                              edata->size.x, edata->size.y};

          if (CheckCollisionCircleRec(self->position, data->radius, bounds)) {
            edata->health -= data->damage;

            Vector2 enemy_center =
                Vector2Add(entity->position, Vector2Scale(edata->size, 0.5f));
            Entity *dmg_number =
                dmg_number_create(enemy_center, data->damage, DMG_NUMBER_SIZE);
            el_add(game->world, dmg_number);
          }
        }
      }
    }

    el_destroy(game->world, self);
  }
}

static void explosion_draw(Entity *self, Game game) {
  ExplosionData *data = (ExplosionData *)self->custom_data;
  f32 current_radius = data->radius * (1 - data->ttl / EXPLOSION_TTL);

  // Draw explosion
  Vector2 screen_pos = game_to_screen(self->position);
  f32 screen_size = game_to_screen_scale(current_radius);

  DrawCircle(screen_pos.x, screen_pos.y, screen_size, RED);
  DrawCircle(screen_pos.x, screen_pos.y, screen_size * 0.95f, ORANGE);
  DrawCircle(screen_pos.x, screen_pos.y, screen_size * 0.8f, YELLOW);
  DrawCircle(screen_pos.x, screen_pos.y, screen_size * 0.5f, WHITE);
}

Entity *explosion_create(Vector2 position, f32 radius, i32 damage) {
  Entity *explosion = ent_create(ENT_EXPLOSION);

  explosion->position = position;
  explosion->custom_data = explosion_init_data(radius, damage);

  explosion->update = explosion_update;
  explosion->draw = explosion_draw;

  return explosion;
}
