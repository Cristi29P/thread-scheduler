/**
 * SD, 2020
 * 
 * Lab #9, BST & Heap
 * 
 * Task #2 - Heap implementation
 */

#include "heap.h"

heap_t *heap_create(mode_op_t mode, int (*cmp_f) (const void *a, const void *b), void (*inner_free) (void *), size_t capacity)
{
	heap_t *heap;

	heap = calloc(1, sizeof(heap_t));
	DIE(!heap, "heap calloc failed!");

	heap->mode = mode;

	DIE(!cmp_f, "NULL pointer to cmp_f not allowed!");
	heap->cmp        = cmp_f;

	DIE(!inner_free, "NULL pointer to inner_free not allowed!");
	heap->inner_free = inner_free;

	heap->size       = 0;

	DIE(capacity <= 0, "Capacity must be greater than 0!");
	heap->capacity   = (int)capacity;

	heap->arr        = calloc(heap->capacity, sizeof(void *));
	DIE(!(heap->arr), "heap->arr calloc failed!");

	return heap;
}

void heap_insert_fix(heap_t *heap, int pos)
{
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

void heap_insert(heap_t *heap, void *val)
{
	if (!heap || !(heap->arr) || !val) {
		fprintf(stderr, "Invalid insert parameter. One or more is NULL!");
		return;
	}

	heap->arr[heap->size] = val;

	if (heap->mode == PRIO_QUEUE)
		heap_insert_fix(heap, heap->size);

	++heap->size;
	if (heap->size == heap->capacity) {
		heap->capacity *= 2;

		heap->arr = realloc(heap->arr, heap->capacity * sizeof(void *));
		DIE(!(heap->arr), "heap->arr realloc");
	}
}

void* heap_top(heap_t *heap)
{
	return heap ? heap->arr[0] : NULL;
}

void heap_pop_fix(heap_t *heap, int pos) {
	void *temp;
	int l = GO_LEFT(pos), r = GO_RIGHT(pos), maximus = pos;

	if (l < heap->size && heap->cmp(heap->arr[l], heap->arr[pos]) < 0)
		maximus = l;

	if (r < heap->size && heap->cmp(heap->arr[r], heap->arr[maximus]) < 0)
		maximus = r;

	if (maximus != pos) {
		temp = heap->arr[pos];
		heap->arr[pos] = heap->arr[maximus];
		heap->arr[maximus] = temp;

		heap_pop_fix(heap, maximus);
  }
}

void heap_pop(heap_t *heap)
{
	if (heap_empty(heap)) {
		return;
	}
	
	if (heap->mode == PRIO_QUEUE)
		heap->arr[0] = heap->arr[heap->size - 1];
	else {
		void *temp = heap->arr[0];
		for(int i=0; i != heap->size - 1; ++i)
        	heap->arr[i] = heap->arr[i + 1];
    	heap->arr[heap->size - 1] = temp;
	}

	--heap->size;
	if (heap->mode == PRIO_QUEUE)
		heap_pop_fix(heap, 0);
}

int heap_empty(heap_t *heap)
{
	return heap ? (heap->size == 0) : -1;
}

int heap_size(heap_t *heap)
{
	return heap ? heap->size : -1;
}

void heap_free(heap_t *heap)
{	
	if (!heap || !(heap->arr)) {
		fprintf(stderr, "heap or heap->arr is NULL!");
		return;
	}
		
	if (!heap_empty(heap))
		for(int i = 0; i != heap->size; ++i) {
			heap->inner_free(heap->arr[i]);
			free(heap->arr[i]);
		}
	
	free(heap->arr);
	free(heap);
}
