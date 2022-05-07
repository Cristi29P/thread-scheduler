/*@Copyright Paris Cristian-Tanase 2022*/
/*Sursa este scrisa exclusiv de mine si este prelucrata dupa scheletul*/
/*de cod de la SD din anul I (2020).*/
/*Din aceasta cauza, semnaturile pot aparea similare sau unele bucati*/
/*de cod pot sa coincida cu ale altor colegi.*/

#ifndef LINKEDLIST_H__
#define LINKEDLIST_H__

#include "utils.h"

struct Node {
	struct Node *next;
	void *data;
};

struct LinkedList {
	struct Node *head;
	int size;
};

void list_init(struct LinkedList *list);

void add_node(struct LinkedList *list, int nth_node, void *new_data,
		  unsigned int new_data_size);

void *remove_node(struct LinkedList *list, int nth_node);

void *get_node(struct LinkedList *list, int n);

int list_size(struct LinkedList *list);

void free_list_mem(struct LinkedList **list);

#endif /* LINKEDLIST_H__ */
