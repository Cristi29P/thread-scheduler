/**
 * @Copyright Paris Cristian-Tanase 2022
 * Sursa este scrisa exclusiv de mine si este prelucrata dupa scheletul
 * de cod de la SD din anul I (2020).
 * Link GIT linkedlist: https://github.com/Cristi29P/c-preprocessor.git
 */

#ifndef LINKEDLIST_H_
#define LINKEDLIST_H_

#include "utils.h"

typedef struct Node Node;
struct Node {
	Node *next;
	void *data;
};

typedef struct LinkedList LinkedList;
struct LinkedList {
	Node *head;
	Node *back;
	int size;
	void (*free_func)(void *a);
};

void list_init(LinkedList *list, void (*free_func)(void *));

void add_node(LinkedList *list, int nth_node, void *new_data);

void *remove_node(LinkedList *list, int nth_node);

void *get_node(LinkedList *list, int nth_node);

int list_size(LinkedList *list);

void free_list_mem(LinkedList **list);

#endif /* LINKEDLIST_H_ */
