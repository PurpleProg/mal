#include "env.h"
#include "gc.h"
#include "hashmap.h"
#include "types.h"
#include <gc/gc.h>
#include <stdio.h>

int set(env_t *env, MalSymbol *key, MalType *value) {
    return map_set(env->data, key, value);
}

MalType *get(env_t *env, MalSymbol *key) {
    MalType *ret = map_get(env->data, key);
    if (ret == NULL && env->outer != NULL) {
        return get(env->outer, key);
    }
    if (ret == NULL && env->outer == NULL) {
        printf("key : %s not found\n", key);
    }
    return ret;
}

env_t *create_env(env_t *outer, MalType *binds, node_t *exprs) {
    env_t *env = GC_MALLOC(sizeof(env_t));
    env->data  = GC_MALLOC(sizeof(map_t));
    if (outer == NULL) {
        env->outer = NULL;
    } else {
        env->outer = outer;
    }

    if (binds == NULL) {
        return env;
    }
    if (binds->type != MAL_LIST && binds->type != MAL_VECTOR) {
        printf("env binds must be a list or a vector\n");
        return env;
    }
    node_t *binds_list = binds->value.ListValue;

    if (binds_list == NULL || exprs == NULL) {
        return env;
    }
    if (binds_list->data == NULL || exprs->data == NULL) {
        return env;
    }
    while (binds_list != NULL && exprs != NULL) {
        if (((MalType *)binds_list->data)->type != MAL_SYMBOL) {
            printf("env binds must be symbols\n");
            return NULL;
        }
        char *bind = ((MalType *)binds_list->data)->value.SymbolValue;
        set(env, bind, exprs->data);

        binds_list = binds_list->next;
        exprs      = exprs->next;
    }
    if ((binds_list == NULL) ^ (exprs == NULL)) {
        fprintf(stderr, "binds_list and exprs are different lenght, some have "
                        "been discarded\n");
    }

    return env;
}
