#ifndef BULLET_H
#define BULLET_H

#include <raylib.h>

#include "entity.h"
#include "game.h"
#include "types.h"

extern const char *bullet_type_names[6];
extern Color bullet_type_colors[6];

Entity *bullet_create(Vector2 position, Vector2 target, BulletType type,
                      u32 level, i32 damage, u32 special_idx, Entity *player);
bool bullet_update(Entity *self, Game game);
bool bullet_update_client(Entity *self, Game game);
void bullet_draw(Entity *self, Game game);

bool bullet_hit_enemy(Entity *self, Entity *enemy, Game game);

const char *get_bullet_description(Entity *player, BulletType type, u32 level);

#endif
