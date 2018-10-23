#include "hashmap.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

#include <stdio.h>
#include <inttypes.h>

#include <util.h>
#include <data/vector.h>

#define HASHMAP_DEFAULT_CAP 1024
#define FIBONACCI_MULT UINT64_C(11400714819323198486)

typedef struct hashmap_location_ {
    char *key;
    unsigned char data[0];
} hashmap_location_t;

hashmap_location_t *hashmap_get_entry(hashmap_t *hashmap, size_t i, void *mem);
int hashmap_grow(hashmap_t *hashmap);
uint64_t hash(const char *str);

int hashmap_init(hashmap_t *hashmap, size_t elem_size) {
    assert(hashmap != NULL);
    assert(elem_size >= 0);

    return hashmap_init_with_cap(hashmap, elem_size, HASHMAP_DEFAULT_CAP);
}

int hashmap_init_with_cap(hashmap_t *hashmap, size_t elem_size,
                          size_t initial_capacity) {

    assert(hashmap != NULL);
    assert(elem_size >= 0);
    assert(initial_capacity >= 256);

    size_t cap = 256;
    int cap_pow = 8;

    while (cap < initial_capacity) {
        cap *= 2;
        cap_pow++;
    }

    hashmap->len = 0;
    hashmap->cap = cap;
    hashmap->elem_size = elem_size;
    hashmap->cap_pow = cap_pow;

    TRYCR(hashmap->mem, malloc(cap * (sizeof(hashmap_location_t) + elem_size)), NULL, -1);

    memset(hashmap->mem, 0, (sizeof(hashmap_location_t) + elem_size) * cap);

    return 0;
}

void hashmap_destroy(hashmap_t *hashmap) {
    assert(hashmap != NULL);

    size_t vlen = hashmap->cap;

    for (size_t i = 0; i < vlen; ++i) {
        const hashmap_location_t *loc = hashmap_get_entry(hashmap, i, NULL);

        if (loc != NULL && loc->key != NULL) {
            free(loc->key);
        }
    }

    free(hashmap->mem);
}

int hashmap_put(hashmap_t *hashmap, const char *key, const void *elem) {
    assert(hashmap != NULL);
    assert(key != NULL);
    assert(elem != NULL);

    int res;

    if (hashmap->len >= hashmap->cap) {
        return -1;
    }

    uint64_t h = hash(key);
    uint64_t fh = (h * FIBONACCI_MULT) >> (64 - hashmap->cap_pow);

    hashmap_location_t *loc = hashmap_get_entry(hashmap, fh, NULL);

    if (loc->key) {
        printf("Collision with %s\n", loc->key);
        while (loc->key && strcmp(loc->key, key) != 0) {
            // Capacity is always a power of 2 so this is modulo
            fh = (fh + 1) & (hashmap->cap - 1);
            loc = hashmap_get_entry(hashmap, fh, NULL);
        }

        free(loc->key);
    }

    TRYCR(loc->key, stralloc(key), NULL, -1);
    memcpy(loc->data, elem, hashmap->elem_size);
    hashmap->len++;

    if (hashmap->len * 2 > hashmap->cap) {
        TRY(res, hashmap_grow(hashmap));
    }

    printf("%zd of %zd\n", hashmap->len, hashmap->cap);

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

int hashmap_grow(hashmap_t *hashmap) {
    assert(hashmap != NULL);

    int res;

    void *old_mem = hashmap->mem;
    size_t old_cap = hashmap->cap;

    size_t new_cap = hashmap->cap * 2;
    void *new_mem;

    TRYCR(new_mem, malloc(new_cap * (sizeof(hashmap_location_t) + hashmap->elem_size)), NULL, -1);
    memset(new_mem, 0, new_cap * (sizeof(hashmap_location_t) + hashmap->elem_size));
    hashmap->mem = new_mem;
    hashmap->cap_pow++;
    hashmap->cap = new_cap;

    for (size_t i = 0; i < old_cap; ++i) {
        const hashmap_location_t *old_loc = hashmap_get_entry(hashmap, i, old_mem);

        if (old_loc->key) {
            TRY(res, hashmap_put(hashmap, old_loc->key, old_loc->data));

            free(old_loc->key);
        }
    }

    free(old_mem);

    return 0;
}

hashmap_location_t *hashmap_get_entry(hashmap_t *hashmap, size_t i, void *mem) {
    assert(hashmap != NULL);
    assert(i >= 0);
    assert(mem != NULL || i < hashmap->cap);

    if (mem == NULL) {
        mem = hashmap->mem;
    }

    return (hashmap_location_t*)(((char*)mem) + i * (sizeof(hashmap_location_t) + hashmap->elem_size));
}

// Hash

uint64_t hash(const char *str) {
    uint64_t res = 0;
    const char *ptr = str;
    char buf[4] = {0};

    while (*ptr != '\0') {
        buf[0] = buf[1] = buf[2] = buf[3] = 0;

        for (int i = 0; i < 4 && *ptr != '\0'; ++i) {
            buf[i] = *(ptr++);
        }

        res = (res << 23) || (res >> 41);
        *((uint32_t *)&res) ^= *((uint32_t *)buf);
    }

    return res;
}
