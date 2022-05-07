#include "linkedlist.h"

void list_init(struct LinkedList *list)
{
	list->head = NULL;
	list->size = 0;
}

void add_node(struct LinkedList *list, int n, void *new_data,
	      unsigned int new_data_size)
{
	struct Node *prev, *curr, *new_node;

	if (list == NULL || n < 0)
		return;
	else if (n > list->size)
		n = list->size;

	prev = NULL;
	curr = list->head;

	for (; n > 0; --n) {
		prev = curr;
		curr = curr->next;
	}

	new_node = (struct Node *)calloc(1, sizeof(struct Node));
	if (new_node == NULL)
		exit(12);

	new_node->next = curr;

	new_node->data = calloc(1, new_data_size);
	if (new_node->data == NULL)
		exit(12);

	memcpy(new_node->data, new_data, new_data_size);
	DIE(new_node->data == NULL, "Memcpy failed for the new_node->data!");

	list->size++;

	if (prev == NULL)
		list->head = new_node;
	else
		prev->next = new_node;
}

void *remove_node(struct LinkedList *list, int n)
{
	struct Node *prev, *curr;

	if (list == NULL || list->head == NULL)
		return NULL;

	if (n > list->size - 1)
		n = list->size - 1;

	if (n < 0)
		return NULL;

	prev = NULL;
	curr = list->head;

	for (; n > 0; --n) {
		prev = curr;
		curr = curr->next;
	}

	list->size--;
	if (prev == NULL)
		list->head = curr->next;
	else
		prev->next = curr->next;

	return curr;
}

void *get_node(struct LinkedList *list, int n)
{
	struct Node *curr;

	if (list == NULL || list->head == NULL)
		return NULL;

	if (n > list->size - 1)
		n = list->size - 1;

	if (n < 0)
		return NULL;

	curr = list->head;

	for (; n > 0; --n)
		curr = curr->next;

	return curr;
}

int list_size(struct LinkedList *list)
{
	return (list == NULL) ? -1 : list->size;
}

void free_list_mem(struct LinkedList **pp_list)
{
	struct Node *tmp;

	if (pp_list == NULL)
		return;

	if (*pp_list == NULL)
		return;

	for (; list_size(*pp_list) > 0;) {
		tmp = remove_node(*pp_list, 0);
		free(tmp->data);
		free(tmp);
	}

	free(*pp_list);
	*pp_list = NULL;
}
