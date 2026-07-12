#include <assert.h>
#include <float.h>
#include <math.h>
#include <raylib.h>
#include <stdio.h>

#include "ai.h"
#include "config.h"
#include "entity.h"
#include "entity_list.h"
#include "game.h"
#include "raymath.h"

const char *ai_state_name[4] = {
    [AI_IDLE] = "Idle",
    [AI_MOVING] = "Moving",
    [AI_SHOOTING] = "Shooting",
    [AI_DEFENDING] = "Defending",
};

void ai_init(AiPlayer *ai) { ai->state = AI_IDLE; }

static void ai_change_state(AiPlayer *ai, AiPlayerState state) {
  ai->state = state;
}

static f32 get_target_x_pos(Game game, u32 player_idx) {
  // Crappy solution to maximization problem
  f32 best_x = 0.0f;
  f32 max_min_dist = 0.0f;
  f32 w = FIELD_WIDTH / 2.0f;

  for (f32 x = -w; x <= w; x += w / 32.0f) {
    f32 min_dist = fminf(fabsf(-w - x), fabsf(w - x));
    for (u32 i = 0; i < MAX_CLIENTS; i++) {
      if (i != player_idx && game->players[i] != NULL) {
        f32 player_x = game->players[i]->position.x;
        min_dist = fminf(min_dist, fabsf(x - player_x));
      }
    }

    if (min_dist > max_min_dist) {
      max_min_dist = min_dist;
      best_x = x;
    }
  }

  return best_x;
}

static Entity *get_lowest_enemy(Game game) {
  f32 max_y = -FLT_MAX;
  Entity *lowest = NULL;

  for (u32 i = 0; i < el_size(game->world); i++) {
    Entity *e = el_get(game->world, i);
    if (e->type == ENT_ENEMY && e->position.y > max_y) {
      lowest = e;
      max_y = e->position.y;
    }
  }

  return lowest;
}

static Entity *get_closest_enemy(Game game, Vector2 pos) {
  f32 min_d = FLT_MAX;
  Entity *closest = NULL;

  for (u32 i = 0; i < el_size(game->world); i++) {
    Entity *e = el_get(game->world, i);
    f32 d = Vector2Distance(e->position, pos);
    if (e->type == ENT_ENEMY && d < min_d) {
      closest = e;
      min_d = d;
    }
  }

  return closest;
}

void ai_update(Game game, u32 player_idx) {
  if (game->player_type[player_idx] != PLAYER_AI) {
    printf("Fatal: AI update on non-AI player %u\n", player_idx);
    assert(false);
  }

  Entity *self = game->players[player_idx];
  AiPlayer *ai = &game->ai_players[player_idx];
  InputData *input = &game->server.input[player_idx];

  switch (ai->state) {
  case AI_IDLE: {
    Entity *lowest_enemy = get_lowest_enemy(game);

    if (lowest_enemy != NULL) {
      ai_change_state(ai, AI_MOVING);
    }

    break;
  }
  case AI_MOVING: {
    Entity *lowest_enemy = get_lowest_enemy(game);

    if (lowest_enemy == NULL) {
      ai_change_state(ai, AI_IDLE);
    } else {
      f32 target_y = lowest_enemy->position.y + AI_TARGET_Y_D;
      f32 target_x = get_target_x_pos(game, player_idx);
      ai->target_pos = (Vector2){target_x, target_y};

      f32 d = Vector2Distance(self->position, ai->target_pos);
      if (d < AI_MOVE_STOP_D) {
        ai_change_state(ai, AI_SHOOTING);
      } else {
        Vector2 direction = Vector2Subtract(ai->target_pos, self->position);
        input->direction = Vector2Normalize(direction);
        ai->firing = false;
      }
    }

    break;
  }
  case AI_SHOOTING: {
    Entity *lowest_enemy = get_lowest_enemy(game);

    if (lowest_enemy == NULL) {
      ai_change_state(ai, AI_IDLE);
    } else {
      f32 target_y = lowest_enemy->position.y + AI_TARGET_Y_D;
      f32 target_x = get_target_x_pos(game, player_idx);
      ai->target_pos = (Vector2){target_x, target_y};

      f32 d = Vector2Distance(self->position, ai->target_pos);
      if (d > AI_MOVE_START_D) {
        ai_change_state(ai, AI_MOVING);
      } else {
        input->direction = (Vector2){0, 0};
        ai->crosshair = get_closest_enemy(game, self->position)->position;
        ai->firing = true;
      }
    }

    break;
  }
  case AI_DEFENDING: {
    // TODO
    ai_change_state(ai, AI_MOVING);
    break;
  }
  }

  input->crosshair = ai->crosshair;
  input->firing = ai->firing;
}
