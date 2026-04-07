#include <stddef.h>
#include <stdlib.h>

#include "entity.h"
#include "entity_list.h"

typedef struct EntityListEntry {
  Entity *entity;
  struct EntityListEntry *next;
} *EntityListEntry;

EntityListEntry el_new_entry(Entity *entity) {
  EntityListEntry entry = malloc(sizeof(struct EntityListEntry));
  entry->entity = entity;
  entry->next = NULL;
  return entry;
}

struct EntityList {
  EntityListEntry first;
  EntityListEntry last;
};

EntityList el_create() {
  EntityList el = malloc(sizeof(struct EntityList));
  el->first = NULL;
  el->last = NULL;
  return el;
}

void el_add(EntityList el, Entity *entity) {
  EntityListEntry entry = el_new_entry(entity);

  if (el->last == NULL) {
    el->first = el->last = entry;
  } else {
    el->last->next = entry;
    el->last = entry;
  }
}

EntityListEntry el_destroy_impl(EntityList el, EntityListEntry entry,
                                Entity *entity) {
  if (entry == NULL) {
    return entry;
  } else if (entry->entity == entity) {
    EntityListEntry next = entry->next;
    free(entity);
    free(entry);
    return next;
  } else {
    entry->next = el_destroy_impl(el, entry->next, entity);

    // Update last pointer if the last element changed
    if (entry->next == NULL) {
      el->last = entry;
    }

    return entry;
  }
}

void el_destroy(EntityList el, Entity *entity) {
  el->first = el_destroy_impl(el, el->first, entity);
}

EntityListIterator el_iter(EntityList el) { return el->first; }

Entity *eli_next(EntityListIterator *eli) {
  Entity *entity = NULL;

  if (*eli != NULL) {
    entity = (*eli)->entity;
    *eli = (*eli)->next;
  }

  return entity;
}
