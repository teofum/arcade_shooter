#include <assert.h>
#include <stdio.h>

#include "ai.h"
#include "entity.h"
#include "game.h"

void ai_init(AiPlayer *ai) { ai->state = AI_IDLE; }

void ai_update(Game game, u32 player_idx) {
  if (game->player_type[player_idx] != PLAYER_AI) {
    printf("Fatal: AI update on non-AI player %u\n", player_idx);
    assert(false);
  }

  Entity *player = game->players[player_idx];
  AiPlayer *ai = &game->ai_players[player_idx];
  InputData *input = &game->server.input[player_idx];

  switch (ai->state) {
  case AI_IDLE: {
    printf("AI update %u\n", player_idx);
    break;
  }
  }
}
