#ifndef SCHC_CORE_CORE_H_
#define SCHC_CORE_CORE_H_

#include <stdint.h>
#include <stdio.h>

#include "env.h"
#include "type.h"

#include "ast.h"

typedef struct core_expr_ core_expr_t;

int core_from_ast(const ast_t *ast, env_t *env, core_expr_t *expr);
int core_print(const env_t *env, const core_expr_t *expr, FILE *fp);
void core_destroy(core_expr_t *expr);

typedef enum core_expr_form_ {
    CORE_NO_FORM = 0,
    CORE_VALUE,
    CORE_APPL,
    CORE_LAMBDA,
    CORE_LITERAL,
    CORE_COND,
} core_expr_form_t;

typedef struct core_value_ {
    env_id_t name;
} core_value_t;

typedef struct core_appl_ {
    core_expr_t *fn;
    core_expr_t *arg;
} core_appl_t;

typedef struct core_lambda {
    env_id_t name;
    core_type_t type;
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
    core_expr_form_t form;
    union {
        core_value_t value;
        core_appl_t appl;
        core_lambda_t lambda;
        core_literal_t literal;
        core_cond_t cond;
    };
};

#endif /*SCHC_CORE_CORE_H_*/
