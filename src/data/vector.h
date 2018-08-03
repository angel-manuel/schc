#ifndef SCHC_DATA_VECTOR_H_
#define SCHC_DATA_VECTOR_H_

#include <stdlib.h>

typedef struct vector_ {
    void *mem;
    size_t len;
    size_t cap;
    size_t elem_size;
} vector_t;

int vector_init(vector_t *vector, size_t elem_size);
int vector_init_with_cap(vector_t *vector, size_t elem_size,
                         size_t initial_capacity);
void vector_destroy(vector_t *vector);

const void *vector_get_ref(const vector_t *vector, size_t index);
void *vector_alloc_elem(vector_t *vector);
void *vector_push_back(vector_t *vector, void *item_ptr);

#endif /*SCHC_DATA_VECTOR_H_*/