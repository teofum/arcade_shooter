#ifndef ENTITY_LIST_H
#define ENTITY_LIST_H

#include "entity.h"

typedef struct EntityListChange {
  u32 idx;
  EntityType type;
  EntityCreateData create_data;
} EntityListChange;

typedef struct EntityList *EntityList;

typedef struct EntityListIterator *EntityListIterator;

EntityList el_create();

Entity *el_add(EntityList el, Entity *entity);

u64 el_indexof(EntityList el, Entity *entity);

Entity *el_get(EntityList el, u32 idx);

void el_set(EntityList el, u32 idx, Entity *entity);

void el_destroy(EntityList el, u32 idx);

u32 el_size(EntityList el);

void el_flush_changes(EntityList el);

EntityListChange *el_get_changes(EntityList el, u32 *size);

EntityListIterator el_iter(EntityList el);

Entity *eli_next(EntityListIterator eli);

void eli_destroy_current(EntityListIterator eli);

void el_free(EntityList el);

#endif
