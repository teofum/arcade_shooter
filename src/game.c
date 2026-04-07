#include <stdlib.h>

#include "entity.h"
#include "entity_list.h"
#include "game.h"
#include "player.h"

Game game_init() {
  Game game = malloc(sizeof(struct Game));

  game->world = el_create();
  game->player = el_add(game->world, ENT_PLAYER);
  game->player->position = (Vector2){200, 200};
  game->player->update = player_update;
  game->player->draw = player_draw;

  return game;
}

void game_process_input(Game game) {
  // TODO
}

void game_update(Game game) {
  EntityListIterator it = el_iter(game->world);
  Entity *e;
  while ((e = eli_next(&it))) {
    if (e->update != NULL) {
      e->update(e, game);
    }
  }
}

void game_draw(Game game) {
  BeginDrawing();
  ClearBackground(BLACK);

  EntityListIterator it = el_iter(game->world);
  Entity *e;
  while ((e = eli_next(&it))) {
    if (e->draw != NULL) {
      e->draw(e, game);
    }
  }

  EndDrawing();
}
