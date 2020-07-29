#ifndef SCHC_LINALLOC_H_
#define SCHC_LINALLOC_H_

#include <stdlib.h>

#include "allocator.h"
#include "stack.h"

typedef struct linalloc_block_ {
    size_t size;
    size_t used;
    void *mem;
} linalloc_block_t;

typedef struct linalloc_ {
    size_t next_block_size;
    stack_t /*linalloc_block_t*/ blocks;
} linalloc_t;

int linalloc_init(linalloc_t *linalloc);
void linalloc_destroy(linalloc_t *linalloc);

void linalloc_allocator(linalloc_t *linalloc, allocator_t *allocator);

void *linalloc_alloc(linalloc_t *linalloc, size_t size);
void *linalloc_realloc(linalloc_t *linalloc, void *ptr, size_t size);
char *linalloc_stralloc(linalloc_t *linalloc, const char *src);

#endif /*SCHC_LINALLOC_H_*/
