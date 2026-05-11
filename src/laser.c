#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>

#include "assets.h"
#include "config.h"
#include "dmg_number.h"
#include "entity.h"
#include "entity_list.h"
#include "game.h"
#include "laser.h"

static LaserData *laser_init_data(Entity *laser, i32 damage) {
  LaserData *data = &laser->laser;
  data->damage = damage;
  data->ttl = LASER_TTL;

  return data;
}

static void laser_update(Entity *self, Game game) {
  LaserData *data = &self->laser;

  if (data->ttl == LASER_TTL && data->damage > 0) {
    EntityListIterator it = el_iter(game->world);
    Entity *entity;
    while ((entity = eli_next(&it))) {
      if (entity->type == ENT_ENEMY) {
        EnemyData *edata = &entity->enemy;
        Rectangle bounds = {entity->position.x, entity->position.y,
                            edata->size.x, edata->size.y};

        if (bounds.y < self->position.y &&
            self->position.y < bounds.y + bounds.height) {
          edata->health -= data->damage;
          edata->dmg_flash_timer = DMG_FLASH_TIME;

          Vector2 enemy_center =
              Vector2Add(entity->position, Vector2Scale(edata->size, 0.5f));
          Entity *dmg_number =
              dmg_number_create(enemy_center, data->damage, DMG_NUMBER_SIZE);
          el_add(game->world, dmg_number);
        }
      }
    }
  }

  data->ttl -= game->delta_time;

  if (data->ttl <= 0) {
    el_destroy(game->world, self);
  }
}

static void laser_draw(Entity *self, Game game) {
  LaserData *data = &self->laser;

  Sprite *sprite = &assets.fire;
  u32 frame = sprite->frames * (1 - data->ttl / LASER_TTL);

  // Draw explosion
  f32 w = (f32)FIELD_WIDTH / FIELD_COLS;
  u8 alpha = 255 * (data->ttl / LASER_TTL);
  Rectangle rect = {-FIELD_WIDTH / 2.0f, self->position.y - 4, w, 8};

  for (u32 i = 0; i < FIELD_COLS; i++) {
    DrawTexturePro(sprite->texture, get_frame_rect(sprite, frame), rect,
                   (Vector2){0, 0}, 0, (Color){255, 255, 255, alpha});
    rect.x += w;
  }
}

Entity *laser_create(Vector2 position, i32 damage) {
  Entity *laser = ent_create(ENT_LASER);

  laser->position = position;
  laser_init_data(laser, damage);

  laser->update = laser_update;
  laser->draw = laser_draw;

  return laser;
}
