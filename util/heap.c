/**
 * Credits:
 *  SD, 2020
 * 
 * Lab #9, BST & Heap
 * 
 * Task #2 - Heap implementation
 * The following implementation is changed but based on the lab.
 */

#include "heap.h"
#include "utils.h"

heap_t *heap_create(int (*cmp_f) (const void *, const void *), void (*free_func) (const void *), const size_t value_size)
{
	heap_t *heap;

	heap = calloc(1, sizeof(heap_t));
	DIE(!heap, "heap calloc");

	heap->cmp = cmp_f;
	heap->free_func = free_func;
	heap->size = 0;
	heap->capacity = 5;
	heap->value_size = value_size;
	
	heap->arr = calloc(heap->capacity, value_size);
	DIE(!heap->arr, "heap->arr calloc.");

	return heap;
}

void heap_insert_fix(heap_t *heap, int pos) {
	void *aux;
	int p = GO_UP(pos);

	while(pos > 0 && (heap->cmp(heap->arr[p], heap->arr[pos]) > 0)) {
		aux = heap->arr[p];
		heap->arr[p] = heap->arr[pos];
		heap->arr[pos] = aux;

		pos = p;
		p = GO_UP(pos);
	}
}


void swap(void  *v1, void  *v2, size_t  size)  {
    void *temp = calloc(1, size);
    DIE(!temp, "Temp calloc failed!");

    memcpy(temp, v1, size);
    memcpy(v1, v2, size);
    memcpy(v2, temp, size);

    free(temp);
}

void heap_insert(heap_t *heap, void *element)
{
	heap->arr[heap->size] = calloc(1, heap->value_size);
	DIE(!heap->arr[heap->size], "heap_insert calloc.");

	memcpy(heap->arr[heap->size], element, heap->value_size);

	heap_insert_fix(heap, heap->size);

	heap->size++;
	if (heap->size == heap->capacity) {
		heap->capacity *= 2;

		heap->arr = realloc(heap->arr, heap->capacity * heap->value_size);
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

  	if (l < heap->size && heap->cmp(heap->arr[l], heap->arr[pos]) < 0)
    	max = l;

  	if (r < heap->size && heap->cmp(heap->arr[r], heap->arr[max]) < 0)
    	max = r;

	if (max != pos) {
		swap(heap->arr[pos], heap->arr[max], heap->value_size);
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

void heap_free(heap_t *heap, void (*free_func) (const void *))
{
	for(int i = 0; i != heap->size; ++i)
		free_func(heap->arr[i]);

	free(heap->arr);
	free(heap);
}
