#include "codegen.h"

#include <string.h>
#include <ctype.h>

#include "data/vector.h"

// Utility to sanitize names for C identifiers
static void emit_sanitized_name(FILE *out, const char *name) {
    for (const char *p = name; *p; p++) {
        if (isalnum(*p) || *p == '_') {
            fputc(*p, out);
        } else {
            fprintf(out, "_%02x", (unsigned char)*p);
        }
    }
}

static void emit_indent(codegen_ctx_t *ctx) {
    for (int i = 0; i < ctx->indent; i++) {
        fprintf(ctx->output, "    ");
    }
}

// Forward declarations
static int codegen_emit_expr(codegen_ctx_t *ctx, const core_expr_t *expr,
                             const char *result_var);
static int codegen_count_lambda_args(const core_expr_t *expr);

int codegen_init(codegen_ctx_t *ctx, FILE *output) {
    ctx->output = output;
    ctx->temp_counter = 0;
    ctx->indent = 0;
    ctx->current_env = NULL;
    ctx->anon_counter = 0;
    vector_init(&ctx->anon_lambdas, sizeof(anon_lambda_t));
    return 0;
}

void codegen_destroy(codegen_ctx_t *ctx) {
    vector_destroy(&ctx->anon_lambdas);
}

// Get the runtime function name for an intrinsic
static const char *intrinsic_to_runtime(const char *name) {
    if (strcmp(name, "plus") == 0) return "schc_plus";
    if (strcmp(name, "minus") == 0) return "schc_minus";
    if (strcmp(name, "mult") == 0) return "schc_mult";
    if (strcmp(name, "div") == 0) return "schc_div";
    if (strcmp(name, "neg") == 0) return "schc_neg";
    if (strcmp(name, "gte") == 0) return "schc_gte";
    if (strcmp(name, "lte") == 0) return "schc_lte";
    if (strcmp(name, "eq") == 0) return "schc_eq";
    if (strcmp(name, "putStrLn") == 0) return "schc_putStrLn";
    if (strcmp(name, "show") == 0) return "schc_show";
    return NULL;
}

// Get the arity of an intrinsic
static int intrinsic_arity(const char *name) {
    if (strcmp(name, "neg") == 0) return 1;
    if (strcmp(name, "putStrLn") == 0) return 1;
    if (strcmp(name, "show") == 0) return 1;
    // All others are binary
    return 2;
}

// Check if expression is a fully saturated intrinsic call
// Returns: 0 = not intrinsic, 1 = saturated intrinsic call, -1 = partial intrinsic
static int is_saturated_intrinsic(const core_expr_t *expr,
                                  const core_expr_t **out_intrinsic,
                                  const core_expr_t **out_args, int *out_argc) {
    if (expr->form != CORE_APPL) return 0;

    // Collect all arguments
    const core_expr_t *args[16];
    int argc = 0;
    const core_expr_t *fn = expr;

    while (fn->form == CORE_APPL && argc < 16) {
        args[argc++] = fn->appl.arg;
        fn = fn->appl.fn;
    }

    // Check if base is an intrinsic
    if (fn->form == CORE_INTRINSIC) {
        int arity = intrinsic_arity(fn->intrinsic.name);
        if (argc == arity) {
            *out_intrinsic = fn;
            // Reverse args (they're in reverse order)
            for (int i = 0; i < argc; i++) {
                out_args[i] = args[argc - 1 - i];
            }
            *out_argc = argc;
            return 1;
        }
    }

    // Check if base is an indirection to an intrinsic
    if (fn->form == CORE_INDIR && fn->indir.target &&
        fn->indir.target->form == CORE_INTRINSIC) {
        int arity = intrinsic_arity(fn->indir.target->intrinsic.name);
        if (argc == arity) {
            *out_intrinsic = fn->indir.target;
            for (int i = 0; i < argc; i++) {
                out_args[i] = args[argc - 1 - i];
            }
            *out_argc = argc;
            return 1;
        }
    }

    return 0;
}

