#ifndef ENTITY_LIST_H
#define ENTITY_LIST_H

#include "entity.h"

typedef struct EntityList *EntityList;

typedef struct EntityListEntry *EntityListIterator;

EntityList el_create();

void el_add(EntityList el, Entity *entity);

void el_destroy(EntityList el, Entity *entity);

EntityListIterator el_iter(EntityList el);

Entity *eli_next(EntityListIterator *eli);

#endif
