#include "env.h"
#include "gc.h"
#include "printer.h"
#include "types.h"
#include <gc/gc.h>
#include <stdio.h>

int set(env_t *env, MalType *key, MalType *value) {
    return map_set(env->data, key, value);
}

MalType *get(env_t *env, MalType *key) {
    MalType *ret = map_get(env->data, key);
    if (ret == NULL && env->outer != NULL) {
        return get(env->outer, key);
    }
    if (ret == NULL && env->outer == NULL) {
        global_error = key;
        printf("'%s' not found.\n", pr_str(key, 0));
        return key;
    }
    return ret;
}

env_t *create_env(env_t *outer, MalType *binds, MalType *exprs) {
    env_t *env = GC_MALLOC(sizeof(env_t));
    env->data  = GC_MALLOC(sizeof(map_t));
    env->outer = outer;

    if (binds == NULL) {
        return env;
    }
    if (!IsListOrVector(binds)) {
        printf("env binds must be a list or a vector\n");
        return env;
    }
    if (!IsListOrVector(exprs)) {
        printf("env exprs must be a list or a vector\n");
        return env;
    }

    node_t *binds_list = GetList(binds);
    node_t *exprs_list = GetList(exprs);

    if (binds_list == NULL || exprs_list == NULL) {
        return env;
    }
    if (binds_list->data == NULL || exprs_list->data == NULL) {
        return env;
    }
    while (binds_list != NULL && exprs_list != NULL) {
        if (!IsSymbol(binds_list->data) && !IsKeyword(binds_list->data)) {
            printf("env binds must be symbols or keyword\n");
            return NULL;
        }
        set(env, binds_list->data, exprs_list->data);

        binds_list = binds_list->next;
        exprs_list = exprs_list->next;
    }
    if ((binds_list == NULL) ^ (exprs_list == NULL)) {
        fprintf(stderr, "binds_list and exprs are different lenght, some have "
                        "been discarded\n");
    }

    return env;
}
