#ifndef SCHC_DATA_STACK_H_
#define SCHC_DATA_STACK_H_

#include "allocator.h"
#include "vector.h"

typedef struct stack_ {
    vector_t vector;
} stack_t;

int stack_init(stack_t *stack, size_t elem_size);
int stack_init_with_allocator(stack_t *stack, size_t elem_size,
                              allocator_t *allocator);
void stack_destroy(stack_t *stack);

int stack_push(stack_t *stack, void *elem);
const void *stack_peek(stack_t *stack);
int stack_pop(stack_t *stack, void *elem);

#endif /*SCHC_DATA_STACK_H_*/