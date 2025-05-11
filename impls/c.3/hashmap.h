#ifndef HASHMAP_H
#define HASHMAP_H

#include "linked_list.h"

// map
typedef node_t map_t;

int   map_set(map_t *map, char *key, void *value);
void *map_get(map_t *map, char *key);
int   map_contains(map_t *map, char *key);
void  map_remove(map_t *map, char *key);

#endif
