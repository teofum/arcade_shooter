#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>

#include "assets.h"
#include "config.h"
#include "dmg_number.h"
#include "entity.h"
#include "entity_list.h"
#include "explosion.h"
#include "game.h"
#include "utils.h"

static Explosion explosion_init_data(f32 radius, i32 damage) {
  return (Explosion){
      .radius = radius,
      .damage = damage,
      .ttl = EXPLOSION_TTL,
  };
}

bool explosion_update(Entity *self, Game game) {
  Explosion *data = &self->explosion;

  if (data->ttl == EXPLOSION_TTL) {
    // Play explosion sound
    // Sound sfx = LoadSoundAlias(assets.sfx_explosion);
    // SetSoundPitch(sfx, 45.0f / data->radius);
    // PlaySound(sfx);

    // Shake camera
    game->camera_shake = data->radius * EXPLOSION_CAMERA_SHAKE;

    // Damage enemies
    if (data->damage > 0) {
      EntityListIterator it = el_iter(game->world);
      Entity *entity;
      while ((entity = eli_next(it))) {
        if (entity->type == ENT_ENEMY) {
          Enemy *edata = &entity->enemy;
          Rectangle bounds = {entity->position.x, entity->position.y,
                              edata->size.x, edata->size.y};

          if (CheckCollisionCircleRec(self->position, data->radius, bounds)) {
            i32 damage = get_damage(data->damage);
            edata->health -= damage;
            edata->dmg_flash_timer = DMG_FLASH_TIME;

            Vector2 enemy_center =
                Vector2Add(entity->position, Vector2Scale(edata->size, 0.5f));
            Entity *dmg_number =
                dmg_number_create(enemy_center, damage, DMG_NUMBER_SIZE);
            el_add(game->world, dmg_number);
          }
        }
      }
    }
  }

  data->ttl -= game->delta_time;

  return data->ttl <= 0;
}

bool explosion_update_client(Entity *self, Game game) {
  Explosion *data = &self->explosion;
  data->ttl -= game->delta_time;

  return false;
}

void explosion_draw(Entity *self, Game game) {
  Explosion *data = &self->explosion;

  Sprite *sprite = &assets.explosion;
  u32 frame = sprite->frames * (1 - data->ttl / EXPLOSION_TTL);

  // Draw explosion
  Rectangle rect = {self->position.x - data->radius,
                    self->position.y - data->radius, data->radius * 2,
                    data->radius * 2};
  DrawTexturePro(sprite->texture, get_frame_rect(sprite, frame), rect,
                 (Vector2){0, 0}, 0, WHITE);
}

Entity *explosion_create(Vector2 position, f32 radius, i32 damage) {
  Entity *explosion = ent_create(ENT_EXPLOSION);

  explosion->position = position;
  explosion->explosion = explosion_init_data(radius, damage);

  return explosion;
}
