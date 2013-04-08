#include <stdlib.h>
#include <string.h>

#include "array.h"

#define DEFAULT_SIZE 32

array_t *array_new()
{
	array_t *new = calloc(1, sizeof(struct array));
	new->size = DEFAULT_SIZE;
	new->free = 0;
	new->entries = calloc(new->size, sizeof(void *));
	return new;
}

void array_ensure(array_t *a, int size)
{
	if (a->size < size) {
		while (a->size < size)
			a->size *= 2;
		a->entries = realloc(a->entries, a->size * sizeof(void));
		memset(a->entries + a->free, 0, a->size - a->free);
	}
}

void *array_get(array_t *a, int index)
{
	return a->entries[index];
}

void array_set(array_t *a, int index, void *value)
{
	a->entries[index] = value;
	if (a->free <= index)
		a->free = index + 1;
}

void array_push(array_t *a, void *value)
{
	array_ensure(a, a->free + 1);
	a->entries[a->free++] = value;
}

void array_foreach_idx(array_t *a, void (*func)(void *, int, void *), void *ctx)
{
	int i;
	for (int i = 0; i < a->free; i++)
		func(a->entries[i], i, ctx);
}
