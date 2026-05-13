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
  return entity;
}

bool ent_update(Entity *entity, struct Game *game) {
  EntityUpdateFunction f = ent_update_dispatch[entity->type];
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
