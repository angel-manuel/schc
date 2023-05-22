#ifndef SCHC_CORE_CORE_H_
#define SCHC_CORE_CORE_H_

#include <stdint.h>
#include <stdio.h>

#include "type.h"

#include "ast.h"

typedef struct core_expr_ core_expr_t;

#include "env.h"

int core_print(const core_expr_t *expr, FILE *fp);
void core_destroy(core_expr_t *expr, allocator_t *allocator);

typedef enum core_expr_form_ {
    CORE_NO_FORM = 0,
    CORE_PLACEHOLDER,
    CORE_CONSTRUCTOR,
    CORE_INDIR,
    CORE_INTRINSIC,
    CORE_APPL,
    CORE_LAMBDA,
    CORE_LITERAL,
    CORE_COND,
} core_expr_form_t;

typedef struct core_constructor_ {
    const char *name;
} core_constructor_t;

typedef struct core_indir_ {
    core_expr_t *target;
} core_indir_t;

typedef struct core_intrinsic_ {
    const char *name;
} core_intrinsic_t;

typedef struct core_appl_ {
    core_expr_t *fn;
    core_expr_t *arg;
} core_appl_t;

typedef struct core_lambda {
    env_t args;
    core_expr_t *body;
} core_lambda_t;

typedef enum core_lit_type_ {
    CORE_LITERAL_I64,
} core_lit_type_t;

typedef struct core_literal_ {
    core_lit_type_t type;
    union {
        int64_t i64;
    };
} core_literal_t;

typedef struct core_cond_ {
    core_expr_t *cond;
    core_expr_t *then_branch;
    core_expr_t *else_branch;
} core_cond_t;

struct core_expr_ {
    const char *name;
    core_expr_form_t form;
    union {
        core_indir_t indir;
        core_constructor_t constructor;
        core_intrinsic_t intrinsic;
        core_appl_t appl;
        core_lambda_t lambda;
        core_literal_t literal;
        core_cond_t cond;
    };
};

#endif /*SCHC_CORE_CORE_H_*/
