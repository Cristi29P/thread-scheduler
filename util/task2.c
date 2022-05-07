/**
 * SD, 2020
 * 
 * Lab #9, BST & Heap
 * 
 * Task #2 - Test Heap Implementation
 */

#include <stdio.h>
#include <stdlib.h>

#include "heap.h"

typedef struct team_t team_t;
struct team_t {
    char        *name;
    int         score;
};


char to_lower(char c)
{
	if ('A' <= c && c <= 'Z')
		return c + 0x20;
	return c;
}

int heap_cmp_str_lexicographically(const char *key1, const char *key2)
{
	int rc, i, len;

	len = strlen(key1) < strlen(key2) ? strlen(key1) : strlen(key2);
	for (i = 0; i < len; ++i) {
		rc = to_lower(key1[i]) - to_lower(key2[i]);

		if (rc == 0)
			continue;
		return rc;
	}

	rc = to_lower(key1[i]) - to_lower(key2[i]);
	return rc;
}

int heap_cmp_teams(const void *a, const void *b)
{
    team_t *key1 = (team_t *)a;
    team_t *key2 = (team_t *)b;

    int score_diff = key2->score - key1->score;

    if (score_diff != 0)
        return score_diff;

    return heap_cmp_str_lexicographically(key1->name, key2->name);
}

void free_team(const void *team) {
    free(((team_t *)team)->name);
}

int main(void)
{
    heap_t *heap;
    team_t tmp_team;
    int N, task;

    heap = heap_create(heap_cmp_teams, free_team);

    scanf("%d", &N);

    while (N--) {
        scanf("%d", &task);
        if (task == 1) {
            team_t team;
            team.name = malloc(BUFSIZ * sizeof(char));
            if (team.name == NULL)
                perror("team.name malloc");

            scanf("%s %d", team.name, &team.score);
            heap_insert(heap, &team);
        } else if (task == 2) {
            if (!heap_empty(heap)) {
                tmp_team = *(team_t *)heap_top(heap);
                printf("%s %d\n", tmp_team.name, tmp_team.score);
            }
        } else if (task == 3) {
            if (!heap_empty(heap)) {
                heap_pop(heap);
            }
        } else {
            perror("Invalid task!");
        }
    }  

    while (heap->size > 0) {
        printf("%s %d\n", ((team_t *)(heap->arr[0]))->name, ((team_t *)(heap->arr[0]))->score);
        heap_pop(heap);
    }

    heap_free(heap);

    return 0;
}
