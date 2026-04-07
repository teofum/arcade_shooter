#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>

#include "config.h"
#include "entity.h"
#include "entity_list.h"
#include "game.h"
#include "player.h"

Game game_init() {
  Game game = malloc(sizeof(struct Game));

  game->world = el_create();
  game->player = el_add(game->world, ENT_PLAYER);
  game->player->position = (Vector2){200, 200};
  game->player->custom_data = player_init_data();
  game->player->update = player_update;
  game->player->draw = player_draw;

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

  v_target = Vector2Scale(Vector2Normalize(v_target), PLAYER_SPEED);

  PlayerData *pdata = (PlayerData *)game->player->custom_data;
  pdata->velocity = Vector2Lerp(pdata->velocity, v_target, PLAYER_ACCEL);

  // Aiming
  pdata->crosshair = GetMousePosition();
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
