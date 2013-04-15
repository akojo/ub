#include "array.h"
#include <string.h>
#undef NDEBUG
#include <assert.h>

typedef struct test_foreach_ctx {
    int numcalls;
    void *values[2];
    int indeces[2];
} test_foreach_ctx_t;

typedef struct test_element {
    int value;
} test_element_t;

void reset_test_foreach_ctx(test_foreach_ctx_t *ctx) {
    memset(ctx, 0, sizeof(*ctx));
}

void test_foreach_func(void *value, int index, void *ctx) {
    test_foreach_ctx_t *c = (test_foreach_ctx_t*)ctx;
    c->values[c->numcalls] = value;
    c->indeces[c->numcalls] = index;
    ++c->numcalls;
}

void test_new_array_is_empty() {
    array_t *array = array_new();
    test_foreach_ctx_t ctx;
    reset_test_foreach_ctx(&ctx);

    array_foreach_idx(array, &test_foreach_func, &ctx);
    assert(ctx.numcalls == 0);

    array_delete(array);
}

void test_push_single_element() {
    array_t *array = array_new();
    test_element_t elem;
    array_push(array, &elem);

    assert(array_get(array, 0) == &elem);

    array_delete(array);
}

void test_iterate_single_element() {
    array_t *array = array_new();
    test_element_t elem;
    array_push(array, &elem);

    test_foreach_ctx_t ctx;
    reset_test_foreach_ctx(&ctx);
    array_foreach_idx(array, &test_foreach_func, &ctx);
    assert(ctx.numcalls == 1);
    assert(ctx.values[0] == &elem);
    assert(ctx.indeces[0] == 0);

    array_delete(array);
}

void test_set_single_element() {
    array_t *array = array_new();
    test_element_t elem;
    array_ensure(array, 1);
    array_set(array, 0, &elem);

    assert(array_get(array, 0) == &elem);

    array_delete(array);
}

void test_unset_elements_are_null() {
    array_t *array = array_new();
    test_element_t elem;
    array_ensure(array, 2);
    array_set(array, 1, &elem);

    assert(array_get(array, 0) == NULL);
    assert(array_get(array, 1) == &elem);

    array_delete(array);
}

void test_push_goes_after_last_set() {
    array_t *array = array_new();
    test_element_t elem1, elem2;
    array_ensure(array, 3);
    array_set(array, 1, &elem1);
    array_push(array, &elem2);

    assert(array_get(array, 1) == &elem1);
    assert(array_get(array, 2) == &elem2);

    array_delete(array);
}

void test_each_value_equals_index_plus1(void *value, int index, void *ctx) {
    assert((int)value == index + 1);
}

void test_ensure_expands_array() {
    array_t *array = array_new();
    array_ensure(array, 1000);
    for (int i = 0; i < 1000; ++i) {
        array_set(array, i, (void*)(i + 1));
    }

    for (int i = 0; i < 1000; ++i) {
        assert(array_get(array, i) == (void*)(i + 1));
    }
    array_foreach_idx(array, &test_each_value_equals_index_plus1, NULL);
    array_delete(array);
}

void test_push_expands_array() {
    array_t *array = array_new();
    for (int i = 0; i < 1000; ++i) {
        array_push(array, (void*)(i + 1));
    }

    for (int i = 0; i < 1000; ++i) {
        assert(array_get(array, i) == (void*)(i + 1));
    }
    array_foreach_idx(array, &test_each_value_equals_index_plus1, NULL);
    array_delete(array);
}

int main(void) {
    test_new_array_is_empty();
    test_push_single_element();
    test_iterate_single_element();
    test_set_single_element();
    test_unset_elements_are_null();
    test_push_goes_after_last_set();
    test_ensure_expands_array();
    test_push_expands_array();
}
