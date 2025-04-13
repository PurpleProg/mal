#include "types.h"
#include "env.h"


int set(env_t * env, MalSymbol * key, MalType * value) {
    return map_set(env->map, key, value);
}


MalType * get(env_t * env, MalSymbol * key){
    return map_get(env->map, key);
}
