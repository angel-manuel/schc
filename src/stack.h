#ifndef SCHC_STACK_H_
#define SCHC_STACK_H_

#include "vector.h"

typedef struct stack_ {
    vector_t vector;
} stack_t;

int stack_init(stack_t *stack, size_t elem_size);
void stack_destroy(stack_t *stack);

int stack_push(stack_t *stack, void *elem);
const void *stack_peek(stack_t *stack);
int stack_pop(stack_t *stack, void *elem);

#endif/*SCHC_STACK_H_*/