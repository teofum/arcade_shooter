#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>

#include "entity.h"
#include "entity_list.h"
#include "game.h"
#include "player.h"

Game game_init() {
  Game game = malloc(sizeof(struct Game));

  game->world = el_create();
  game->player = player_create();
  el_add(game->world, game->player);

  return game;
}

void game_process_input(Game game) {
  // Player movement
  Vector2 v_target = {0, 0};

  if (IsKeyDown(KEY_W))
    v_target.y--;
  if (IsKeyDown(KEY_S))
    v_target.y++;
  if (IsKeyDown(KEY_A))
    v_target.x--;
  if (IsKeyDown(KEY_D))
    v_target.x++;

  PlayerData *pdata = (PlayerData *)game->player->custom_data;
  pdata->direction = Vector2Normalize(v_target);

  // Aiming
  pdata->crosshair = GetMousePosition();

  // Fire!
  pdata->firing = IsKeyPressed(KEY_SPACE);
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
  ClearBackground(WHITE);

  EntityListIterator it = el_iter(game->world);
  Entity *e;
  while ((e = eli_next(&it))) {
    if (e->draw != NULL) {
      e->draw(e, game);
    }
  }

  EndDrawing();
}
