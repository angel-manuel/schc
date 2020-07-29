#include "allocator.h"

#include <assert.h>
#include <string.h>

allocator_t default_allocator = {
    .allocator_data = NULL,
    .alloc = malloc_null,
    .realloc = realloc_null,
    .free = free_null,
};

void *malloc_null(void *_alloc_data, size_t size) { return malloc(size); }
void *realloc_null(void *alloc_data, void *ptr, size_t size) {
    return realloc(ptr, size);
}
void free_null(void *_alloc_data, void *mem) { free(mem); }
void free_noop(void *_alloc_data, void *mem) { return; }

char *allocator_stralloc(allocator_t *allocator, const char *src) {
    assert(allocator != NULL);
    assert(src != NULL);

    size_t srclen = strlen(src);
    char *dst = (char *)ALLOCATOR_ALLOC(allocator, srclen + 1);

    if (dst == NULL) {
        return NULL;
    }

    memcpy(dst, src, srclen + 1);

    return dst;
}
