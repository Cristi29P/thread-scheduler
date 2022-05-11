#include "prio_queue.h"

prio_queue_t *queue_init(int (*cmp)(const void *a, const void *b), void (*free_func)(void *))
{
    prio_queue_t *queue = calloc(1, sizeof(prio_queue_t));
    DIE(!queue, "queue calloc failed!");

    DIE(!cmp, "NULL pointer to cmp not allowed!");
	queue->cmp        = cmp;

	DIE(!free_func, "NULL pointer to free_func not allowed!");
	queue->free_func  = free_func;

	queue->size       = 0;

    list_init(queue->list, free_func);

    return queue;
}

