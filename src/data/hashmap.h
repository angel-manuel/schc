#ifndef SCHC_DATA_HASHMAP_H_
#define SCHC_DATA_HASHMAP_H_

#include <stdlib.h>

#include "allocator.h"
#include "vector.h"

typedef struct hashmap_ {
    allocator_t *allocator;
    size_t cap;
    size_t len;
    size_t elem_size;
    int cap_pow;
    void *mem;
    void (*elem_destroy)(void *);
    vector_t /* const char* */ keys;
} hashmap_t;

int hashmap_init(hashmap_t *hashmap, size_t elem_size,
                 void (*elem_destroy)(void *));
int hashmap_init_with_cap(hashmap_t *hashmap, size_t elem_size,
                          size_t initial_capacity,
                          void (*elem_destroy)(void *));
int hashmap_init_with_cap_and_allocator(hashmap_t *hashmap, size_t elem_size,
                                        size_t initial_capacity,
                                        allocator_t *allocator,
                                        void (*elem_destroy)(void *));
void hashmap_destroy(hashmap_t *hashmap);

const vector_t /* const char * */ *hashmap_keys(const hashmap_t *hashmap);
int hashmap_put(hashmap_t *hashmap, const char *key, const void *elem);
int hashmap_put_no_alloc(hashmap_t *hashmap, char *key, const void *elem);
void *hashmap_get(hashmap_t *hashmap, const char *key);
const void *hashmap_get_const(const hashmap_t *hashmap, const char *key);

#endif /*SCHC_DATA_HASHMAP_H_*/
