#ifndef SCHC_ALLOCATOR_H_
#define SCHC_ALLOCATOR_H_

#include <stdlib.h>

typedef struct allocator_ {
    void *allocator_data;
    void *(*alloc)(void *alloc_data, size_t size);
    void *(*realloc)(void *alloc_data, void *ptr, size_t size);
    void (*free)(void *alloc_data, void *mem);
} allocator_t;

void *malloc_null(void *alloc_data, size_t size);
void *realloc_null(void *alloc_data, void *ptr, size_t size);
void free_null(void *alloc_data, void *mem);
void free_noop(void *alloc_data, void *mem);

char *allocator_stralloc(allocator_t *allocator, const char *src);

extern allocator_t default_allocator;

#define ALLOCATOR_ALLOC(allocator, size)                                       \
    ((allocator)->alloc)((allocator)->allocator_data, (size))
#define ALLOCATOR_STRALLOC(allocator, src)                                     \
    allocator_stralloc((allocator), (src))
#define ALLOCATOR_REALLOC(allocator, ptr, size)                                \
    ((allocator)->realloc)((allocator)->allocator_data, (ptr), (size))
#define ALLOCATOR_FREE(allocator, mem)                                         \
    ((allocator)->free)((allocator)->allocator_data, (mem))

#define ALLOC(size) ALLOCATOR_ALLOC(allocator, (size))
#define STRALLOC(str) ALLOCATOR_STRALLOC(allocator, (str))
#define REALLOC(ptr, size) ALLOCATOR_REALLOC(allocator, (ptr), (size))
#define FREE(mem) ALLOCATOR_FREE(allocator, (mem))

#endif /*SCHC_ALLOCATOR_H_*/