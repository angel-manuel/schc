#ifndef SCHC_DATA_HASHMAP_H_
#define SCHC_DATA_HASHMAP_H_

#include <stdlib.h>

typedef struct hashmap_ {
    size_t cap;
    size_t len;
    size_t elem_size;
    int cap_pow;
    void *mem;
} hashmap_t;

int hashmap_init(hashmap_t *hashmap, size_t elem_size);
int hashmap_init_with_cap(hashmap_t *hashmap, size_t elem_size,
                          size_t initial_capacity);
void hashmap_destroy(hashmap_t *hashmap);

int hashmap_put(hashmap_t *hashmap, const char *key, const void *elem);
void *hashmap_get(hashmap_t *hashmap, const char *key);

#endif /*SCHC_DATA_HASHMAP_H_*/
