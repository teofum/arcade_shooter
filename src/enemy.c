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
    [ENEMY_BOSS] = 10000,
};
static Color enemy_colors[][2] = {
    [ENEMY_NORMAL] = {BROWN, DARKBROWN},
    [ENEMY_SHOOTER] = {BLUE, DARKBLUE},
    [ENEMY_BOSS] = {PURPLE, DARKPURPLE},
};

static const char *level_text[] = {"I", "II", "III", "IV", "V"};
static f32 avg_xp_drop[] = {1.5f, 5.0f, 15.0f, 45.0f, 150.0f};

#define EPS 1e-6f

static Vector2 up = {0, -1};
static Vector2 down = {0, 1};
static Vector2 left = {-1, 0};
static Vector2 right = {1, 0};

/*============================================================================*
 * Enemy initialization                                                       *
 *============================================================================*/

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

/*============================================================================*
 * Enemy update helpers                                                       *
 *============================================================================*/

static void enemy_drop_xp(Entity *self, Game game, Vector2 position) {
  EnemyData *data = (EnemyData *)self->custom_data;

  f32 xp_drop_p = 1 - 1 / (avg_xp_drop[data->level - 1] * data->stat_scaling);
  u32 xp_drop = 0;
  do {
    xp_drop++;
  } while (frand() < xp_drop_p);

  for (u32 i = 0; i < XP_GEM_TIERS; i++) {
    u32 amt = xp_gem_values[i];
    while (xp_drop >= amt) {
      Entity *xp_gem = xp_gem_create(position, amt);
      el_add(game->world, xp_gem);
      xp_drop -= amt;
    }
  }
}

static void push_entity(Entity *entity, Rectangle bounds, f32 size,
                        Vector2 direction) {
  if (direction.x != 0) {
    if (direction.x < 0) {
      entity->position.x = bounds.x - size;
    } else {
      entity->position.x = bounds.x + bounds.height + size;
    }
  }
  if (direction.y != 0) {
    if (direction.y < 0) {
      entity->position.y = bounds.y - size;
    } else {
      entity->position.y = bounds.y + bounds.width + size;
    }
  }
}

static void enemy_move(Entity *self, Game game, Vector2 direction) {
  EnemyData *data = (EnemyData *)self->custom_data;
  PlayerData *pdata = (PlayerData *)game->player->custom_data;

  // Move, pushing the player and bullets if it collides
  Vector2 delta = Vector2Scale(direction, game->delta_time * ENEMY_SPEED);
  self->position = Vector2Add(self->position, delta);

  Rectangle bounds = {self->position.x + EPS, self->position.y + EPS,
                      data->size.x - 2 * EPS, data->size.y - 2 * EPS};

  if (CheckCollisionCircleRec(game->player->position, pdata->size, bounds)) {
    push_entity(game->player, bounds, pdata->size, direction);
  }

  EntityListIterator it = el_iter(game->world);
  Entity *entity;
  while ((entity = eli_next(&it))) {
    if (entity->type == ENT_BULLET) {
      BulletData *bdata = (BulletData *)entity->custom_data;

      if (CheckCollisionCircleRec(entity->position, bdata->size, bounds)) {
        bullet_hit_enemy(entity, self, game);

        if (bdata->deferred_destroy) {
          el_destroy(game->world, entity);
        } else {
          push_entity(entity, bounds, bdata->size, direction);
          if (direction.y != 0)
            bdata->velocity.y = fabsf(bdata->velocity.y) * direction.y;
          if (direction.x != 0)
            bdata->velocity.x = fabsf(bdata->velocity.x) * direction.x;
        }
      }
    }
  }
}

static void enemy_fire(Entity *self, Game game, Vector2 target) {
  EnemyData *data = (EnemyData *)self->custom_data;
  Vector2 center = Vector2Add(self->position, Vector2Scale(data->size, 0.5f));

  if (data->fire_timer <= 0) {
    Entity *bullet = enemy_bullet_create(center, target, data->ranged_damage);
    el_add(game->world, bullet);

    data->fire_timer = data->fire_cooldown;

    if (data->type == ENEMY_BOSS) {
      data->boss_bullet_counter++;
    }
  }
  data->fire_timer -= game->delta_time;
}

/*============================================================================*
 * Enemy update function                                                      *
 *============================================================================*/

