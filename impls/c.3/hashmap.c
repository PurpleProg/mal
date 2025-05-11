#include "hashmap.h"
#include "gc.h"
#include "linked_list.h"
#include "types.h"
#include <gc/gc.h>
#include <stdio.h>
#include <string.h>

int map_set(map_t *map, char *key, void *value) {
    if (map_contains(map, key)) {
        node_t *current_node = map;
        if (map == NULL) {
            printf("map is null\n");
        }
        while (current_node != NULL) {
            // here data must be a MalSymbol (aka char *)
            if (key == NULL) {
                printf("key is null");
            }
            if (current_node->data == NULL) {
                printf("data is null");
            }
            if (strcmp(key, current_node->data) == 0) {
                // replace old value with new one
                current_node->next->data = value;
                return 0;
            }
            // skip values
            current_node = current_node->next->next;
        }

    } else {
        append(map, key, strlen(key) + 1);
        append(map, value, sizeof(MalType));
    }
    return 0;
}

void *map_get(map_t *map, char *key) {
    node_t  *current_node = map;
    MalType *nil          = GC_MALLOC(sizeof(MalType));
    nil->type             = MAL_NIL;
    nil->value.NilValue   = NULL;

    if (map == NULL) {
        printf("map is null\n");
        return nil;
    }
    if (map->data == NULL) {
        printf("map is empty\n");
        return nil;
    }
    while (current_node != NULL) {
        // here data must be a MalSymbol (aka char *)
        if (strcmp(key, current_node->data) == 0) {
            return current_node->next->data;
        }
        // skip values
        current_node = current_node->next->next;
    }
    return nil;
}

int map_contains(map_t *map, char *key) {
    node_t *current_node = map;
    while (current_node != NULL) {
        if (current_node->data == NULL) {
            // the map is empty
            return 0;
        }
        // here data must be a MalSymbol (aka char *)
        // or is it ? spend hours because it was not (just why did i think it
        // would be)
        if (strcmp(key, current_node->data) == 0) {
            return 1;
        }
        // skip values
        current_node = current_node->next->next;
    }
    return 0;
}

void map_remove(map_t *map, char *key) {
    // not tested at all probably broken somehow but hey i dont need it yet
    node_t *current_node = map;
    while (current_node != NULL) {
        if (current_node->data == key) {
            // skip the k/v pair to be removed, gc will dealocate (thanks)
            break;
        }
        // skip values
        current_node = current_node->next->next;
    }
}
