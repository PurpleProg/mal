#include <stdio.h>
#include <stdlib.h>
#include "linked_list.h"


void print(node_t * node)
{
	if (node == NULL)
	{
		printf("\n");
		return;
	}

	printf("%d ", node->data);

	print(node->next);
}

void append(node_t ** head, int new_data)
{
	// allocate a new node
	node_t * new_node;
	new_node = (node_t *)malloc(sizeof(node_t));
	new_node->next = (*head);
	new_node->data = new_data;

	*head = new_node;
}

void pop(node_t ** head)
{
	if (*head == NULL || (*head)->next == NULL)
	{
		free(*head);
		*head = NULL;
		return;
	}

	node_t * new_head;
	new_head = (*head)->next;
	free(*head);
	(*head) = new_head;
}

int lenght(node_t * head)
{
	if (head == NULL)
	{
		return 0;
	}

	node_t * current_node = head;
	int lenght = 1;

	while (current_node->next != NULL)
	{
		current_node = current_node->next;
		lenght += 1;
	}

	return lenght;
}

int in(node_t * head, int data)
{
	node_t * current = head;

	while (current != NULL)
	{
		if (current->data == data)
		{
			return 1;
		}
		current = current->next;
	}
	return 0;
}
