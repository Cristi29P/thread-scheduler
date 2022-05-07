/*@Copyright Paris Cristian-Tanase 2022*/
/*Sursa este scrisa exclusiv de mine si este prelucrata dupa scheletul*/
/*de cod de la SD din anul I (2020).*/
/*Din aceasta cauza, semnaturile pot aparea similare sau unele bucati*/
/*de cod pot sa coincida cu ale altor colegi.*/

#ifndef HASHMAP_H__
#define HASHMAP_H__

#include "linkedlist.h"
#include "utils.h"

struct pair {
	void *key;
	void *value;
};

struct Hashmap {
	struct LinkedList *buckets;
	int size;
	int hmax;
	unsigned int (*hash_function)(void *arg);
	int (*compare_function)(void *arg1, void *arg2);
};

void init_ht(struct Hashmap *ht, int hmax,
	     unsigned int (*hash_function)(void *),
	     int (*compare_function)(void *, void *));

void put(struct Hashmap *ht, void *key, int key_size_bytes, void *value,
	 unsigned int value_size);

void *get(struct Hashmap *ht, void *key);

int has_key(struct Hashmap *ht, void *key);

void remove_ht_entry(struct Hashmap *ht, void *key);

int get_ht_size(struct Hashmap *ht);

void free_ht(struct Hashmap *ht);

int cmp_strings(void *a, void *b);

unsigned int hash_function_string(void *a);

#endif /* HASHMAP_H__ */
