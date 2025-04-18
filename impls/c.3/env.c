#include <stdio.h>
#include "gc.h"
#include "types.h"
#include "hashmap.h"
#include "env.h"


int set(env_t * env, MalSymbol * key, MalType * value) {
    return map_set(env->data, key, value);
}


MalType * get(env_t * env, MalSymbol * key){
    MalType * ret =  map_get(env->data, key);
    if (ret == NULL && env->outer != NULL){
        return get(env->outer, key);
    }
    if (ret == NULL && env->outer == NULL) {
        printf("key : %s not found\n", key);
    }
    return ret;
}


env_t * create_env(env_t * outer, node_t * binds, node_t * exprs) {
    env_t * env = GC_MALLOC(sizeof(env_t));
    env->data = GC_MALLOC(sizeof(map_t));
    if (outer == NULL) {
        env->outer = NULL;
    } else {
        env->outer = outer;
    }

    if (binds == NULL || exprs == NULL) {
        return env;
    }
    if (binds->data == NULL || exprs->data == NULL) {
        return env;
    }
    while (binds != NULL && exprs != NULL) {
        set(env, binds->data, exprs->data);

        binds = binds->next;
        exprs = exprs->next;
    }
    if ((binds == NULL) ^ (exprs == NULL)) {
        fprintf(stderr, "binds and exprs are different lenght, some have been discarded\n");
    }


    return env;
}
