#include "linked_list.h"
#include "gc.h"
#include "printer.h"
#include <gc/gc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void append(node_t *head, void *new_data, size_t size) {
    // if head is null create a new node
    if (head == NULL) {
        head       = GC_MALLOC(sizeof(node_t));
        head->data = NULL;
        // this will fallback to the second check
    }

    // if the data of head is empty, do not creat a new node.
    if (head->data == NULL) {
        // allocate data
        head->data = GC_MALLOC(size);
        if (head->data == NULL) {
            fprintf(stderr, "faild to allocate new data");
        }
        memcpy(head->data, new_data, size);
        return;
    }

    // allocate a new node
    node_t *new_node;
    new_node = (node_t *)GC_MALLOC(sizeof(node_t));
    if (new_node == NULL) {
        fprintf(stderr, "faild to allocate new node");
    }
    new_node->next = NULL;

    // allocate data
    new_node->data = GC_MALLOC(size);
    if (new_node->data == NULL) {
        fprintf(stderr, "faild to allocate new data");
    }
    memcpy(new_node->data, new_data, size);

    node_t *node = head;
    while (node->next != NULL) {
        node = node->next;
    }
    node->next = new_node;
}

int prepend(node_t **head, void *new_data, size_t size) {
    /* add something to the start, begining of the list */

    // if head is null create a new node
    if (head == NULL) {
        head          = GC_MALLOC(sizeof(node_t));
        (*head)->data = NULL;
        // this will fallback to the second check
    }

    // if the data of head is empty, do not creat a new node.
    if ((*head)->data == NULL) {
        // allocate data
        (*head)->data = GC_MALLOC(size);
        if ((*head)->data == NULL) {
            fprintf(stderr, "faild to allocate new data");
        }
        memcpy((*head)->data, new_data, size);
        return 0;
    }

    // allocate a new node
    node_t *new_node;
    new_node = (node_t *)GC_MALLOC(sizeof(node_t));
    if (new_node == NULL) {
        fprintf(stderr, "faild to allocate new node");
        return 1;
    }
    new_node->next = NULL;

    // allocate data for the new_node
    new_node->data = GC_MALLOC(size);
    if (new_node->data == NULL) {
        fprintf(stderr, "faild to allocate new data");
        return 1;
    }

    // copy new_data into the new_node
    memcpy(new_node->data, new_data, size);

    // put current head after new_node
    new_node->next = *head;

    // copy new_node over head
    *head = new_node;

    return 0;
}

void reverse_list(node_t *head) {
    node_t *new_list = GC_MALLOC(sizeof(node_t));
    node_t *node     = head;

    // while (node != NULL) {}
}

void pop(node_t **head) {
    if (*head == NULL || (*head)->next == NULL) {
        *head = NULL;
        return;
    }

    node_t *new_head;
    new_head = (*head)->next;
    (*head)  = new_head;
}
