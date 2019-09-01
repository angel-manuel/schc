#include "coregen.h"

#include <assert.h>

#include "data/vector.h"
#include "util.h"

#define CGFAIL(fail)                                                           \
    fprintf(stderr, "Coregen FAIL(%s:%d): %s\n", __FILE__, __LINE__, (fail));
#define CGWARN(warn)                                                           \
    fprintf(stderr, "Coregen WARN(%s:%d): %s\n", __FILE__, __LINE__, (warn));

int coregen_from_module_ast(const ast_t *ast, env_t *env) {
    assert(ast != NULL);
    assert(env != NULL);

    return 0;
}

/*
int coregen_from_module_ast(const ast_t *ast, env_t *env) {
    assert(ast != NULL);
    assert(env != NULL);

    int res;

    if (ast->rule != AST_BODY) {
        CGFAIL("Not a module or named expression");
    }

    const ast_body_t *body = &ast->body;

    for (size_t i = 0; i < body->topdecls.len; ++i) {
        const ast_t *decl = (const ast_t *)vector_get_ref(&body->topdecls, i);
        core_expr_t new_expr;

        switch (decl->rule) {
        case AST_VAL_DECL: {
            const ast_val_decl_t *val_decl = &decl->val_decl;

            TRY(res, coregen_from_expr_ast(decl, &new_expr, env));

            break;
        }
        case AST_FN_DECL: {
            const ast_fn_decl_t *fn_decl = &decl->fn_decl;

            TRY(res, coregen_from_expr_ast(decl, &new_expr, env));

            break;
        }
        default:
            CGWARN("Unknown rule");
            ast_print(decl, stderr);
            break;
        }
    }

    return 0;
}

int coregen_from_expr_ast(const ast_t *ast, core_expr_t *expr, env_t *env) {
    assert(ast != NULL);
    assert(expr != NULL);
    assert(env != NULL);

    expr->form = CORE_NO_FORM;

    switch (ast->rule) {
    case AST_VAL_DECL: {
        const ast_val_decl_t *val_decl_ast = &ast->val_decl;

        val_decl_ast->name;
        break;
    }
    case AST_FN_DECL: {
        break;
    }
    case AST_LET: {
        ast_let_t *let_ast = &ast->let;

        for (size_t i = 0; i < let_ast->bindings.len; ++i) {
            core_expr_t decl_expr;
            const ast_t *binding_ast =
                (const ast_t *)vector_get_ref(&let_ast->bindings, i);

            coregen_from_expr_ast(binding_ast, &decl_expr, env);
        }

        break;
    }
    default:
        break;
    }

    return 0;
}
*/