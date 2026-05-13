#include <math.h>
#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>

#include "assets.h"
#include "bullet.h"
#include "config.h"
#include "dmg_number.h"
#include "entity.h"
#include "entity_list.h"
#include "game.h"
#include "physics.h"
#include "player.h"
#include "types.h"
#include "utils.h"

// Imaginary bottom wall to prevent player going OOB
static Rectangle bottom_wall = {-FIELD_WIDTH / 2.0f, 100, FIELD_WIDTH, 100};

/*============================================================================*
 * Player initialization                                                      *
 *============================================================================*/

static Player player_init_data() {
  return (Player){
      .size = 5.0f,

      .velocity = (Vector2){0, 0},
      .direction = (Vector2){0, 0},
      .crosshair = (Vector2){0, 0},

      .health = 100,
      .max_health = 100,
      .base_damage = 5,
      .move_speed = PLAYER_SPEED,

      .level = 1,
      .xp = 0,
      .to_next_level = 5,

      .ammo = 5,
      .max_ammo = 5,
      .special_bullet_count = 0,

      .active_powerup = POWER_NONE,
      .powerup_timer = 0.0f,

      .fire_cooldown = 0.1f,
      .fire_timer = 0.0f,
      .firing = false,

      .orientation = 1,
  };
}

/*============================================================================*
 * Player update helpers                                                      *
 *============================================================================*/

/*
 * Hit callback for enemy projectiles
 */
static bool player_hit_bullet(Entity *self, Entity *enemy_bullet, Game game) {
  Player *data = &self->player;
  EnemyBullet *ebdata = &enemy_bullet->enemy_bullet;

  i32 damage = get_damage(ebdata->damage);
  data->health -= damage;

  Entity *dmg_number =
      dmg_number_create(enemy_bullet->position, damage, DMG_NUMBER_SIZE);
  el_add(game->world, dmg_number);

  ebdata->deferred_destroy = true;
  return false;
}

static void player_move(Entity *self, Game game) {
  Player *data = &self->player;

  // Update velocity
  f32 speed = data->move_speed * (data->active_powerup == POWER_FAST ? 2 : 1);
  Vector2 target_velocity = Vector2Scale(data->direction, speed);
  data->velocity = Vector2Lerp(data->velocity, target_velocity, PLAYER_ACCEL);

  // Predict next position
  Vector2 delta_pos = Vector2Scale(data->velocity, game->delta_time);
  Vector2 next_pos = Vector2Add(self->position, delta_pos);

  // Check for collisions
  // Also collide with a "fake" wall at the bottom so player can't go OOB
  Collision collision = check_collisions(self, game, next_pos, data->size, NULL,
                                         player_hit_bullet);

  Collision c =
      collide_particle_rect(self->position, next_pos, data->size, bottom_wall);
  collision.direction |= c.direction;
  collision.t = fminf(collision.t, c.t);

  // If there was a collision, change the trajectory
  if (collision.direction != COL_NONE) {
    next_pos = apply_collision(self->position, delta_pos, &data->velocity, 0,
                               collision, game);
  }

  // Update position
  self->position = next_pos;

  if (data->velocity.x > 0) {
    data->orientation = 1;
  } else if (data->velocity.x < 0) {
    data->orientation = -1;
  }
}

static Entity *player_fire_special(Entity *self, Game game, i32 damage) {
  Player *data = &self->player;

  for (u32 i = 0; i < data->special_bullet_count; i++) {
    SpecialBulletSlot *sb = &data->special_bullets[i];
    if (!sb->fired) {
      if (sb->cooldown <= 0) {
        sb->fired = true;
        return bullet_create(self->position, data->crosshair, sb->type,
                             sb->level, damage, i);
      }
    }
  }

  return NULL;
}

static void player_fire(Entity *self, Game game) {
  Player *data = &self->player;

  if (!data->firing)
    return;

  if (data->fire_timer == 0.0f) {
    i32 damage =
        data->base_damage * (data->active_powerup == POWER_DMG ? 2 : 1);

    // Fire special bullets first if available
    Entity *bullet = player_fire_special(self, game, damage);

    // Otherwise fire a regular bullet if we have ammo
    if (!bullet && data->ammo > 0) {
      bullet = bullet_create(self->position, data->crosshair, BULLET_NORMAL, 1,
                             damage, 0);
      data->ammo--;
    }

    if (bullet) {
      el_add(game->world, bullet);
      data->fire_timer = data->fire_cooldown;
    }
  }
}

