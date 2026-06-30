#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>

#include "bullet.h"
#include "dmg_number.h"
#include "enemy.h"
#include "enemy_bullet.h"
#include "entity.h"
#include "explosion.h"
#include "laser.h"
#include "player.h"
#include "powerup.h"
#include "wall.h"
#include "xp_gem.h"

const char *entity_type_name[] = {
    [ENT_PLAYER] = "player",
    [ENT_BULLET] = "bullet",
    [ENT_WALL] = "wall",
    [ENT_ENEMY] = "enemy",
    [ENT_DMG_NUMBER] = "dmg_number",
    [ENT_XP_GEM] = "xp_gem",
    [ENT_POWERUP] = "powerup",
    [ENT_EXPLOSION] = "explosion",
    [ENT_LASER] = "laser",
    [ENT_ENEMY_BULLET] = "enemy_bullet",
};

static EntityUpdateFunction ent_update_dispatch[] = {
    [ENT_PLAYER] = player_update,
    [ENT_BULLET] = bullet_update,
    [ENT_WALL] = NULL,
    [ENT_ENEMY] = enemy_update,
    [ENT_DMG_NUMBER] = dmg_number_update,
    [ENT_XP_GEM] = xp_gem_update,
    [ENT_POWERUP] = powerup_update,
    [ENT_EXPLOSION] = explosion_update,
    [ENT_LASER] = laser_update,
    [ENT_ENEMY_BULLET] = enemy_bullet_update,
};

static EntityUpdateFunction ent_client_update_dispatch[] = {
    [ENT_PLAYER] = player_update_client,
    [ENT_BULLET] = bullet_update_client,
    [ENT_WALL] = NULL,
    [ENT_ENEMY] = enemy_update_client,
    [ENT_DMG_NUMBER] = NULL,
    [ENT_XP_GEM] = NULL,
    [ENT_POWERUP] = NULL,
    [ENT_EXPLOSION] = explosion_update_client,
    [ENT_LASER] = laser_update_client,
    [ENT_ENEMY_BULLET] = enemy_bullet_update_client,
};

static EntityDrawFunction ent_draw_dispatch[] = {
    [ENT_PLAYER] = player_draw,
    [ENT_BULLET] = bullet_draw,
    [ENT_WALL] = wall_draw,
    [ENT_ENEMY] = enemy_draw,
    [ENT_DMG_NUMBER] = dmg_number_draw,
    [ENT_XP_GEM] = xp_gem_draw,
    [ENT_POWERUP] = powerup_draw,
    [ENT_EXPLOSION] = explosion_draw,
    [ENT_LASER] = laser_draw,
    [ENT_ENEMY_BULLET] = enemy_bullet_draw,
};

Entity *ent_create(EntityType type) {
  Entity *entity = calloc(1, sizeof(Entity));
  entity->type = type;
  entity->position = (Vector2){0, 0};
  entity->velocity = (Vector2){0, 0};
  return entity;
}

bool ent_update(Entity *entity, struct Game *game) {
  EntityUpdateFunction f = ent_update_dispatch[entity->type];
  if (f == NULL)
    return false;

  return f(entity, game);
}

bool ent_update_client(Entity *entity, struct Game *game) {
  EntityUpdateFunction f = ent_client_update_dispatch[entity->type];
  if (f == NULL)
    return false;

  return f(entity, game);
}

void ent_draw(Entity *entity, struct Game *game) {
  EntityDrawFunction f = ent_draw_dispatch[entity->type];
  if (f == NULL)
    return;

  f(entity, game);
}

EntityCreateData ent_get_create_data(Entity *entity) {
  EntityCreateData cd = {.position = entity->position};

  switch (entity->type) {
  case ENT_BULLET:
    cd.bullet = (BulletCreateData){
        .type = entity->bullet.type,
        .target = Vector2Add(entity->position, entity->velocity),
        .damage = entity->bullet.damage,
        .level = entity->bullet.level,
        .special_idx = entity->bullet.special_idx,
    };
    break;
  case ENT_ENEMY:
    cd.enemy = (EnemyCreateData){
        .w = 1,
        .h = 1,
        .level = entity->enemy.level,
        .type = entity->enemy.type,
    };
    break;
  case ENT_ENEMY_BULLET:
    cd.bullet = (BulletCreateData){
        .target = Vector2Add(entity->position, entity->velocity),
        .damage = entity->enemy_bullet.damage,
    };
    break;
  case ENT_EXPLOSION:
    cd.explosion = (ExplosionCreateData){
        .damage = entity->explosion.damage,
        .radius = entity->explosion.radius,
    };
    break;
  case ENT_LASER:
    cd.laser_damage = entity->laser.damage;
    break;
  case ENT_XP_GEM:
    cd.xp_value = entity->xp_gem.value;
    break;
  case ENT_POWERUP:
    cd.powerup_type = entity->powerup.type;
    break;
  case ENT_DMG_NUMBER:
    cd.dmg_number = (DmgNumberCreateData){
        .damage = entity->dmg_number.damage,
        .size = entity->dmg_number.size,
    };
    break;
  default:
    break;
  }

  return cd;
}
