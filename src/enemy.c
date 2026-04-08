#include <raylib.h>
#include <stdlib.h>

#include "config.h"
#include "enemy.h"
#include "entity.h"
#include "entity_list.h"
#include "utils.h"

#define GRID_SIZE ((float)FIELD_WIDTH / FIELD_COLS)

static EnemyData *enemy_init_data(Vector2 size) {
  EnemyData *data = malloc(sizeof(EnemyData));
  data->size = size;

  data->health = 50;

  return data;
}

Entity *enemy_create(u32 x, u32 y, u32 w, u32 h) {
  Entity *enemy = ent_create(ENT_ENEMY);

  enemy->position = (Vector2){
      -FIELD_WIDTH / 2.0f + x * GRID_SIZE,
      -FIELD_HEIGHT / 2.0f + y * GRID_SIZE,
  };
  Vector2 size = {w * GRID_SIZE, h * GRID_SIZE};

  enemy->custom_data = enemy_init_data(size);

  enemy->update = enemy_update;
  enemy->draw = enemy_draw;

  return enemy;
}

void enemy_update(Entity *enemy, Game game) {
  EnemyData *data = (EnemyData *)enemy->custom_data;

  // Die
  if (data->health <= 0) {
    el_destroy(game->world, enemy);
    return;
  }

  enemy->position.y += game->delta_time * 5;

  // Destroy self on reaching the bottom of the screen
  if (enemy->position.y > FIELD_HEIGHT / 2.0f - data->size.y / 2.0f) {
    // TODO damage player
    el_destroy(game->world, enemy);
  }
}

void enemy_draw(Entity *enemy, Game game) {
  EnemyData *data = (EnemyData *)enemy->custom_data;

  Rectangle rect = {enemy->position.x, enemy->position.y, data->size.x,
                    data->size.y};
  rect = game_to_screen_rect(rect);

  Rectangle rect2 = rect;
  rect2.height *= data->health / 50.0f;

  DrawRectangleRec(rect, RED);
  DrawRectangleRec(rect2, ORANGE);
  DrawRectangleLinesEx(rect, 1.0f, BLACK);
}
