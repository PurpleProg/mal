#ifndef HASHMAP_H
#define HASHMAP_H


#include "types.h"


int map_set(map_t * map, MalSymbol * key, MalType * value);
MalType * map_get(map_t * map, MalSymbol * key);
int map_contains(map_t * map, MalSymbol * key);
void map_remove(map_t * map, MalSymbol * key);

#endif
