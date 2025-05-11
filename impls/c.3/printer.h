#ifndef PRINTER_H
#define PRINTER_H 1

#include "types.h"

char *pr_str(MalType *AST, int print_readably);
char *pr_env(env_t *ent);

#endif
