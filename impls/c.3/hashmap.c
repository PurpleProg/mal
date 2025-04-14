#include <stdio.h>
#include "linked_list.h"
#include "hashmap.h"
#include "types.h"
#include "gc.h"
#include <string.h>


int map_set(map_t * map, MalSymbol * key, MalType * value){
    append(map, key, sizeof(*key));
    append(map, value, sizeof(*value));
    return 0;
}

MalType * map_get(map_t * map, MalSymbol * key){

    node_t * current_node = map;
    if (map == NULL) {
        printf("map is null\n");
    }
    while (current_node != NULL){
        // here data must be a MalSymbol (aka char *)
        if (strcmp(key, current_node->data) == 0) {
            return current_node->next->data;
        }
        // skip values
        current_node = current_node->next->next;
    }
    printf("key : %s not found\n", key);
    return NULL;
}

int map_contains(map_t * map, MalSymbol * key){
    node_t * current_node = map;
    while (current_node != NULL){
        // here data must be a MalSymbol (aka char *)
        if (strcmp(key, current_node->data)) {
            return 1;
        }
        // skip values
        current_node = current_node->next->next;
    }
    return 0;
}

void map_remove(map_t * map, MalSymbol * key){
    node_t * current_node = map;
    while (current_node != NULL){
        if (current_node->data == key) {
            // skip the k/v pair to be removed, gc will dealocate (thanks)
            current_node = current_node->next->next;
            return;
        }
        // skip values
        current_node = current_node->next->next;
    }
}
