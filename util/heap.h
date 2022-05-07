/**
 * SD, 2020
 * 
 * Lab #9, BST & Heap
 * 
 * Heap structure and function definitions
 */

#ifndef HEAP_H
#define HEAP_H

#include "utils.h"

#define GO_UP(x)        (((x) - 1) >> 1)
#define GO_LEFT(x)      (((x) << 1) + 1)
#define GO_RIGHT(x)     (((x) << 1) + 2)

typedef struct heap_t heap_t;
struct heap_t {
    /* heap elements */
    void **arr;

    /* Effective size, capacity and size of values stored */
    size_t size, capacity;

    /* Function used for comparing the keys */
    int	(*cmp)(const void *key1, const void *key2);

    /* Function used for freeing a heap element */
    void (*free_func) (const void *);
};

/**
 * Alloc memory for a new heap
 * @cmp_f: pointer to a function used for comparing
 * @free_func: pointer to a function used for freeing an element
 * @return: pointer to the newly created heap
 */
heap_t *heap_create(int (*cmp_f) (const void *, const void *), void (*free_func) (const void *));
/**
 * Insert a new element in a heap
 * @heap: the heap where to insert the new element
 * @element: the element to be inserted in heap
 */
void heap_insert(heap_t *heap, void *element);
/**
 * Get the top element
 * @heap: the heap where to search for the top element
 * @return: the top element
 */
void *heap_top(heap_t *heap);

/**
 * Remove the top element
 */
void heap_pop(heap_t *heap);

/**
 * Check if the heap is empty
 * @heap: the heap to be checked
 * @return: 1 if the heap is empty else 0
 */
int heap_empty(heap_t *heap);

/**
 * Free a heap
 * @heap: the heap to be freed
 */
void heap_free(heap_t *heap);

#endif /* HEAP_H */
