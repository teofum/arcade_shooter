#ifndef AI_H
#define AI_H

#include "types.h"

typedef enum {
  AI_IDLE,
} AiPlayerState;

typedef struct AiPlayer {
  AiPlayerState state;
} AiPlayer;

void ai_init(AiPlayer *ai);

struct Game;
void ai_update(struct Game *game, u32 player_idx);

#endif
