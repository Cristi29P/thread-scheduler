#ifndef PRIO_QUEUE_H_
#define PRIO_QUEUE_H_

#include "linkedlist.h"

typedef struct prio_queue_t prio_queue_t;
struct prio_queue_t
{   
    /* List used for storing elements */
    LinkedList *list; 
    /* Queue size */
    unsigned int size; 
    /* Function used for freeing a custom element */
	void (*free_func)(void *);
    /* Compare function for priority */
    int	(*cmp)(const void *a, const void *b);
};

prio_queue_t *queue_init(int (*cmp)(const void *a, const void *b), void (*free_func)(void *));





#endif /* PRIO_QUEUE_H_ */