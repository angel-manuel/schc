#include "runtime.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Allocation

schc_val_t *schc_alloc_int(int64_t v) {
    schc_val_t *val = malloc(sizeof(schc_val_t));
    val->tag = SCHC_VAL_INT;
    val->i64 = v;
    return val;
}

schc_val_t *schc_alloc_str(const char *s) {
    schc_val_t *val = malloc(sizeof(schc_val_t));
    val->tag = SCHC_VAL_STR;
    val->str = s;  // Note: doesn't copy, assumes string is static or managed
    return val;
}

schc_val_t *schc_alloc_closure(void *fn_ptr, int arity) {
    schc_val_t *val = malloc(sizeof(schc_val_t));
    val->tag = SCHC_VAL_CLOSURE;
    val->closure.fn_ptr = fn_ptr;
    val->closure.arity = arity;
    val->closure.applied = 0;
    if (arity > 0) {
        val->closure.args = malloc(sizeof(schc_val_t *) * arity);
        memset(val->closure.args, 0, sizeof(schc_val_t *) * arity);
    } else {
        val->closure.args = NULL;
    }
    return val;
}

schc_val_t *schc_alloc_con(int tag, int arity) {
    schc_val_t *val = malloc(sizeof(schc_val_t));
    val->tag = SCHC_VAL_CON;
    val->con.tag = tag;
    val->con.arity = arity;
    if (arity > 0) {
        val->con.fields = malloc(sizeof(schc_val_t *) * arity);
        memset(val->con.fields, 0, sizeof(schc_val_t *) * arity);
    } else {
        val->con.fields = NULL;
    }
    return val;
}

// Copy a closure with its captured arguments
static schc_val_t *schc_copy_closure(schc_val_t *orig) {
    schc_val_t *val = malloc(sizeof(schc_val_t));
    val->tag = SCHC_VAL_CLOSURE;
    val->closure.fn_ptr = orig->closure.fn_ptr;
    val->closure.arity = orig->closure.arity;
    val->closure.applied = orig->closure.applied;
    if (orig->closure.arity > 0) {
        val->closure.args = malloc(sizeof(schc_val_t *) * orig->closure.arity);
        memcpy(val->closure.args, orig->closure.args,
               sizeof(schc_val_t *) * orig->closure.arity);
    } else {
        val->closure.args = NULL;
    }
    return val;
}

// Application

schc_val_t *schc_apply(schc_val_t *fn, schc_val_t *arg) {
    if (fn->tag != SCHC_VAL_CLOSURE) {
        fprintf(stderr, "Runtime error: applying non-closure\n");
        exit(1);
    }

    schc_closure_t *clos = &fn->closure;

    if (clos->applied + 1 < clos->arity) {
        // Partial application: copy closure and add argument
        schc_val_t *new_clos = schc_copy_closure(fn);
        new_clos->closure.args[new_clos->closure.applied] = arg;
        new_clos->closure.applied++;
        return new_clos;
    }

    // Full application: call the function
    // Build argument array with all captured args plus new arg
    int total_args = clos->arity;
    schc_val_t **all_args = malloc(sizeof(schc_val_t *) * total_args);
    for (int i = 0; i < clos->applied; i++) {
        all_args[i] = clos->args[i];
    }
    all_args[clos->applied] = arg;

    schc_val_t *result;

    // Call based on arity
    switch (total_args) {
    case 1: {
        typedef schc_val_t *(*fn1_t)(schc_val_t *);
        fn1_t f = (fn1_t)clos->fn_ptr;
        result = f(all_args[0]);
        break;
    }
    case 2: {
        typedef schc_val_t *(*fn2_t)(schc_val_t *, schc_val_t *);
        fn2_t f = (fn2_t)clos->fn_ptr;
        result = f(all_args[0], all_args[1]);
        break;
    }
    case 3: {
        typedef schc_val_t *(*fn3_t)(schc_val_t *, schc_val_t *, schc_val_t *);
        fn3_t f = (fn3_t)clos->fn_ptr;
        result = f(all_args[0], all_args[1], all_args[2]);
        break;
    }
    case 4: {
        typedef schc_val_t *(*fn4_t)(schc_val_t *, schc_val_t *, schc_val_t *,
                                     schc_val_t *);
        fn4_t f = (fn4_t)clos->fn_ptr;
        result = f(all_args[0], all_args[1], all_args[2], all_args[3]);
        break;
    }
    case 5: {
        typedef schc_val_t *(*fn5_t)(schc_val_t *, schc_val_t *, schc_val_t *,
                                     schc_val_t *, schc_val_t *);
        fn5_t f = (fn5_t)clos->fn_ptr;
        result = f(all_args[0], all_args[1], all_args[2], all_args[3],
                   all_args[4]);
        break;
    }
    default:
        fprintf(stderr, "Runtime error: arity %d not supported\n", total_args);
        exit(1);
    }

    free(all_args);
    return result;
}