static void enemy_update(Entity *self, Game game) {
  EnemyData *data = (EnemyData *)self->custom_data;
  PlayerData *pdata = (PlayerData *)game->player->custom_data;
  Vector2 center = Vector2Add(self->position, Vector2Scale(data->size, 0.5f));

  // Die
  if (data->health <= 0) {
    enemy_drop_xp(self, game, center);

    // Drop a powerup sometimes
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

  enemy_move(self, game, down);

  if (data->type == ENEMY_SHOOTER) {
    enemy_fire(self, game, game->player->position);
  }

  // Destroy self on reaching the bottom of the screen
  if (self->position.y >
      FIELD_HEIGHT / 2.0f - data->size.y - ((f32)FIELD_WIDTH / FIELD_COLS)) {
    pdata->health -= data->damage;
    el_destroy(game->world, self);
  }
}

/*============================================================================*
 * Enemy draw function                                                        *
 *============================================================================*/

static void enemy_draw(Entity *self, Game game) {
  EnemyData *data = (EnemyData *)self->custom_data;

  Rectangle rect = {self->position.x, self->position.y, data->size.x,
                    data->size.y};
  rect = game_to_screen_rect(rect);

  Rectangle rect2 = rect;
  rect2.height *= (float)data->health / data->max_health;

  DrawRectangleRec(rect, enemy_colors[data->type][1]);
  DrawRectangleRec(rect2, enemy_colors[data->type][0]);
  DrawRectangleLinesEx(rect, 1.0f, BLACK);
  DrawText(level_text[data->level - 1], rect.x + 10, rect.y + 10, 10, WHITE);
}

/*============================================================================*
 * Enemy constructor                                                          *
 *============================================================================*/

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

/*============================================================================*
 * Boss functions                                                             *
 *============================================================================*/

static EnemyData *boss_init_data() {
  EnemyData *data = malloc(sizeof(EnemyData));

  u32 w = 3, h = 4;

  Vector2 size = {w * GRID_SIZE, h * GRID_SIZE};
  data->size = size;

  data->type = ENEMY_BOSS;
  data->boss_state = BOSS_ENTER;
  data->boss_bullet_counter = 0;
  data->health = data->max_health = base_health[ENEMY_BOSS];

  data->damage = 9999;
  data->ranged_damage = 20;

  data->fire_timer = data->fire_cooldown = 0.0f;

  return data;
}

static void boss_next_state(EnemyData *data) {
  if (data->boss_state == BOSS_MOVE_LEFT_2) {
    data->boss_state = BOSS_SHOOT_ARC;
  } else {
    data->boss_state++;
  }

  // Set up new state
  switch (data->boss_state) {
  case BOSS_SHOOT_ARC:
    data->fire_timer = data->fire_cooldown = BOSS_ARC_COOLDOWN;
    data->boss_bullet_counter = 0;
    break;
  case BOSS_SHOOT_HOMING_1:
  case BOSS_SHOOT_HOMING_2:
    data->fire_timer = data->fire_cooldown = BOSS_HOMING_COOLDOWN;
    data->boss_bullet_counter = 0;
    break;
  case BOSS_SHOOT_SPIRAL:
    data->fire_timer = data->fire_cooldown = BOSS_SPIRAL_COOLDOWN;
    data->boss_bullet_counter = 0;
    break;
  default:
    break;
  }
}

static void boss_update(Entity *self, Game game) {
  EnemyData *data = (EnemyData *)self->custom_data;
  Vector2 center = Vector2Add(self->position, Vector2Scale(data->size, 0.5f));

  switch (data->boss_state) {
  case BOSS_ENTER: {
    enemy_move(self, game, down);

    if (self->position.y >= -FIELD_HEIGHT / 2.0f + GRID_SIZE)
      boss_next_state(data);

    break;
  }
  case BOSS_SHOOT_ARC: {
    u32 b = data->boss_bullet_counter;
    f32 angle = -45 + b * (180.0f / BOSS_ARC_BULLETS);
    if (b >= BOSS_ARC_BULLETS / 2) {
      angle = 90 - angle;
    }
    angle *= PI / 180.0f;

    Vector2 direction = {sinf(angle), cosf(angle)};
    enemy_fire(self, game, Vector2Add(center, direction));

    if (data->boss_bullet_counter == BOSS_ARC_BULLETS)
      boss_next_state(data);
    break;
  }
  case BOSS_MOVE_LEFT_1: {
    enemy_move(self, game, left);

    if (self->position.x <= -FIELD_WIDTH / 2.0f)
      boss_next_state(data);

    break;
  }
  case BOSS_SHOOT_HOMING_1:
  case BOSS_SHOOT_HOMING_2: {
    enemy_fire(self, game, game->player->position);

    if (data->boss_bullet_counter == BOSS_HOMING_BULLETS)
      boss_next_state(data);
    break;
  }
  case BOSS_MOVE_RIGHT_1: {
    enemy_move(self, game, right);

    if (self->position.x >= -1.5f * GRID_SIZE)
      boss_next_state(data);

    break;
  }
  case BOSS_SHOOT_SPIRAL: {
    u32 b = data->boss_bullet_counter;
    f32 angle = b * (1080.0f / BOSS_SPIRAL_BULLETS);
    angle *= PI / 180.0f;

    Vector2 direction = {sinf(angle), cosf(angle)};
    enemy_fire(self, game, Vector2Add(center, direction));

    if (data->boss_bullet_counter == BOSS_SPIRAL_BULLETS)
      boss_next_state(data);
    break;
  }
  case BOSS_MOVE_RIGHT_2: {
    enemy_move(self, game, right);

    if (self->position.x >= FIELD_WIDTH / 2.0f - 3 * GRID_SIZE)
      boss_next_state(data);

    break;
  }
  case BOSS_MOVE_LEFT_2: {
    enemy_move(self, game, left);

    if (self->position.x <= -1.5f * GRID_SIZE)
      boss_next_state(data);

    break;
  }
  default:
    break;
  }
}

static void boss_draw(Entity *self, Game game) {
  EnemyData *data = (EnemyData *)self->custom_data;

  Rectangle rect = {self->position.x, self->position.y, data->size.x,
                    data->size.y};
  rect = game_to_screen_rect(rect);

  Rectangle rect2 = rect;
  rect2.height *= (float)data->health / data->max_health;

  DrawRectangleRec(rect, enemy_colors[data->type][1]);
  DrawRectangleRec(rect2, enemy_colors[data->type][0]);
  DrawRectangleLinesEx(rect, 1.0f, BLACK);
}

Entity *boss_create() {
  Entity *boss = ent_create(ENT_ENEMY);

  boss->position = (Vector2){
      -1.5f * GRID_SIZE,
      -FIELD_HEIGHT / 2.0f - 4 * GRID_SIZE,
  };
  boss->custom_data = boss_init_data();

  boss->update = boss_update;
  boss->draw = boss_draw;

  return boss;
}
