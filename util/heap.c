/**
 * Credits:
 * SD, 2020
 * 
 * Lab #9, BST & Heap
 * 
 * Task #2 - Heap implementation
 * The following implementation is changed but based on the lab.
 */

#include "heap.h"

void swap(void  *v1, void  *v2) {
    void *temp;
	temp = v1;
	v1 = v2;
	v2 = temp;
}

heap_t *heap_create(int (*cmp_f) (const void *, const void *), void (*free_func) (const void *))
{
	heap_t *heap;

	heap = calloc(1, sizeof(heap_t));
	DIE(!heap, "heap calloc");

	heap->cmp = cmp_f;
	heap->free_func = free_func;
	heap->size = 0;
	heap->capacity = 2;
	
	heap->arr = calloc(heap->capacity, sizeof(void *));
	DIE(!heap->arr, "heap->arr calloc");

	return heap;
}

void heap_insert_fix(heap_t *heap, int pos) {
	int p = GO_UP(pos);

	while(pos > 0 && (heap->cmp(heap->arr[p], heap->arr[pos]) > 0)) {
		swap(heap->arr[p], heap->arr[pos]);

		pos = p;
		p = GO_UP(pos);
	}
}

void heap_insert(heap_t *heap, void *element)
{

	heap->arr[heap->size] = element;

	heap_insert_fix(heap, heap->size);

	++heap->size;
	if (heap->size == heap->capacity) {
		heap->capacity *= 2;

		heap->arr = realloc(heap->arr, heap->capacity * sizeof(void *));
		DIE(!heap->arr, "heap->arr realloc");
	}
}

void *heap_top(heap_t *heap)
{
	return heap->arr[0];
}

void heap_pop_fix(heap_t *heap, int pos) {
	int l = GO_LEFT(pos);
  	int r = GO_RIGHT(pos);
  	int max = pos;

  	if (l < (int)heap->size && heap->cmp(heap->arr[l], heap->arr[pos]) < 0)
    	max = l;

  	if (r < (int)heap->size && heap->cmp(heap->arr[r], heap->arr[max]) < 0)
    	max = r;

	if (max != pos) {
		swap(heap->arr[pos], heap->arr[max]);
		heap_pop_fix(heap, max);
	}
}

void heap_pop(heap_t *heap)
{
	heap->free_func(heap->arr[0]);

	heap->arr[0] = heap->arr[heap->size - 1];
	heap->size--;

	heap_pop_fix(heap, 0);
}

int heap_empty(heap_t *heap)
{
	return heap->size == 0;
}

void heap_free(heap_t *heap)
{
	for(int i = 0; i != (int)heap->size; ++i) {
		heap->free_func(heap->arr[i]);
	}
	
	free(heap->arr);
	free(heap);
}
