#ifndef SCHC_RUNTIME_H
#define SCHC_RUNTIME_H

#include <stdint.h>
#include <stddef.h>

typedef struct schc_val schc_val_t;

typedef enum schc_val_tag {
    SCHC_VAL_INT = 0,
    SCHC_VAL_STR,
    SCHC_VAL_CLOSURE,
    SCHC_VAL_CON,
} schc_val_tag_t;

typedef struct schc_closure {
    void *fn_ptr;
    int arity;
    int applied;
    schc_val_t **args;
} schc_closure_t;

typedef struct schc_con {
    int tag;
    int arity;
    schc_val_t **fields;
} schc_con_t;

struct schc_val {
    schc_val_tag_t tag;
    union {
        int64_t i64;
        const char *str;
        schc_closure_t closure;
        schc_con_t con;
    };
};

// Allocation
schc_val_t *schc_alloc_int(int64_t v);
schc_val_t *schc_alloc_str(const char *s);
schc_val_t *schc_alloc_closure(void *fn_ptr, int arity);
schc_val_t *schc_alloc_con(int tag, int arity);

// Application
schc_val_t *schc_apply(schc_val_t *fn, schc_val_t *arg);

// Intrinsics - Arithmetic
schc_val_t *schc_plus(schc_val_t *a, schc_val_t *b);
schc_val_t *schc_minus(schc_val_t *a, schc_val_t *b);
schc_val_t *schc_mult(schc_val_t *a, schc_val_t *b);
schc_val_t *schc_div(schc_val_t *a, schc_val_t *b);
schc_val_t *schc_neg(schc_val_t *a);

// Intrinsics - Comparison (return 1 or 0 as int)
schc_val_t *schc_lte(schc_val_t *a, schc_val_t *b);
schc_val_t *schc_gte(schc_val_t *a, schc_val_t *b);
schc_val_t *schc_eq(schc_val_t *a, schc_val_t *b);

// Intrinsics - IO
schc_val_t *schc_putStrLn(schc_val_t *s);
schc_val_t *schc_show(schc_val_t *v);

// Debug
void schc_print_val(schc_val_t *v);

#endif // SCHC_RUNTIME_H
