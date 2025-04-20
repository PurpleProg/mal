#ifndef ENV_H
#define ENV_H

#include "types.h"


env_t * create_env(env_t * outer, MalType * binds, node_t * exprs);
int set(env_t * env, MalSymbol * key, MalType * value);
MalType * get(env_t * env, MalSymbol * key);

#endif
