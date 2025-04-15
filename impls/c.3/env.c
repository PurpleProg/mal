#include <stdio.h>
#include "types.h"
#include "env.h"


int set(env_t * env, MalSymbol * key, MalType * value) {
    return map_set(env->data, key, value);
}


MalType * get(env_t * env, MalSymbol * key){
    MalType * ret =  map_get(env->data, key);
    if (ret == NULL && env->outer != nil){
        return get(env->outer, key);
    }
    if (ret == NULL && env->outer == nil) {
        printf("key : %s not found\n", key);
    }
    return ret;
}
