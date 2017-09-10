#include "stack.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

int stack_init(stack_t *stack, size_t elem_size) {
    assert(stack != NULL);

    return vector_init(&stack->vector, elem_size);
}

void stack_destroy(stack_t *stack) {
    assert(stack != NULL);

    vector_destroy(&stack->vector);
}

int stack_push(stack_t *stack, void *elem) {
    assert(stack != NULL);
    assert(elem != NULL);

    return vector_push_back(&stack->vector, elem) == NULL ? -1 : 0;
}

const void *stack_peek(stack_t *stack) {
    assert(stack != NULL);

    int len = stack->vector.len;

    if (len == 0) {
        return NULL;
    }

    return vector_get_ref(&stack->vector, len - 1);
}

int stack_pop(stack_t *stack, void *elem) {
    assert(stack != NULL);
    
    int len = stack->vector.len;

    if (len == 0) {
        return -1;
    }

    if (elem != NULL) {
        memcpy(elem, vector_get_ref(&stack->vector, len - 1), stack->vector.elem_size);
    }

    stack->vector.len--;

    return 0;
}