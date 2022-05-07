#include "hashmap.h"
#include "linkedlist.h"

int cmp_strings(void *a, void *b) { return strcmp((char *)a, (char *)b); }

/*
 * Credits: https://stackoverflow.com/a/12996028/7883884
 */
unsigned int hash_function_string(void *a)
{
	unsigned char *puchar_a = (unsigned char *)a;
	unsigned long hash = 5381;
	int c;

	while ((c = *puchar_a++))
		hash = ((hash << 5u) + hash) + c; /* hash * 33 + c */

	return hash;
}

void init_ht(struct Hashmap *ht, int hmax,
	     unsigned int (*hash_function)(void *),
	     int (*compare_function)(void *, void *))
{
	int i = 0;

	ht->size = 0;
	ht->hmax = hmax;
	ht->hash_function = hash_function;
	ht->compare_function = compare_function;

	ht->buckets =
	    (struct LinkedList *)calloc(hmax, sizeof(struct LinkedList));
	if (ht->buckets == NULL)
		exit(12);

	while (i < hmax) {
		list_init(&ht->buckets[i]);
		i++;
	}
}

void put(struct Hashmap *ht, void *key, int key_size_bytes, void *value,
	 unsigned int value_size)
{
	struct pair info_tmp;
	int index = ht->hash_function(key) % ht->hmax;
	struct Node *it = ht->buckets[index].head;

	for (; it != NULL; it = it->next) {
		if (ht->compare_function(((struct pair *)it->data)->key, key) ==
		    0) {

			free((((struct pair *)it->data)->key));
			free((((struct pair *)it->data)->value));
			free(it->data);

			it->data = calloc(1, sizeof(struct pair));
			DIE(it->data == NULL,
			    "Memory allocation for the new_node->data failed!");

			((struct pair *)it->data)->key =
			    calloc(1, key_size_bytes);
			DIE(((struct pair *)it->data)->key == NULL,
			    "Memory allocation for info_tmp->key failed!");

			memcpy(((struct pair *)it->data)->key, key,
			       key_size_bytes);
			DIE(((struct pair *)it->data)->key == NULL,
			    "Memory allocation for info_tmp->key failed!");

			((struct pair *)it->data)->value =
			    calloc(1, value_size);
			DIE(((struct pair *)it->data)->value == NULL,
			    "Memory allocation for info_tmp->key failed!");

			memcpy(((struct pair *)it->data)->value, value,
			       value_size);
			DIE(((struct pair *)it->data)->value == NULL,
			    "Memory allocation for info_tmp->key failed!");

			return;
		}
	}

	info_tmp.key = calloc(1, key_size_bytes);
	if (info_tmp.key == NULL)
		exit(12);

	memcpy(info_tmp.key, key, key_size_bytes);
	DIE(info_tmp.key == NULL,
	    "Memory allocation for info_tmp->key failed!");

	info_tmp.value = calloc(1, value_size);
	if (info_tmp.value == NULL)
		exit(12);

	memcpy(info_tmp.value, value, value_size);
	DIE(info_tmp.value == NULL,
	    "Memory allocation for info_tmp->key failed!");

	add_node(&ht->buckets[index], list_size(&ht->buckets[index]), &info_tmp,
		 sizeof(struct pair));

	ht->size++;
}

void *get(struct Hashmap *ht, void *key)
{
	int index = ht->hash_function(key) % ht->hmax;
	struct Node *it = ht->buckets[index].head;

	for (; it != NULL; it = it->next) {
		if (ht->compare_function(((struct pair *)it->data)->key, key) ==
		    0) {
			return ((struct pair *)it->data)->value;
		}
	}

	return NULL;
}

int has_key(struct Hashmap *ht, void *key)
{
	int index = ht->hash_function(key) % ht->hmax;
	struct Node *it = ht->buckets[index].head;

	for (; it != NULL; it = it->next) {
		if (ht->compare_function(((struct pair *)it->data)->key, key) ==
		    0) {
			return 1;
		}
	}

	return 0;
}

void remove_ht_entry(struct Hashmap *ht, void *key)
{
	int index = ht->hash_function(key) % ht->hmax, pozitie = 0;
	struct Node *tmp, *it = ht->buckets[index].head;

	for (; it != NULL; pozitie++, it = it->next) {
		if (ht->compare_function(((struct pair *)it->data)->key, key) ==
		    0) {
			break;
		}
	}

	tmp = remove_node(&ht->buckets[index], pozitie);
	ht->size--;

	free((((struct pair *)tmp->data)->key));
	free((((struct pair *)tmp->data)->value));
	free(tmp->data);
	free(tmp);
}

void free_ht(struct Hashmap *ht)
{
	int i;
	struct LinkedList *lista_curenta;
	struct Node *it, *tmp;

	for (i = 0; i < ht->hmax; i++) {
		lista_curenta = &ht->buckets[i];
		it = lista_curenta->head;

		for (; it != NULL;) {
			tmp = it;
			it = it->next;
			tmp->next = NULL;
			free((((struct pair *)tmp->data)->key));
			free((((struct pair *)tmp->data)->value));
			free(tmp->data);
			free(tmp);
		}
	}
	free(ht->buckets);
	free(ht);
}

int get_ht_size(struct Hashmap *ht) { return (ht == NULL) ? -1 : ht->size; }
