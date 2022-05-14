/**
 * @Copyright Paris Cristian-Tanase 2022
 * Sursa este scrisa exclusiv de mine de la zero si se foloseste
 * de implementarea de lista inlantuita (modificata, de asemenea,
 * pentru aceasta tema) din cadrul temei 1 de la S.O.
 * Link GIT linkedlist: https://github.com/Cristi29P/c-preprocessor.git
 */

#ifndef PRIO_QUEUE_H_
#define PRIO_QUEUE_H_

#include "linkedlist.h"

typedef struct prio_queue_t prio_queue_t;
struct prio_queue_t
{   
    /* List used for storing elements */
    LinkedList *list; 
    /* Queue size */
    int size; 
    /* Function used for freeing a custom element */
	void (*free_func)(void *);
    /* Compare function for priority */
    int	(*cmp)(const void *a, const void *b);
};

prio_queue_t *queue_init(int (*cmp)(const void *a, const void *b), void (*free_func)(void *));

void queue_push(prio_queue_t *queue, void *val);

void *queue_pop(prio_queue_t *queue);

void *queue_top(prio_queue_t *queue);

void queue_free(prio_queue_t *queue);

int queue_size(prio_queue_t *queue);

#endif /* PRIO_QUEUE_H_ */