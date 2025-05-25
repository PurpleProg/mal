#ifndef ENV_H
#define ENV_H

#include "types.h"

env_t   *create_env(env_t *outer, MalType *binds, MalType *exprs);
int      set(env_t *env, MalType *key, MalType *value);
MalType *get(env_t *env, MalType *key);

#endif
