#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "entity.h"
#include "entity_list.h"

#define ENTITY_CAPACITY 1024
#define CHANGES_CAPACITY 128

struct EntityList {
  u32 size;
  u32 capacity;

  Entity entities[ENTITY_CAPACITY];

  EntityListChange changes[CHANGES_CAPACITY];
  u32 changes_size;
};

EntityList el_create() {
  EntityList el = malloc(sizeof(struct EntityList));
  *el = (struct EntityList){
      .size = 0,
      .capacity = ENTITY_CAPACITY,
      .entities = {0},
      .changes = {0},
      .changes_size = 0,
  };

  return el;
}

Entity *el_add(EntityList el, Entity *entity) {
  if (el->size == el->capacity) {
    printf("Fatal: maximum entity capacity exceeded\n");
    assert(false);
  }

  el->entities[el->size] = *entity;
  if (el->changes_size == CHANGES_CAPACITY) {
    printf("Fatal: too many entities created/destroyed in one frame\n");
    assert(false);
  }
  el->changes[el->changes_size++] = (EntityListChange){
      .idx = el->size,
      .type = entity->type,
      .create_data = ent_get_create_data(entity),
  };

  free(entity);
  return &el->entities[el->size++];
}

u64 el_indexof(EntityList el, Entity *entity) { return entity - el->entities; }

Entity *el_get(EntityList el, u32 idx) { return &el->entities[idx]; }

void el_set(EntityList el, u32 idx, Entity *entity) {
  el->entities[idx] = *entity;
  if (el->size < idx + 1) {
    el->size = idx + 1;
  }
}

void el_destroy(EntityList el, u32 idx) {
  el->entities[idx] = el->entities[--el->size];
  if (el->changes_size == CHANGES_CAPACITY) {
    printf("Fatal: too many entities created/destroyed in one frame\n");
    assert(false);
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

void el_free(EntityList el) { free(el); }
