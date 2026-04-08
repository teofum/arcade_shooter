#include <math.h>
#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>

#include "bullet.h"
#include "config.h"
#include "enemy.h"
#include "entity.h"
#include "entity_list.h"
#include "physics.h"
#include "player.h"
#include "types.h"
#include "utils.h"
#include "wall.h"

static Rectangle bottom_wall = {-FIELD_WIDTH / 2.0f, 100, FIELD_WIDTH, 100};

static PlayerData *player_init_data() {
  PlayerData *data = malloc(sizeof(PlayerData));

  data->size = 5.0f;

  data->velocity = (Vector2){0, 0};
  data->direction = (Vector2){0, 0};
  data->crosshair = (Vector2){0, 0};

  data->health = data->max_health = 100;

  data->ammo = data->max_ammo = 10;
  data->fire_cooldown = 0.1f;
  data->fire_timer = 0.0f;
  data->firing = false;

  return data;
}

Entity *player_create() {
  Entity *player = ent_create(ENT_PLAYER);

  player->position = (Vector2){0, 0};
  player->custom_data = player_init_data();
  player->update = player_update;
  player->draw = player_draw;

  return player;
}

void player_update(Entity *player, Game game) {
  PlayerData *data = (PlayerData *)player->custom_data;

  // Die
  if (data->health <= 0) {
    game->game_over = true;
    return;
  }

  // Update velocity
  Vector2 target_velocity = Vector2Scale(data->direction, PLAYER_SPEED);
  data->velocity = Vector2Lerp(data->velocity, target_velocity, PLAYER_ACCEL);

  Vector2 delta_pos = Vector2Scale(data->velocity, game->delta_time);
  Vector2 next_pos = Vector2Add(player->position, delta_pos);

  // Check for collisions with walls
  Collision collision = {
      .direction = COL_NONE,
      .t = INFINITY,
  };
  EntityListIterator it = el_iter(game->world);
  Entity *e;
  while ((e = eli_next(&it))) {
    if (e->type == ENT_WALL || e->type == ENT_ENEMY) {
      Rectangle rect;
      if (e->type == ENT_WALL) {
        WallData *wdata = (WallData *)e->custom_data;
        rect = wdata->bounds;
      } else {
        EnemyData *edata = (EnemyData *)e->custom_data;
        rect = (Rectangle){e->position.x, e->position.y, edata->size.x,
                           edata->size.y};
      }

      Collision c =
          collide_particle_rect(player->position, next_pos, data->size, rect);
      collision.direction |= c.direction;
      collision.t = fminf(collision.t, c.t);
    }
  }

  // Also collide with an "imaginary" wall at the bottom so player can't leave
  // the screen
  Collision c = collide_particle_rect(player->position, next_pos, data->size,
                                      bottom_wall);
  collision.direction |= c.direction;
  collision.t = fminf(collision.t, c.t);

  // If there was a collision change the trajectory
  if (collision.direction != COL_NONE) {
    // 1. Move in the original direction up to the point of collision
    delta_pos = Vector2Scale(delta_pos, collision.t);
    next_pos = Vector2Add(player->position, delta_pos);

    // 2. Modify velocity (fully plastic collision, ie wallslide)
    if (collision.direction & COL_X)
      data->velocity.x = 0;
    if (collision.direction & COL_Y)
      data->velocity.y = 0;

    // 3. Move the rest of the way
    delta_pos =
        Vector2Scale(data->velocity, game->delta_time * (1 - collision.t));
    next_pos = Vector2Add(next_pos, delta_pos);
  }

  // Update position
  player->position = next_pos;

  // Spawn a bullet
  if (data->firing) {
    if (data->fire_timer == 0.0f && data->ammo > 0) {
      Entity *bullet = bullet_create(player->position, data->crosshair);
      el_add(game->world, bullet);

      data->fire_timer = data->fire_cooldown;
      data->ammo--;
    }
  }

  // Update firing timer
  data->fire_timer = fmaxf(data->fire_timer - game->delta_time, 0.0f);
}

void player_draw(Entity *player, Game game) {
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

  DrawLine(screen_pos.x, screen_pos.y, aim.x, aim.y, BLUE);
}
