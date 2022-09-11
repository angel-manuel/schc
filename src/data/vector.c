#include "vector.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define VECTOR_DEFAULT_INITIAL_CAP 4

#define VALLOC(size) ALLOCATOR_ALLOC(vector->allocator, (size))
#define VREALLOC(ptr, size) ALLOCATOR_REALLOC(vector->allocator, (ptr), (size))
#define VFREE(mem) ALLOCATOR_FREE(vector->allocator, (mem))

int vector_init(vector_t *vector, size_t elem_size) {
    return vector_init_with_cap(vector, elem_size, VECTOR_DEFAULT_INITIAL_CAP);
}

int vector_init_with_cap(vector_t *vector, size_t elem_size,
                         size_t initial_capacity) {
    return vector_init_with_cap_and_allocator(
        vector, elem_size, initial_capacity, &default_allocator);
}

int vector_init_with_allocator(vector_t *vector, size_t elem_size,
                               allocator_t *allocator) {
    assert(vector != NULL);
    assert(elem_size > 0);
    assert(allocator != NULL);

    return vector_init_with_cap_and_allocator(
        vector, elem_size, VECTOR_DEFAULT_INITIAL_CAP, allocator);
}

int vector_init_with_cap_and_allocator(vector_t *vector, size_t elem_size,
                                       size_t initial_capacity,
                                       allocator_t *allocator) {
    assert(vector != NULL);
    assert(elem_size > 0);
    assert(initial_capacity >= 0);
    assert(allocator != NULL);

    vector->allocator = allocator;
    vector->elem_size = elem_size;
    vector->len = 0;
    vector->cap = initial_capacity;
    vector->mem = VALLOC(elem_size * initial_capacity);
    if (vector->mem == NULL) {
        return -1;
    }

    return 0;
}

void vector_destroy(vector_t *vector) {
    assert(vector != NULL);
    assert(vector->mem != NULL);

    VFREE(vector->mem);
}

void *vector_get_mem(vector_t *vector) {
    assert(vector != NULL);
    assert(vector->mem != NULL);

    return vector->mem;
}

const void *vector_get_ref(const vector_t *vector, size_t index) {
    assert(vector != NULL);
    assert(vector->mem != NULL);

    if (index >= vector->len) {
        return NULL;
    } else {
        return vector->mem + (vector->elem_size * index);
    }
}

size_t vector_get_len(const vector_t *vector) {
    assert(vector != NULL);

    return vector->len;
}

void *vector_alloc_elem(vector_t *vector) {
    assert(vector != NULL);
    assert(vector->mem != NULL);

    if (vector->len >= vector->cap) {
        if (vector_grow(vector)) {
            return NULL;
        }
    }

    return vector->mem + (vector->len++ * vector->elem_size);
}

void *vector_push_back(vector_t *vector, void *item_ptr) {
    assert(vector != NULL);
    assert(item_ptr != NULL);

    void *dst_mem = vector_alloc_elem(vector);
    if (dst_mem == NULL) {
        return NULL;
    }
    memcpy(dst_mem, item_ptr, vector->elem_size);

    return dst_mem;
}

int vector_grow(vector_t *vector) {
    if (vector->cap == 0) {
        vector->cap = VECTOR_DEFAULT_INITIAL_CAP;
    } else {
        vector->cap *= 2;
    }

    void *new_mem = VREALLOC(vector->mem, vector->elem_size * vector->cap);
    if (new_mem == NULL) {
        return -1;
    }
    vector->mem = new_mem;

    return 0;
}