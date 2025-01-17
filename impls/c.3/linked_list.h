#ifndef LINKED_LIST_H
#define LINKED_LIST_H 1

typedef struct node {
	int data;
	struct node * next;
} node_t;


void print(node_t * node);
void append(node_t ** head, int new_data);
void pop(node_t ** head);
int lenght(node_t * head);
int in(node_t * head, int data);

#endif
