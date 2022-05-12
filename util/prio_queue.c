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
    
    queue->list = calloc(1, sizeof(LinkedList));
    DIE(!(queue->list), "queue->list calloc failed!");

    list_init(queue->list, free_func);

    return queue;
}

void queue_push(prio_queue_t *queue, void *val)
{
    int cnt = 0;
    Node *first = get_node(queue->list, 0);

    if (!queue || !(queue->list) || !val)
        return;

    /* If first element */
    if (!queue->size) {
        ++(queue->size);
        add_node(queue->list, 0, val);
        return;
    }
    
    /* Get correct position in queue */
    while (first && (queue->cmp(val, first->data) <= 0)) {
        ++cnt;
        first = get_node(queue->list, cnt);
        if (cnt == queue->list->size)
            break;
    }

    ++(queue->size);
    
    add_node(queue->list, cnt, val);
}

void *queue_pop(prio_queue_t *queue)
{
    Node *temp;
    void *val;

    if (!queue || !(queue->list) || !queue->size)
        return NULL;
    
    temp = (Node *)(remove_node(queue->list, 0));
    val = temp->data;

    --(queue->size);

    free(temp);
    return val;
}

void *queue_top(prio_queue_t *queue)
{
    return queue ? ((Node *)(get_node(queue->list, 0)))->data : NULL;
}

void queue_free(prio_queue_t *queue)
{
    if (!queue || !(queue->list))
        return;

    free_list_mem(&(queue->list));
    free(queue);
}

int queue_size(prio_queue_t *queue)
{
    return queue ? queue->size : -1;
}
