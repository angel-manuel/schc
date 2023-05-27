#include "linalloc.h"

#include <assert.h>
#include <string.h>

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

void linalloc_allocator(linalloc_t *linalloc, allocator_t *allocator) {
    allocator->allocator_data = linalloc;
    allocator->alloc = (void *)linalloc_alloc;
    allocator->realloc = (void *)linalloc_realloc;
    allocator->free = free_noop;
}

void *linalloc_alloc(linalloc_t *linalloc, size_t size) {
    assert(linalloc != NULL);
    assert(size > 0);

    size += sizeof(size_t);

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

    // Save alloc size
    *((size_t *)ret) = size - sizeof(size_t);

    return ret + sizeof(size_t);
}

void *linalloc_realloc(linalloc_t *linalloc, void *ptr, size_t size) {
    assert(linalloc != NULL);
    assert(size > 0);

    if (ptr == NULL) {
        return linalloc_alloc(linalloc, size);
    }

    size += sizeof(size_t);

    size_t old_size = *((size_t *)ptr - 1);
    size_t new_size = size - sizeof(size_t);

    if (new_size <= old_size) {
        *((size_t *)ptr - 1) = new_size;

        return ptr;
    }

    void *new;
    TRYCR(new, linalloc_alloc(linalloc, size), NULL, NULL);

    // Save new size
    *((size_t *)new) = new_size;
    new += sizeof(size_t);

    // Copy old data

    memcpy(new, ptr, old_size);

    return new;
}

char *linalloc_stralloc(linalloc_t *linalloc, const char *src) {
    assert(linalloc != NULL);
    assert(src != NULL);

    size_t srclen = strlen(src);
    char *dst;
    TRYCR(dst, linalloc_alloc(linalloc, srclen + 1), NULL, NULL);

    if (dst == NULL) {
        return NULL;
    }

    memcpy(dst, src, srclen + 1);

    return dst;
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