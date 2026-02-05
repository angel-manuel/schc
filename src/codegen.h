#ifndef SCHC_CODEGEN_H
#define SCHC_CODEGEN_H

#include <stdio.h>

#include "core.h"
#include "env.h"
#include "data/vector.h"

// Tracks an inline (anonymous) lambda
typedef struct anon_lambda {
    const core_expr_t *expr;
    int id;
} anon_lambda_t;

typedef struct codegen_ctx {
    FILE *output;
    int temp_counter;
    int indent;
    const env_t *current_env;
    vector_t anon_lambdas;  // vector of anon_lambda_t
    int anon_counter;
} codegen_ctx_t;

// Initialize codegen context
int codegen_init(codegen_ctx_t *ctx, FILE *output);

// Destroy codegen context
void codegen_destroy(codegen_ctx_t *ctx);

// Emit the complete C program
int codegen_emit_program(codegen_ctx_t *ctx, const env_t *env);

#endif // SCHC_CODEGEN_H