// Recursively collect all inline lambdas from an expression
static void collect_inline_lambdas(codegen_ctx_t *ctx, const core_expr_t *expr) {
    if (!expr) return;

    switch (expr->form) {
    case CORE_LAMBDA: {
        // This is an inline lambda - add it to our collection
        anon_lambda_t anon;
        anon.expr = expr;
        anon.id = ctx->anon_counter++;
        vector_push_back(&ctx->anon_lambdas, &anon);
        // Also collect from the lambda body
        collect_inline_lambdas(ctx, expr->lambda.body);
        break;
    }
    case CORE_APPL:
        collect_inline_lambdas(ctx, expr->appl.fn);
        collect_inline_lambdas(ctx, expr->appl.arg);
        break;
    case CORE_COND:
        collect_inline_lambdas(ctx, expr->cond.cond);
        collect_inline_lambdas(ctx, expr->cond.then_branch);
        collect_inline_lambdas(ctx, expr->cond.else_branch);
        break;
    case CORE_INDIR:
        // Don't recurse into indirections to avoid infinite loops
        break;
    default:
        break;
    }
}

// Find the anonymous lambda ID for a given lambda expression
static int find_anon_lambda_id(codegen_ctx_t *ctx, const core_expr_t *expr) {
    for (size_t i = 0; i < ctx->anon_lambdas.len; i++) {
        anon_lambda_t *anon = (anon_lambda_t *)vector_get_ref(&ctx->anon_lambdas, i);
        if (anon->expr == expr) {
            return anon->id;
        }
    }
    return -1;
}

