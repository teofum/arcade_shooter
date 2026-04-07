#include <stdlib.h>

#include "entity.h"

Entity *ent_create(EntityType type) {
  Entity *entity = calloc(1, sizeof(Entity));
  entity->type = type;
  return entity;
}
