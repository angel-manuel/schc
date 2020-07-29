#include "hashmap.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

#include <data/vector.h>
#include <util.h>

#define HASHMAP_DEFAULT_CAP 1024
#define FIBONACCI_MULT UINT64_C(11400714819323198486)

#define ALLOC(size) ALLOCATOR_ALLOC(hashmap->allocator, (size))
#define STRALLOC(str) ALLOCATOR_STRALLOC(hashmap->allocator, (str))
#define REALLOC(ptr, size) ALLOCATOR_REALLOC(hashmap->allocator, (ptr), (size))
#define FREE(mem) ALLOCATOR_FREE(hashmap->allocator, (mem))

typedef struct hashmap_location_ {
    char *key;
    unsigned char data[0];
} hashmap_location_t;

hashmap_location_t *hashmap_get_entry(hashmap_t *hashmap, size_t i, void *mem);
int hashmap_grow(hashmap_t *hashmap);
uint64_t hash(const char *str);

int hashmap_init(hashmap_t *hashmap, size_t elem_size,
                 void (*elem_destroy)(void *)) {
    assert(hashmap != NULL);
    assert(elem_size >= 0);

    return hashmap_init_with_cap(hashmap, elem_size, HASHMAP_DEFAULT_CAP,
                                 elem_destroy);
}

int hashmap_init_with_cap(hashmap_t *hashmap, size_t elem_size,
                          size_t initial_capacity,
                          void (*elem_destroy)(void *)) {
    return hashmap_init_with_cap_and_allocator(
        hashmap, elem_size, initial_capacity, &default_allocator, elem_destroy);
}

int hashmap_init_with_cap_and_allocator(hashmap_t *hashmap, size_t elem_size,
                                        size_t initial_capacity,
                                        allocator_t *allocator,
                                        void (*elem_destroy)(void *)) {

    assert(hashmap != NULL);
    assert(elem_size >= 0);
    assert(initial_capacity >= 256);
    assert(allocator != NULL);

    size_t cap = 256;
    int cap_pow = 8;

    while (cap < initial_capacity) {
        cap *= 2;
        cap_pow++;
    }

    hashmap->allocator = allocator;
    hashmap->len = 0;
    hashmap->cap = cap;
    hashmap->elem_size = elem_size;
    hashmap->cap_pow = cap_pow;
    hashmap->elem_destroy = elem_destroy;

    TRYCR(hashmap->mem, ALLOC(cap * (sizeof(hashmap_location_t) + elem_size)),
          NULL, -1);

    memset(hashmap->mem, 0, (sizeof(hashmap_location_t) + elem_size) * cap);

    int res;
    TRY(res, vector_init_with_cap_and_allocator(
                 &hashmap->keys, sizeof(const char *), cap, allocator));

    return 0;
}

void hashmap_destroy(hashmap_t *hashmap) {
    assert(hashmap != NULL);

    size_t vlen = hashmap->cap;

    for (size_t i = 0; i < vlen; ++i) {
        hashmap_location_t *loc = hashmap_get_entry(hashmap, i, NULL);

        if (loc != NULL && loc->key != NULL) {
            FREE(loc->key);

            if (hashmap->elem_destroy) {
                hashmap->elem_destroy(&loc->data);
            }
        }
    }

    FREE(hashmap->mem);

    vector_destroy(&hashmap->keys);
}

const vector_t /* const char * */ *hashmap_keys(const hashmap_t *hashmap) {
    assert(hashmap != NULL);

    return &hashmap->keys;
}

int hashmap_put(hashmap_t *hashmap, const char *key, const void *elem) {
    assert(hashmap != NULL);
    assert(key != NULL);
    assert(elem != NULL);

    char *owned_key;

    TRYCR(owned_key, STRALLOC(key), NULL, -1);

    return hashmap_put_no_alloc(hashmap, owned_key, elem);
}