// Emit code to evaluate an expression into a temporary variable
static int codegen_emit_expr(codegen_ctx_t *ctx, const core_expr_t *expr,
                             const char *result_var) {
    FILE *out = ctx->output;

    switch (expr->form) {
    case CORE_LITERAL: {
        emit_indent(ctx);
        switch (expr->literal.type) {
        case CORE_LITERAL_I64:
            fprintf(out, "schc_val_t *%s = schc_alloc_int(%ldLL);\n", result_var,
                    expr->literal.i64);
            break;
        case CORE_LITERAL_STR:
            fprintf(out, "schc_val_t *%s = schc_alloc_str(\"", result_var);
            // Escape the string
            for (const char *p = expr->literal.str; *p; p++) {
                switch (*p) {
                case '\n': fprintf(out, "\\n"); break;
                case '\r': fprintf(out, "\\r"); break;
                case '\t': fprintf(out, "\\t"); break;
                case '\\': fprintf(out, "\\\\"); break;
                case '"': fprintf(out, "\\\""); break;
                default: fputc(*p, out);
                }
            }
            fprintf(out, "\");\n");
            break;
        }
        break;
    }

    case CORE_INDIR: {
        // Reference to another expression
        if (expr->indir.target == NULL) {
            // This is a placeholder (lambda argument)
            emit_indent(ctx);
            fprintf(out, "schc_val_t *%s = schc_", result_var);
            emit_sanitized_name(out, expr->name);
            fprintf(out, ";\n");
        } else if (expr->indir.target->form == CORE_LAMBDA) {
            // Reference to a function - get its closure
            emit_indent(ctx);
            fprintf(out, "schc_val_t *%s = schc_fn_", result_var);
            emit_sanitized_name(out, expr->indir.target->name);
            fprintf(out, ";\n");
        } else if (expr->indir.target->form == CORE_INTRINSIC) {
            // Reference to intrinsic - wrap in closure
            const char *iname = expr->indir.target->intrinsic.name;
            const char *runtime_fn = intrinsic_to_runtime(iname);
            int arity = intrinsic_arity(iname);
            emit_indent(ctx);
            fprintf(out, "schc_val_t *%s = schc_alloc_closure(%s, %d);\n",
                    result_var, runtime_fn, arity);
        } else {
            // Evaluate the target expression
            return codegen_emit_expr(ctx, expr->indir.target, result_var);
        }
        break;
    }

    case CORE_INTRINSIC: {
        // Intrinsic as value - wrap in closure
        const char *runtime_fn = intrinsic_to_runtime(expr->intrinsic.name);
        int arity = intrinsic_arity(expr->intrinsic.name);
        if (runtime_fn) {
            emit_indent(ctx);
            fprintf(out, "schc_val_t *%s = schc_alloc_closure((void*)%s, %d);\n",
                    result_var, runtime_fn, arity);
        } else {
            emit_indent(ctx);
            fprintf(out, "// Unsupported intrinsic: %s\n", expr->intrinsic.name);
            fprintf(out, "schc_val_t *%s = NULL;\n", result_var);
        }
        break;
    }

    case CORE_APPL: {
        // Check for saturated intrinsic call
        const core_expr_t *intrinsic;
        const core_expr_t *args[16];
        int argc;

        if (is_saturated_intrinsic(expr, &intrinsic, args, &argc)) {
            // Direct intrinsic call
            const char *runtime_fn = intrinsic_to_runtime(intrinsic->intrinsic.name);
            char arg_vars[16][32];

            for (int i = 0; i < argc; i++) {
                snprintf(arg_vars[i], 32, "_t%d", ctx->temp_counter++);
                codegen_emit_expr(ctx, args[i], arg_vars[i]);
            }

            emit_indent(ctx);
            fprintf(out, "schc_val_t *%s = %s(", result_var, runtime_fn);
            for (int i = 0; i < argc; i++) {
                if (i > 0) fprintf(out, ", ");
                fprintf(out, "%s", arg_vars[i]);
            }
            fprintf(out, ");\n");
        } else {
            // General application
            char fn_var[32], arg_var[32];
            snprintf(fn_var, 32, "_t%d", ctx->temp_counter++);
            snprintf(arg_var, 32, "_t%d", ctx->temp_counter++);

            codegen_emit_expr(ctx, expr->appl.fn, fn_var);
            codegen_emit_expr(ctx, expr->appl.arg, arg_var);

            emit_indent(ctx);
            fprintf(out, "schc_val_t *%s = schc_apply(%s, %s);\n", result_var,
                    fn_var, arg_var);
        }
        break;
    }

    case CORE_COND: {
        char cond_var[32];
        snprintf(cond_var, 32, "_t%d", ctx->temp_counter++);
        codegen_emit_expr(ctx, expr->cond.cond, cond_var);

        emit_indent(ctx);
        fprintf(out, "schc_val_t *%s;\n", result_var);
        emit_indent(ctx);
        fprintf(out, "if (%s->i64) {\n", cond_var);

        ctx->indent++;
        char then_var[32];
        snprintf(then_var, 32, "_t%d", ctx->temp_counter++);
        codegen_emit_expr(ctx, expr->cond.then_branch, then_var);
        emit_indent(ctx);
        fprintf(out, "%s = %s;\n", result_var, then_var);
        ctx->indent--;

        emit_indent(ctx);
        fprintf(out, "} else {\n");

        ctx->indent++;
        char else_var[32];
        snprintf(else_var, 32, "_t%d", ctx->temp_counter++);
        codegen_emit_expr(ctx, expr->cond.else_branch, else_var);
        emit_indent(ctx);
        fprintf(out, "%s = %s;\n", result_var, else_var);
        ctx->indent--;

        emit_indent(ctx);
        fprintf(out, "}\n");
        break;
    }

    case CORE_LAMBDA: {
        // Inline lambda - reference the pre-collected anonymous function
        int anon_id = find_anon_lambda_id(ctx, expr);
        if (anon_id >= 0) {
            int arity = codegen_count_lambda_args(expr);
            emit_indent(ctx);
            fprintf(out, "schc_val_t *%s = schc_alloc_closure((void*)schc_anon_%d_impl, %d);\n",
                    result_var, anon_id, arity);
        } else {
            emit_indent(ctx);
            fprintf(out, "// ERROR: inline lambda not found\n");
            fprintf(out, "schc_val_t *%s = NULL;\n", result_var);
        }
        break;
    }

    case CORE_PLACEHOLDER: {
        // Reference to a lambda argument
        emit_indent(ctx);
        fprintf(out, "schc_val_t *%s = schc_", result_var);
        emit_sanitized_name(out, expr->name);
        fprintf(out, ";\n");
        break;
    }

    default:
        emit_indent(ctx);
        fprintf(out, "// Unknown expression form: %d\n", expr->form);
        fprintf(out, "schc_val_t *%s = NULL;\n", result_var);
        break;
    }

    return 0;
}

