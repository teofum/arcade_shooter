#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>

#include "config.h"
#include "dmg_number.h"
#include "enemy.h"
#include "entity_list.h"
#include "game.h"
#include "laser.h"
#include "utils.h"

static LaserData *laser_init_data(i32 damage) {
  LaserData *data = malloc(sizeof(LaserData));
  data->damage = damage;
  data->ttl = LASER_TTL;

  return data;
}

static void laser_update(Entity *self, Game game) {
  LaserData *data = (LaserData *)self->custom_data;

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

          if (bounds.y < self->position.y &&
              self->position.y < bounds.y + bounds.height) {
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

static void laser_draw(Entity *self, Game game) {
  LaserData *data = (LaserData *)self->custom_data;

  // Draw laser
  Vector2 screen_pos = game_to_screen(self->position);
  DrawRectangle(game_to_screen_x(-FIELD_WIDTH / 2.0f), screen_pos.y - 3,
                game_to_screen_scale(FIELD_WIDTH), 6, RED);
}

Entity *laser_create(Vector2 position, i32 damage) {
  Entity *laser = ent_create(ENT_LASER);

  laser->position = position;
  laser->custom_data = laser_init_data(damage);

  laser->update = laser_update;
  laser->draw = laser_draw;

  return laser;
}
