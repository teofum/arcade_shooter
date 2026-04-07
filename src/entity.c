#include <stdlib.h>

#include "entity.h"

Entity *ent_create(EntityType type, const char *name) {
  Entity *entity = malloc(sizeof(Entity));
  entity->type = type;
  entity->name = name;
  return entity;
}