// Count lambda arguments
static int codegen_count_lambda_args(const core_expr_t *expr) {
    if (expr->form != CORE_LAMBDA) return 0;

    vector_t keys;
    vector_init(&keys, sizeof(const char *));
    env_list_scope(&expr->lambda.args, &keys, 0);
    int count = (int)keys.len;
    vector_destroy(&keys);
    return count;
}

// Emit a function implementation for a lambda
static int codegen_emit_function(codegen_ctx_t *ctx, const char *name,
                                 const core_expr_t *expr) {
    if (expr->form != CORE_LAMBDA) return -1;

    FILE *out = ctx->output;

    // Get argument names
    vector_t arg_names;
    vector_init(&arg_names, sizeof(const char *));
    env_list_scope(&expr->lambda.args, &arg_names, 0);

    // Emit function signature
    fprintf(out, "schc_val_t *schc_fn_");
    emit_sanitized_name(out, name);
    fprintf(out, "_impl(");

    for (size_t i = 0; i < arg_names.len; i++) {
        if (i > 0) fprintf(out, ", ");
        const char *arg_name = *(const char **)vector_get_ref(&arg_names, i);
        fprintf(out, "schc_val_t *schc_");
        emit_sanitized_name(out, arg_name);
    }
    fprintf(out, ") {\n");

    // Emit body
    ctx->indent = 1;
    char result_var[32];
    snprintf(result_var, 32, "_result");
    codegen_emit_expr(ctx, expr->lambda.body, result_var);

    emit_indent(ctx);
    fprintf(out, "return %s;\n", result_var);
    fprintf(out, "}\n\n");

    vector_destroy(&arg_names);
    return 0;
}

// Emit implementation for an anonymous lambda
static int codegen_emit_anon_lambda(codegen_ctx_t *ctx, int anon_id,
                                    const core_expr_t *expr) {
    if (expr->form != CORE_LAMBDA) return -1;

    FILE *out = ctx->output;

    // Get argument names
    vector_t arg_names;
    vector_init(&arg_names, sizeof(const char *));
    env_list_scope(&expr->lambda.args, &arg_names, 0);

    // Emit function signature
    fprintf(out, "schc_val_t *schc_anon_%d_impl(", anon_id);

    for (size_t i = 0; i < arg_names.len; i++) {
        if (i > 0) fprintf(out, ", ");
        const char *arg_name = *(const char **)vector_get_ref(&arg_names, i);
        fprintf(out, "schc_val_t *schc_");
        emit_sanitized_name(out, arg_name);
    }
    fprintf(out, ") {\n");

    // Emit body
    ctx->indent = 1;
    char result_var[32];
    snprintf(result_var, 32, "_result");
    codegen_emit_expr(ctx, expr->lambda.body, result_var);

    emit_indent(ctx);
    fprintf(out, "return %s;\n", result_var);
    fprintf(out, "}\n\n");

    vector_destroy(&arg_names);
    return 0;
}

