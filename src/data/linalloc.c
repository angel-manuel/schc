#include "linalloc.h"

#include <assert.h>

#include "util.h"

#define LINALLOC_FIRST_SIZE 32678

int linalloc_new_block(linalloc_t *linalloc);

int linalloc_init(linalloc_t *linalloc) {
    assert(linalloc != NULL);

    int res;

    linalloc->next_block_size = LINALLOC_FIRST_SIZE;
    TRY(res, stack_init(&linalloc->blocks, sizeof(linalloc_block_t)));

    TRY(res, linalloc_new_block(linalloc));

    return res;
}

void linalloc_destroy(linalloc_t *linalloc) {
    assert(linalloc != NULL);

    for (size_t i = 0; i < linalloc->blocks.vector.len; ++i) {
        linalloc_block_t *block =
            (linalloc_block_t *)vector_get_ref(&linalloc->blocks.vector, i);

        free(block->mem);
    }

    stack_destroy(&linalloc->blocks);
}

void *linalloc_alloc(linalloc_t *linalloc, size_t size) {
    assert(linalloc != NULL);
    assert(size > 0);

    int res;

    linalloc_block_t *block;

    TRYCR(block, (linalloc_block_t *)stack_peek(&linalloc->blocks), NULL, NULL);

    if (block->used + size > block->size) {
        while (size > linalloc->next_block_size) {
            linalloc->next_block_size *= 2;
        }

        TRYCR(res, linalloc_new_block(linalloc), -1, NULL);
        TRYCR(block, (linalloc_block_t *)stack_peek(&linalloc->blocks), NULL,
              NULL);
    }

    void *ret = block->mem + block->used;
    block->used += size;

    return ret;
}

int linalloc_new_block(linalloc_t *linalloc) {
    assert(linalloc != NULL);

    int res;

    linalloc_block_t next;
    next.used = 0;
    next.size = linalloc->next_block_size;

    linalloc->next_block_size *= 2;

    TRYCR(next.mem, malloc(next.size), NULL, -1);

    TRY(res, stack_push(&linalloc->blocks, &next));

    return res;
}