#ifndef HASHMAP_H
#define HASHMAP_H


#include "linked_list.h"
#include "types.h"

typedef node_t map_t;

int map_set(map_t * map, MalSymbol * key, MalType * value);
MalType * map_get(map_t * map, MalSymbol * key);
int map_contains(map_t * map, MalSymbol * key);
void map_remove(map_t * map, MalSymbol * key);

#endif
