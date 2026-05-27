#ifndef ENTITY_LIST_H
#define ENTITY_LIST_H

#include "entity.h"

typedef struct EntityList *EntityList;

typedef struct EntityListIterator *EntityListIterator;

EntityList el_create();

Entity *el_add(EntityList el, Entity *entity);

Entity *el_get(EntityList el, u32 idx);

EntityListIterator el_iter(EntityList el);

Entity *eli_next(EntityListIterator eli);

void eli_destroy_current(EntityListIterator eli);

void el_free(EntityList el);

#endif
