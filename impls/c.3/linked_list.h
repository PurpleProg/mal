#ifndef LINKED_LIST_H
#define LINKED_LIST_H 1

#include <stdlib.h>

typedef struct node {
    void        *data;
    size_t       size;
    struct node *next;
} node_t;

int  is_empty(node_t *head);
void reverse_list(node_t *src, node_t **dest);
int  prepend(node_t **head, void *new_data, size_t size);
void append(node_t *head, void *new_data, size_t size);
void pop(node_t **head);

#endif
