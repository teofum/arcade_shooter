#ifndef ENTITY_H
#define ENTITY_H

#include "raylib.h"

typedef enum {
  ENT_PLAYER,
  ENT_BULLET,
  ENT_WALL,
} EntityType;

struct Entity;
struct Game;

typedef void (*EntityUpdateFunction)(struct Entity *entity, struct Game *game);
typedef void (*EntityDrawFunction)(struct Entity *entity, struct Game *game);

typedef struct Entity {
  EntityType type;

  Vector2 position;
  void *custom_data;

  EntityUpdateFunction update;
  EntityDrawFunction draw;
} Entity;

Entity *ent_create(EntityType type);

#endif