static void player_update_timers(Entity *self, Game game) {
  Player *data = &self->player;

  // Update firing timer
  data->fire_timer = fmaxf(data->fire_timer - game->delta_time, 0.0f);

  // Update special bullet cooldowns
  for (u32 i = 0; i < data->special_bullet_count; i++) {
    SpecialBulletSlot *sb = &data->special_bullets[i];
    if (!sb->fired && sb->cooldown > 0) {
      sb->cooldown -= game->delta_time;
    }
  }

  // Update powerup state
  if (data->active_powerup != POWER_NONE) {
    if (data->powerup_timer <= 0.0f) {
      data->active_powerup = POWER_NONE;
    } else {
      data->powerup_timer -= game->delta_time;
    }
  }
}

static void player_level_up(Entity *self, Game game) {
  Player *data = &self->player;

  // Level up
  data->level++;
  data->xp -= data->to_next_level;
  data->to_next_level *= 2;

  // Increase stats
  u32 leveled_up_stats = 0;
  for (u32 i = 0; i < 4; i++) {
    data->leveled_up_stats[i] = false;
  }

  while (leveled_up_stats == 0) {
    if (frand() < 0.5f) {
      data->max_ammo++;
      data->ammo++;
      data->leveled_up_stats[STAT_AMMO] = true;
      leveled_up_stats++;
    }
    if (frand() < 0.5f) {
      data->base_damage++;
      data->leveled_up_stats[STAT_DAMAGE] = true;
      leveled_up_stats++;
    }
    if (frand() < 0.5f) {
      data->max_health += 10;
      data->health += 10;
      data->leveled_up_stats[STAT_HEALTH] = true;
      leveled_up_stats++;
    }
    if (frand() < 0.5f) {
      data->move_speed += 5;
      data->leveled_up_stats[STAT_MOVEMENT] = true;
      leveled_up_stats++;
    }
  }

  // Prepare level up
  // TODO clean this up
  // Populate list with valid options
  static LevelUpOption all_level_up_options[MAX_SPECIAL_BULLETS + 5];
  u32 option_count = 0;

  // Upgrade options for existing bullets
  for (u32 i = 0; i < data->special_bullet_count; i++) {
    all_level_up_options[option_count].type = LU_UPGRADE;
    all_level_up_options[option_count].bullet_idx = i;
    option_count++;
  }

  // New bullet options
  if (data->special_bullet_count < MAX_SPECIAL_BULLETS) {
    for (u32 i = 0; i < 5; i++) {
      all_level_up_options[option_count].type = LU_NEW;
      all_level_up_options[option_count].bullet_type = i + 1;
      option_count++;
    }
  }

  // If there are more than three options, shuffle them
  if (option_count > LEVEL_UP_OPTIONS) {
    for (u32 i = 0; i < option_count - 1; i++) {
      u32 j = i + rand() / (RAND_MAX / (option_count - i) + 1);
      LevelUpOption t = all_level_up_options[j];
      all_level_up_options[j] = all_level_up_options[i];
      all_level_up_options[i] = t;
    }
  }

  // Pick the first three
  for (u32 i = 0; i < LEVEL_UP_OPTIONS; i++) {
    data->level_up_options[i] =
        i < option_count ? &all_level_up_options[i] : NULL;
  }

  // Show level up screen
  game_set_state(game, GS_LEVEL_UP);
}

/*============================================================================*
 * Player update function                                                     *
 *============================================================================*/

bool player_update(Entity *self, Game game) {
  Player *data = &self->player;

  // Die
  if (data->health <= 0) {
    PlaySound(assets.sfx_oof);
    game_set_state(game, GS_GAME_OVER);
    return false;
  }

  player_move(self, game);
  player_fire(self, game);

  if (data->xp >= data->to_next_level) {
    player_level_up(self, game);
  }

  player_update_timers(self, game);

  return false;
}

/*============================================================================*
 * Player draw function                                                       *
 *============================================================================*/

void player_draw(Entity *self, Game game) {
  Player *data = &self->player;

  Sprite *sprite = &assets.player;

  Rectangle source = get_frame_rect(sprite, 0);
  source.width *= data->orientation;

  Vector2 p = self->position;
  f32 size = data->size * 3;
  Rectangle dest = (Rectangle){p.x, p.y, size, size};

  Vector2 origin = {size / 2, size / 2};

  // Draw player
  DrawTexturePro(sprite->texture, source, dest, origin, data->velocity.x * 0.2f,
                 WHITE);
}

/*============================================================================*
 * Player constructor                                                         *
 *============================================================================*/

Entity *player_create() {
  Entity *player = ent_create(ENT_PLAYER);

  player->position = (Vector2){0, 0};
  player->player = player_init_data();

  return player;
}
