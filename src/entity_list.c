#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

#include "entity.h"
#include "entity_list.h"

#define INITIAL_CAPACITY 1024
#define CHANGES_CAPACITY 128

struct EntityList {
  u32 size;
  u32 capacity;

  Entity *entities;

  EntityListChange changes[CHANGES_CAPACITY];
  u32 changes_size;
};

struct EntityListIterator {
  EntityList list;
  u32 i;
};

EntityList el_create() {
  EntityList el = malloc(sizeof(struct EntityList));
  *el = (struct EntityList){
      .size = 0,
      .capacity = INITIAL_CAPACITY,
      .entities = malloc(sizeof(Entity) * INITIAL_CAPACITY),
      .changes = {0},
      .changes_size = 0,
  };

  return el;
}

static void el_resize(EntityList el) {
  el->size *= 2;
  Entity *new_list = realloc(el->entities, el->size);
  el->entities = new_list;
}

Entity *el_add(EntityList el, Entity *entity) {
  if (el->size == el->capacity) {
    el_resize(el);
  }

  el->entities[el->size] = *entity;
  if (el->changes_size == CHANGES_CAPACITY) {
    assert(false && "too many entities created/destroyed in one frame");
  }
  el->changes[el->changes_size++] = (EntityListChange){
      .idx = el->size,
      .type = entity->type,
  };

  return &el->entities[el->size++];
}

Entity *el_get(EntityList el, u32 idx) { return &el->entities[idx]; }

void el_destroy(EntityList el, u32 idx) {
  el->entities[idx] = el->entities[--el->size];
  if (el->changes_size == CHANGES_CAPACITY) {
    assert(false && "too many entities created/destroyed in one frame");
  }
  el->changes[el->changes_size++] = (EntityListChange){
      .idx = idx,
      .type = -1,
  };
}

u32 el_size(EntityList el) { return el->size; }

void el_flush_changes(EntityList el) { el->changes_size = 0; }

EntityListChange *el_get_changes(EntityList el, u32 *size) {
  *size = el->changes_size;
  return el->changes;
}

EntityListIterator el_iter(EntityList el) {
  EntityListIterator eli = malloc(sizeof(struct EntityListIterator));
  *eli = (struct EntityListIterator){
      .list = el,
      .i = 0,
  };

  return eli;
}

Entity *eli_next(EntityListIterator eli) {
  Entity *entity = NULL;

  if (eli->i < eli->list->size) {
    entity = &eli->list->entities[eli->i++];
  }

  return entity;
}

void eli_destroy_current(EntityListIterator eli) {
  EntityList el = eli->list;
  el->entities[--eli->i] = el->entities[--el->size];

  if (el->changes_size == CHANGES_CAPACITY) {
    assert(false && "too many entities created/destroyed in one frame");
  }
  el->changes[el->changes_size++] = (EntityListChange){
      .idx = eli->i,
      .type = -1,
  };
}

void el_free(EntityList el) {
  free(el->entities);
  free(el);
}
