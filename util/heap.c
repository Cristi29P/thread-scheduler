/**
 * SD, 2020
 * 
 * Lab #9, BST & Heap
 * 
 * Task #2 - Heap implementation
 */

#include "heap.h"
#include "utils.h"

heap_t *heap_create(int (*cmp_f) (const void *a, const void *b), void (*inner_free) (const void *), size_t capacity)
{
	heap_t *heap;

	heap = calloc(1, sizeof(heap_t));
	DIE(heap == NULL, "heap malloc");

	heap->cmp       = cmp_f;
	heap->inner_free = inner_free;
	heap->size      = 0;
	heap->capacity  = (int)capacity;
	heap->arr       = calloc(heap->capacity, sizeof(void *));
	DIE(heap->arr == NULL, "heap->arr malloc");

	return heap;
}

static void __heap_insert_fix(heap_t *heap, int pos) {
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
	heap->arr[heap->size] = val;

	__heap_insert_fix(heap, heap->size);

	heap->size++;
	if (heap->size == heap->capacity) {
		heap->capacity *= 2;

		heap->arr = realloc(heap->arr, heap->capacity * sizeof(void *));
		DIE(heap->arr == NULL, "heap->arr realloc");
	}
}

void* heap_top(heap_t *heap)
{
	return heap->arr[0];
}

static void __heap_pop_fix(heap_t *heap, int pos) {
	int l = GO_LEFT(pos);
	int r = GO_RIGHT(pos);
	int maximus = pos;

	void *temp;
  

	if (l < heap->size && heap->cmp(heap->arr[l], heap->arr[pos]) < 0)
	maximus = l;
	if (r < heap->size && heap->cmp(heap->arr[r], heap->arr[maximus]) < 0)
	maximus = r;
	if (maximus != pos) {
	temp = heap->arr[pos];
	heap->arr[pos] = heap->arr[maximus];
	heap->arr[maximus] = temp;

	__heap_pop_fix(heap, maximus);
  }
}

void heap_pop(heap_t *heap)
{
	if (heap_empty(heap)) {
		return;
	}
	
	heap->inner_free(heap->arr[0]);
	free(heap->arr[0]);

	heap->arr[0] = heap->arr[heap->size - 1];

	heap->size--;

	__heap_pop_fix(heap, 0);
}

int heap_empty(heap_t *heap)
{
	return heap->size <= 0;
}

int heap_size(heap_t *heap)
{
	return heap->size;
}

void heap_free(heap_t *heap)
{
	for(int i = 0; i < heap->size; i++) {
		heap->inner_free(heap->arr[i]);
		free(heap->arr[i]);
	}

	free(heap->arr);
	free(heap);
}