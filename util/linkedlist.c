#include "linkedlist.h"

void list_init(LinkedList *list, void (*free_func)(void *))
{
	list->head = NULL;
	list->back = NULL;
	list->size = 0;
	list->free_func = free_func;
}

void add_node(LinkedList *list, int n, void *new_data)
{
	Node *prev, *curr, *new_node;

	if (!list || n < 0 || !new_data)
		return;
	
	new_node = calloc(1, sizeof(Node));
	DIE(!new_node, "new_node calloc failed!");

	if (n >= list->size) {
		++(list->size);
		list->back->next = new_node;
		list->back = new_node;
		new_node->next = NULL;
		return;
	}

	prev = NULL;
	curr = list->head;

	for (; n > 0; --n) {
		prev = curr;
		curr = curr->next;
	}

	new_node->next = curr;
	new_node->data = new_data;

	++(list->size);

	if (!prev)
		list->head = new_node;
	else
		prev->next = new_node;
}

void *remove_node(LinkedList *list, int n)
{
	Node *prev, *curr;

	if (!list || !(list->head) || (n < 0))
		return NULL;

	if (n > list->size - 1)
		n = list->size - 1;

	prev = NULL;
	curr = list->head;

	for (; n > 0; --n) {
		prev = curr;
		curr = curr->next;
	}

	--(list->size);
	if (!prev)
		list->head = curr->next;
	else
		prev->next = curr->next;

	return curr;
}

void *get_node(LinkedList *list, int nth_node)
{
	Node *curr;

	if (!list || !(list->head) || (nth_node < 0))
		return NULL;

	if (nth_node >= list->size - 1)
		return list->back;

	curr = list->head;

	for (; nth_node > 0; --nth_node)
		curr = curr->next;

	return curr;
}

int list_size(LinkedList *list)
{
	return list ? list->size : -1;
}

void free_list_mem(LinkedList **pp_list)
{
	Node *tmp;

	if (!pp_list || !(*pp_list))
		return;

	for (; list_size(*pp_list) > 0;) {
		tmp = remove_node(*pp_list, 0);
		(*pp_list)->free_func(tmp->data);
		free(tmp);
	}

	free(*pp_list);
	*pp_list = NULL;
}
