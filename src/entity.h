#ifndef ENTITY_H
#define ENTITY_H

#include "raylib.h"

typedef enum {
  ENT_PLAYER,
  ENT_BULLET,
  ENT_WALL,
  ENT_ENEMY,
  ENT_DMG_NUMBER,
  ENT_XP_GEM,
  ENT_POWERUP,
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
