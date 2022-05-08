/**
 * SD, 2020
 * 
 * Lab #9, BST & Heap
 * 
 * Heap structure and function definitions
 */

#ifndef __HEAP_H_
#define __HEAP_H_

#include <stdlib.h>
#include <string.h>
#include <stdio.h>


#define GO_UP(x)        (((x) - 1) >> 1)
#define GO_LEFT(x)      (((x) << 1) + 1)
#define GO_RIGHT(x)     (((x) << 1) + 2)

typedef struct heap_t heap_t;
struct heap_t {
    /* heap elements */
    void **arr;

    int size, capacity;

    /* function used for sorting the keys */
    int	(*cmp)(const void *a, const void *b);

    void (*inner_free)(const void *);
};

/**
 * Alloc memory for a new heap
 * @cmp_f: pointer to a function used for sorting
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
 * @return: the top element
 */
void* heap_top(heap_t *heap);

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

#endif