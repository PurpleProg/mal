#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gc.h"
#include "linked_list.h"


void append(node_t * head, void * new_data, size_t size) {
	// if the data of head is empty, do not creat a new node.
	if (head->data == NULL) {
		// allocate data
		head->data = GC_MALLOC(size);
		if (head->data == NULL) {
			fprintf(stderr, "faild to allocate new data");
		}
		memcpy(head->data, new_data, size);
		head->next = NULL;
		return;
	}

	// allocate a new node
	node_t * new_node;
	new_node = (node_t *)GC_MALLOC(sizeof(node_t));
	if (new_node == NULL) {
		fprintf(stderr, "faild to allocate new node");
	}

	// allocate data
	new_node->data = GC_MALLOC(size);
	if (new_node->data == NULL) {
		fprintf(stderr, "faild to allocate new data");
	}
	memcpy(new_node->data, new_data, size);
	new_node->next = NULL;

	node_t * node = head;
	while (node->next != NULL) {
		node = node->next;
	}
	node->next = new_node;
}

void pop(node_t ** head) {
	if (*head == NULL || (*head)->next == NULL) {
		free(*head);
		*head = NULL;
		return;
	}

	node_t * new_head;
	new_head = (*head)->next;
	free(*head);
	(*head) = new_head;
}