// Emit the complete program
int codegen_emit_program(codegen_ctx_t *ctx, const env_t *env) {
    FILE *out = ctx->output;
    ctx->current_env = env;

    // Get all top-level definitions
    vector_t names;
    vector_init(&names, sizeof(const char *));
    env_list_scope(env, &names, 0);

    // Pre-pass: collect all inline lambdas from all expressions
    for (size_t i = 0; i < names.len; i++) {
        const char *name = *(const char **)vector_get_ref(&names, i);
        const core_expr_t *expr = env_get_expr((env_t *)env, name);
        if (expr) {
            collect_inline_lambdas(ctx, expr);
        }
    }

    // Header
    fprintf(out, "// Generated by SCHC\n");
    fprintf(out, "#include \"runtime/runtime.h\"\n");
    fprintf(out, "#include <stdio.h>\n\n");

    // Forward declarations for all named functions
    fprintf(out, "// Forward declarations\n");
    for (size_t i = 0; i < names.len; i++) {
        const char *name = *(const char **)vector_get_ref(&names, i);
        const core_expr_t *expr = env_get_expr((env_t *)env, name);

        if (expr && expr->form == CORE_LAMBDA) {
            int arity = codegen_count_lambda_args(expr);
            fprintf(out, "schc_val_t *schc_fn_");
            emit_sanitized_name(out, name);
            fprintf(out, "_impl(");
            for (int j = 0; j < arity; j++) {
                if (j > 0) fprintf(out, ", ");
                fprintf(out, "schc_val_t *");
            }
            fprintf(out, ");\n");
        }
    }

    // Forward declarations for anonymous lambdas
    for (size_t i = 0; i < ctx->anon_lambdas.len; i++) {
        anon_lambda_t *anon = (anon_lambda_t *)vector_get_ref(&ctx->anon_lambdas, i);
        int arity = codegen_count_lambda_args(anon->expr);
        fprintf(out, "schc_val_t *schc_anon_%d_impl(", anon->id);
        for (int j = 0; j < arity; j++) {
            if (j > 0) fprintf(out, ", ");
            fprintf(out, "schc_val_t *");
        }
        fprintf(out, ");\n");
    }
    fprintf(out, "\n");

    // Global closure variables
    fprintf(out, "// Global closures\n");
    for (size_t i = 0; i < names.len; i++) {
        const char *name = *(const char **)vector_get_ref(&names, i);
        const core_expr_t *expr = env_get_expr((env_t *)env, name);

        if (expr && expr->form == CORE_LAMBDA) {
            fprintf(out, "schc_val_t *schc_fn_");
            emit_sanitized_name(out, name);
            fprintf(out, ";\n");
        }
    }
    fprintf(out, "\n");

    // Function implementations
    fprintf(out, "// Function implementations\n");
    for (size_t i = 0; i < names.len; i++) {
        const char *name = *(const char **)vector_get_ref(&names, i);
        const core_expr_t *expr = env_get_expr((env_t *)env, name);

        if (expr && expr->form == CORE_LAMBDA) {
            codegen_emit_function(ctx, name, expr);
        }
    }

    // Anonymous lambda implementations
    fprintf(out, "// Anonymous lambda implementations\n");
    for (size_t i = 0; i < ctx->anon_lambdas.len; i++) {
        anon_lambda_t *anon = (anon_lambda_t *)vector_get_ref(&ctx->anon_lambdas, i);
        codegen_emit_anon_lambda(ctx, anon->id, anon->expr);
    }

    // Initialization function
    fprintf(out, "// Initialize closures\n");
    fprintf(out, "void schc_init(void) {\n");
    for (size_t i = 0; i < names.len; i++) {
        const char *name = *(const char **)vector_get_ref(&names, i);
        const core_expr_t *expr = env_get_expr((env_t *)env, name);

        if (expr && expr->form == CORE_LAMBDA) {
            int arity = codegen_count_lambda_args(expr);
            fprintf(out, "    schc_fn_");
            emit_sanitized_name(out, name);
            fprintf(out, " = schc_alloc_closure((void*)schc_fn_");
            emit_sanitized_name(out, name);
            fprintf(out, "_impl, %d);\n", arity);
        }
    }
    fprintf(out, "}\n\n");

    // Main function
    fprintf(out, "// Main\n");
    fprintf(out, "int main(void) {\n");
    fprintf(out, "    schc_init();\n");

    // Find and evaluate 'main'
    const core_expr_t *main_expr = env_get_expr((env_t *)env, "main");
    if (main_expr) {
        ctx->indent = 1;
        codegen_emit_expr(ctx, main_expr, "_main_result");
        // Print result unless it's unit (int 0 from IO actions)
        fprintf(out, "    if (!(_main_result->tag == SCHC_VAL_INT && _main_result->i64 == 0)) {\n");
        fprintf(out, "        schc_print_val(_main_result);\n");
        fprintf(out, "        printf(\"\\n\");\n");
        fprintf(out, "    }\n");
    } else {
        fprintf(out, "    printf(\"No main defined\\n\");\n");
    }

    fprintf(out, "    return 0;\n");
    fprintf(out, "}\n");

    vector_destroy(&names);
    return 0;
}
