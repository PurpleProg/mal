#ifndef CORE_H
#define CORE_H

#include "env.h"

env_t   *create_repl();
MalType *wrap_function(MalType *(*func)(node_t *node));

#endif