int hashmap_put_no_alloc(hashmap_t *hashmap, char *key, const void *elem) {
    assert(hashmap != NULL);
    assert(key != NULL);
    assert(elem != NULL);
    assert(hashmap->len < hashmap->cap);

    int res;
    int key_exists = 0;

    uint64_t h = hash(key);
    uint64_t fh = (h * FIBONACCI_MULT) >> (64 - hashmap->cap_pow);

    hashmap_location_t *loc = hashmap_get_entry(hashmap, fh, NULL);

    while (loc->key) {
        if (strcmp(loc->key, key) == 0) {
            FREE(loc->key);
            hashmap->len--;
            key_exists = 1;
            break;
        }

        // Capacity is always a power of 2 so this is modulo
        fh = (fh + 1) & (hashmap->cap - 1);
        loc = hashmap_get_entry(hashmap, fh, NULL);
    }

    if (!key_exists) {
        vector_push_back(&hashmap->keys, &key);
    }

    loc->key = key;
    memcpy(loc->data, elem, hashmap->elem_size);
    hashmap->len++;

    if (hashmap->len * 2 >= hashmap->cap) {
        TRY(res, hashmap_grow(hashmap));
    }

    return 0;
}

void *hashmap_get(hashmap_t *hashmap, const char *key) {
    assert(hashmap != NULL);
    assert(key != NULL);

    uint64_t h = hash(key);
    uint64_t fh = (h * FIBONACCI_MULT) >> (64 - hashmap->cap_pow);

    hashmap_location_t *loc = hashmap_get_entry(hashmap, fh, NULL);

    size_t limit = hashmap->cap;
    while (loc->key && limit--) {
        if (strcmp(key, loc->key) == 0) {
            return loc->data;
        }

        fh = (fh + 1) & (hashmap->cap - 1);
        loc = hashmap_get_entry(hashmap, fh, NULL);
    }

    return NULL;
}

const void *hashmap_get_const(const hashmap_t *hashmap, const char *key) {
    assert(hashmap != NULL);
    assert(key != NULL);

    return hashmap_get((void *)hashmap, key);
}

int hashmap_grow(hashmap_t *hashmap) {
    assert(hashmap != NULL);

    int res;

    void *old_mem = hashmap->mem;
    size_t old_cap = hashmap->cap;

    size_t new_cap = hashmap->cap * 2;
    void *new_mem;

    TRYCR(new_mem,
          ALLOC(new_cap * (sizeof(hashmap_location_t) + hashmap->elem_size)),
          NULL, -1);
    memset(new_mem, 0,
           new_cap * (sizeof(hashmap_location_t) + hashmap->elem_size));
    hashmap->mem = new_mem;
    hashmap->cap_pow++;
    hashmap->cap = new_cap;
    hashmap->len = 0;

    for (size_t i = 0; i < old_cap; ++i) {
        const hashmap_location_t *old_loc =
            hashmap_get_entry(hashmap, i, old_mem);

        if (old_loc->key) {
            TRY(res,
                hashmap_put_no_alloc(hashmap, old_loc->key, old_loc->data));
        }
    }

    FREE(old_mem);

    return 0;
}

hashmap_location_t *hashmap_get_entry(hashmap_t *hashmap, size_t i, void *mem) {
    assert(hashmap != NULL);
    assert(i >= 0);
    assert(mem != NULL || i < hashmap->cap);

    if (mem == NULL) {
        mem = hashmap->mem;
    }

    return (hashmap_location_t *)(((char *)mem) +
                                  i * (sizeof(hashmap_location_t) +
                                       hashmap->elem_size));
}

// Hash

uint64_t hash(const char *str) {
    uint64_t res = 0;
    const char *ptr = str;
    uint64_t buf = 0;

    while (*ptr != '\0') {
        strncpy((char *)&buf, ptr, sizeof(uint64_t));

        for (int i = 0; i < sizeof(uint64_t) && *ptr != '\0'; ++i, ++ptr)
            ;

        res = (res << 23) || (res >> 41);
        res ^= buf;
    }

    return res;
}
