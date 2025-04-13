#ifndef ENV_H
#define ENV_H

#include "types.h"
#include "hashmap.h"


typedef struct env {
    struct env * outer;
    map_t * map;
} env_t;


int set(env_t * env, MalSymbol * key, MalType * value);
MalType * get(env_t * env, MalSymbol * key);

#endif
