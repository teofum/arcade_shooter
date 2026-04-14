#include <math.h>
#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>

#include "bullet.h"
#include "config.h"
#include "entity.h"
#include "entity_list.h"
#include "game.h"
#include "physics.h"
#include "player.h"
#include "powerup.h"
#include "types.h"
#include "utils.h"

// Imaginary bottom wall to prevent player going OOB
static Rectangle bottom_wall = {-FIELD_WIDTH / 2.0f, 100, FIELD_WIDTH, 100};

/*============================================================================*
 * Player initialization                                                      *
 *============================================================================*/

static PlayerData *player_init_data() {
  PlayerData *data = malloc(sizeof(PlayerData));

  data->size = 5.0f;

  data->velocity = (Vector2){0, 0};
  data->direction = (Vector2){0, 0};
  data->crosshair = (Vector2){0, 0};

  data->health = data->max_health = 100;
  data->base_damage = 5;
  data->move_speed = PLAYER_SPEED;

  data->level = 1;
  data->xp = 0;
  data->to_next_level = 5;

  data->ammo = data->max_ammo = 5;
  data->special_bullet_count = 0;

  data->active_powerup = POWER_NONE;
  data->powerup_timer = 0.0f;

  data->fire_cooldown = 0.1f;
  data->fire_timer = 0.0f;
  data->firing = false;

  return data;
}

/*============================================================================*
 * Player update helpers                                                      *
 *============================================================================*/

static void player_move(Entity *self, Game game) {
  PlayerData *data = (PlayerData *)self->custom_data;

  // Update velocity
  f32 speed = data->move_speed * (data->active_powerup == POWER_FAST ? 2 : 1);
  Vector2 target_velocity = Vector2Scale(data->direction, speed);
  data->velocity = Vector2Lerp(data->velocity, target_velocity, PLAYER_ACCEL);

  // Predict next position
  Vector2 delta_pos = Vector2Scale(data->velocity, game->delta_time);
  Vector2 next_pos = Vector2Add(self->position, delta_pos);

  // Check for collisions
  // Also collide with a "fake" wall at the bottom so player can't go OOB
  Collision collision =
      check_collisions(self, game, next_pos, data->size, NULL);

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
}

static Entity *player_fire_special(Entity *self, Game game, i32 damage) {
  PlayerData *data = (PlayerData *)self->custom_data;

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
  PlayerData *data = (PlayerData *)self->custom_data;

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
  PlayerData *data = (PlayerData *)self->custom_data;

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
  PlayerData *data = (PlayerData *)self->custom_data;

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
    if ((f32)rand() / RAND_MAX < 0.5f) {
      data->max_ammo++;
      data->ammo++;
      data->leveled_up_stats[STAT_AMMO] = true;
      leveled_up_stats++;
    }
    if ((f32)rand() / RAND_MAX < 0.5f) {
      data->base_damage++;
      data->leveled_up_stats[STAT_DAMAGE] = true;
      leveled_up_stats++;
    }
    if ((f32)rand() / RAND_MAX < 0.5f) {
      data->max_health += 10;
      data->health += 10;
      data->leveled_up_stats[STAT_HEALTH] = true;
      leveled_up_stats++;
    }
    if ((f32)rand() / RAND_MAX < 0.5f) {
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
  game->state = GS_LEVEL_UP;
}

/*============================================================================*
 * Player update function                                                     *
 *============================================================================*/

static void player_update(Entity *self, Game game) {
  PlayerData *data = (PlayerData *)self->custom_data;

  // Die
  if (data->health <= 0) {
    game->state = GS_GAME_OVER;
    return;
  }

  player_move(self, game);
  player_fire(self, game);

  if (data->xp >= data->to_next_level) {
    player_level_up(self, game);
  }

  player_update_timers(self, game);
}

/*============================================================================*
 * Player draw function                                                       *
 *============================================================================*/

static void player_draw(Entity *player, Game game) {
  PlayerData *data = (PlayerData *)player->custom_data;

  Vector2 screen_pos = game_to_screen(player->position);
  Vector2 screen_crosshair = game_to_screen(data->crosshair);
  f32 screen_size = game_to_screen_scale(data->size);

  // Draw player
  DrawCircle(screen_pos.x, screen_pos.y, screen_size, RED);

  // Draw targeting crosshairs
  DrawRectangle(screen_crosshair.x - 5, screen_crosshair.y - 1, 11, 3, BLACK);
  DrawRectangle(screen_crosshair.x - 1, screen_crosshair.y - 5, 3, 11, BLACK);

  // Draw debug aim line
  Vector2 aim = Vector2Subtract(screen_crosshair, screen_pos);
  aim = Vector2Scale(Vector2Normalize(aim), 40);
  aim = Vector2Add(aim, screen_pos);

  DrawLine(screen_pos.x, screen_pos.y, aim.x, aim.y, WHITE);
}

/*============================================================================*
 * Player constructor                                                         *
 *============================================================================*/

Entity *player_create() {
  Entity *player = ent_create(ENT_PLAYER);

  player->position = (Vector2){0, 0};
  player->custom_data = player_init_data();
  player->update = player_update;
  player->draw = player_draw;

  return player;
}
