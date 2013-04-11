#ifndef ARRAY_H
#define ARRAY_H

struct array {
	int size;
	int free;
	void **entries;
};

typedef struct array array_t;

array_t *array_new();
void array_delete(array_t *array);
void array_ensure(array_t *array, int size);

void *array_get(array_t *array, int index);
void array_set(array_t *array, int index, void *value);
void array_push(array_t *array, void *value);

void array_foreach_idx(array_t *array, void (*func)(void *value, int index, void *ctx), void *ctx);

#endif /* end of include guard: ARRAY_H */
