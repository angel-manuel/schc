#ifndef SCHC_CODEGEN_H
#define SCHC_CODEGEN_H

#include <stdio.h>

#include "core.h"
#include "env.h"

typedef struct codegen_ctx {
    FILE *output;
    int temp_counter;
    int indent;
    const env_t *current_env;
} codegen_ctx_t;

// Initialize codegen context
int codegen_init(codegen_ctx_t *ctx, FILE *output);

// Emit the complete C program
int codegen_emit_program(codegen_ctx_t *ctx, const env_t *env);

#endif // SCHC_CODEGEN_H
