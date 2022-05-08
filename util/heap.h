/**
 * SD, 2020
 * 
 * Lab #9, BST & Heap
 * 
 * Heap structure and function definitions
 */

#ifndef HEAP_H_
#define HEAP_H_

#include "utils.h"

#define GO_UP(x)        (((x) - 1) >> 1)
#define GO_LEFT(x)      (((x) << 1) + 1)
#define GO_RIGHT(x)     (((x) << 1) + 2)

typedef struct heap_t heap_t;
struct heap_t {
    /* heap elements */
    void **arr;
    /* number of elements in the heap, initial capacity */
    int size, capacity;
    /* function used for sorting the keys */
    int	(*cmp)(const void *a, const void *b);
    /* function used for freeing up internal structure stored in heap */
    void (*inner_free)(const void *);
};

/**
 * Alloc memory for a new heap
 * @cmp_f: pointer to a function used for sorting
 * @capacity: initial heap capacity
 * @return: pointer to the newly created heap
 */
heap_t *heap_create(int (*cmp_f) (const void *a, const void *b), void (*inner_free) (const void *), size_t capacity);

/**
 * Insert a new element in a heap
 * @heap: the heap where to insert the new element
 * @val: the val to be inserted in heap
 */
void heap_insert(heap_t *heap, void *val);

/**
 * Get the top element
 * @heap: the heap where to search for the top element
 * @return: top element pointer, NULL on error
 */
void* heap_top(heap_t *heap);

/**
 * Remove the top element
 * @heap: the heap where to remove the first element from
 */
void heap_pop(heap_t *heap);

/**
 * Check if the heap is empty
 * @heap: the heap to be checked
 * @return: 1 if the heap is empty else 0, -1 on error
 */
int heap_empty(heap_t *heap);

/**
 * Return number of values stored inside the heap
 * @heap: the heap to be checked
 * @return: the number of values or -1 if error
 */
int heap_size(heap_t *heap);

/**
 * Free a heap
 * @heap: the heap to be freed
 */
void heap_free(heap_t *heap);

#endif /* HEAP_H_ */
