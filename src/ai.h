#ifndef AI_H
#define AI_H

#include <raylib.h>

#include "types.h"

typedef enum {
  AI_IDLE,
  AI_MOVING,
  AI_SHOOTING,
  AI_DEFENDING,
} AiPlayerState;

extern const char *ai_state_name[4];

typedef struct AiPlayer {
  AiPlayerState state;
  Vector2 target_pos;
  Vector2 crosshair;
  bool firing;
} AiPlayer;

void ai_init(AiPlayer *ai);

struct Game;
void ai_update(struct Game *game, u32 player_idx);

#endif
