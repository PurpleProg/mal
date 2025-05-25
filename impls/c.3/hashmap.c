#include "gc.h"
#include "linked_list.h"
#include "types.h"
#include <gc/gc.h>
#include <stdio.h>
#include <string.h>

char *GetStringValue(MalType *atom) {
    return atom->value.StringValue;
}

int map_set(map_t *map, MalType *key, MalType *value) {
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
            // accesing throug StringValue to hande symbol or keyword
            if (strcmp(GetStringValue(key),
                       GetStringValue(current_node->data)) == 0) {
                // replace old value with new one
                current_node->next->data = value;
                return 0;
            }
            // skip values
            current_node = current_node->next->next;
        }

    } else {
        append(map, key, sizeof(MalType));
        append(map, value, sizeof(MalType));
    }
    return 0;
}

void *map_get(map_t *map, MalType *key) {
    node_t *current_node = map;

    if (map == NULL) {
        return NULL;
    }
    if (map->data == NULL) {
        return NULL;
    }
    while (current_node != NULL) {
        if (strcmp(GetStringValue(key), GetStringValue(current_node->data)) ==
            0) {
            return current_node->next->data;
        }
        // skip values
        current_node = current_node->next->next;
    }
    return NULL;
}

int map_contains(map_t *map, MalType *key) {
    node_t *current_node = map;
    while (current_node != NULL) {
        if (current_node->data == NULL) {
            // the map is empty
            return 0;
        }
        if (strcmp(GetStringValue(key), GetStringValue(current_node->data)) ==
            0) {
            return 1;
        }
        // skip values
        current_node = current_node->next->next;
    }
    return 0;
}

void map_remove(map_t *map, MalType *key) {
    printf("Test this\n");
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
