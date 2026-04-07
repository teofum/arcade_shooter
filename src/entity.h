#ifndef ENTITY_H
#define ENTITY_H

typedef enum {
  ENT_PLAYER,
} EntityType;

typedef struct Entity {
  EntityType type;
  const char *name;
} Entity;

Entity *ent_create(EntityType type, const char *name);

#endif