// Intrinsics - Arithmetic

schc_val_t *schc_plus(schc_val_t *a, schc_val_t *b) {
    if (a->tag != SCHC_VAL_INT || b->tag != SCHC_VAL_INT) {
        fprintf(stderr, "Runtime error: plus on non-integers\n");
        exit(1);
    }
    return schc_alloc_int(a->i64 + b->i64);
}

schc_val_t *schc_minus(schc_val_t *a, schc_val_t *b) {
    if (a->tag != SCHC_VAL_INT || b->tag != SCHC_VAL_INT) {
        fprintf(stderr, "Runtime error: minus on non-integers\n");
        exit(1);
    }
    return schc_alloc_int(a->i64 - b->i64);
}

schc_val_t *schc_mult(schc_val_t *a, schc_val_t *b) {
    if (a->tag != SCHC_VAL_INT || b->tag != SCHC_VAL_INT) {
        fprintf(stderr, "Runtime error: mult on non-integers\n");
        exit(1);
    }
    return schc_alloc_int(a->i64 * b->i64);
}

schc_val_t *schc_div(schc_val_t *a, schc_val_t *b) {
    if (a->tag != SCHC_VAL_INT || b->tag != SCHC_VAL_INT) {
        fprintf(stderr, "Runtime error: div on non-integers\n");
        exit(1);
    }
    if (b->i64 == 0) {
        fprintf(stderr, "Runtime error: division by zero\n");
        exit(1);
    }
    return schc_alloc_int(a->i64 / b->i64);
}

schc_val_t *schc_neg(schc_val_t *a) {
    if (a->tag != SCHC_VAL_INT) {
        fprintf(stderr, "Runtime error: neg on non-integer\n");
        exit(1);
    }
    return schc_alloc_int(-a->i64);
}

// Intrinsics - Comparison

schc_val_t *schc_lte(schc_val_t *a, schc_val_t *b) {
    if (a->tag != SCHC_VAL_INT || b->tag != SCHC_VAL_INT) {
        fprintf(stderr, "Runtime error: lte on non-integers\n");
        exit(1);
    }
    return schc_alloc_int(a->i64 <= b->i64 ? 1 : 0);
}

schc_val_t *schc_gte(schc_val_t *a, schc_val_t *b) {
    if (a->tag != SCHC_VAL_INT || b->tag != SCHC_VAL_INT) {
        fprintf(stderr, "Runtime error: gte on non-integers\n");
        exit(1);
    }
    return schc_alloc_int(a->i64 >= b->i64 ? 1 : 0);
}

schc_val_t *schc_eq(schc_val_t *a, schc_val_t *b) {
    if (a->tag != SCHC_VAL_INT || b->tag != SCHC_VAL_INT) {
        fprintf(stderr, "Runtime error: eq on non-integers\n");
        exit(1);
    }
    return schc_alloc_int(a->i64 == b->i64 ? 1 : 0);
}

// Intrinsics - IO

schc_val_t *schc_putStrLn(schc_val_t *s) {
    if (s->tag != SCHC_VAL_STR) {
        fprintf(stderr, "Runtime error: putStrLn on non-string\n");
        exit(1);
    }
    puts(s->str);
    // Return unit (represented as 0 for now)
    return schc_alloc_int(0);
}

schc_val_t *schc_show(schc_val_t *v) {
    char buf[64];
    switch (v->tag) {
    case SCHC_VAL_INT:
        snprintf(buf, sizeof(buf), "%ld", v->i64);
        // Need to allocate a copy since buf is on stack
        {
            char *copy = malloc(strlen(buf) + 1);
            strcpy(copy, buf);
            return schc_alloc_str(copy);
        }
    case SCHC_VAL_STR:
        return v;  // Strings show as themselves
    default:
        return schc_alloc_str("<value>");
    }
}

// Debug

void schc_print_val(schc_val_t *v) {
    switch (v->tag) {
    case SCHC_VAL_INT:
        printf("%ld", v->i64);
        break;
    case SCHC_VAL_STR:
        printf("%s", v->str);
        break;
    case SCHC_VAL_CLOSURE:
        printf("<closure arity=%d applied=%d>", v->closure.arity,
               v->closure.applied);
        break;
    case SCHC_VAL_CON:
        printf("<constructor tag=%d arity=%d>", v->con.tag, v->con.arity);
        break;
    }
}
