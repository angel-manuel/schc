#include "coregen.h"

#include <assert.h>
#include <string.h>

#include "data/vector.h"
#include "util.h"

#define CGFAIL(fmt, ...)                                                       \
    fprintf(stderr, "Coregen FAIL(%s:%d): " fmt "\n", __FILE__, __LINE__,      \
            ##__VA_ARGS__);
#define CGWARN(fmt, ...)                                                       \
    fprintf(stderr, "Coregen WARN(%s:%d): " fmt "\n", __FILE__, __LINE__,      \
            ##__VA_ARGS__);

int coregen_populate_env(const vector_t /* core_ast_t */ *decls, env_t *env,
                         vector_t /* core_expr_t */ *expr_heap);
int coregen_generate_env(const vector_t /* core_ast_t */ *decls, env_t *env,
                         vector_t /* core_expr_t */ *expr_heap);
int coregen_from_ast(const ast_t *ast, env_t *env, core_expr_t *expr,
                     vector_t /* core_expr_t */ *expr_heap);

int coregen_from_module_ast(const ast_t *ast, env_t *env,
                            vector_t /* core_expr_t */ *expr_heap) {
    assert(ast != NULL);
    assert(env != NULL);
    assert(expr_heap != NULL);

    int res;

    if (ast->rule == AST_MODULE) {
        assert(ast->module.body->rule == AST_BODY);

        const vector_t *decls = &ast->module.body->body.topdecls;

        TRY(res, coregen_populate_env(decls, env, expr_heap));
        TRY(res, coregen_generate_env(decls, env, expr_heap));
    } else {
        CGFAIL("Not a module");
        return -1;
    }

    return 0;
}

int coregen_populate_env(const vector_t /* core_ast_t */ *decls, env_t *env,
                         vector_t /* core_expr_t */ *expr_heap) {
    assert(decls != NULL);
    assert(env != NULL);
    assert(expr_heap != NULL);

    int res;

    core_expr_t empty_tmpl;

    empty_tmpl.form = CORE_NO_FORM;
    empty_tmpl.name = NULL;

    for (size_t i = 0; i < decls->len; ++i) {
        const ast_t *decl = (const ast_t *)vector_get_ref(decls, i);

        switch (decl->rule) {
        case AST_VAL_DECL: {
            const ast_val_decl_t *val_decl = &decl->val_decl;

            core_expr_t *new_expr;
            TRYCR(new_expr, vector_push_back(expr_heap, &empty_tmpl), NULL, -1);

            printf("PUT %s\n", val_decl->name);
            new_expr->name = val_decl->name;

            TRY(res, env_put_expr(env, val_decl->name, new_expr));

            break;
        }
        case AST_FN_DECL: {
            const ast_fn_decl_t *fn_decl = &decl->fn_decl;

            core_expr_t *new_expr;
            TRYCR(new_expr, vector_push_back(expr_heap, &empty_tmpl), NULL, -1);

            new_expr->name = fn_decl->name;

            TRY(res, env_put_expr(env, fn_decl->name, new_expr));

            break;
        }
        case AST_HAS_TYPE_DECL:
        case AST_CLASS_DECL:
        case AST_DATA_DECL:
        case AST_DEFAULT_DECL:
        case AST_FIXITY_DECL:
        case AST_FOREING_DECL:
        case AST_INSTANCE_DECL:
        case AST_NEWTYPE_DECL:
        case AST_TYPE_DECL:
            CGWARN("Not implemented");
            ast_print(decl, stderr);
            break;
        default:
            CGWARN("Unknown rule");
            ast_print(decl, stderr);
            break;
        }
    }

    return 0;
}

int coregen_generate_env(const vector_t /* core_ast_t */ *decls, env_t *env,
                         vector_t /* core_expr_t */ *expr_heap) {
    assert(decls != NULL);
    assert(env != NULL);
    assert(expr_heap != NULL);

    int res;

    for (size_t i = 0; i < decls->len; ++i) {
        const ast_t *decl = (const ast_t *)vector_get_ref(decls, i);

        switch (decl->rule) {
        case AST_VAL_DECL: {
            const ast_val_decl_t *val_decl = &decl->val_decl;
            core_expr_t *expr;

            TRYCR(expr, env_get_expr(env, val_decl->name), NULL, -1);

            TRY(res, coregen_from_ast(val_decl->body, env, expr, expr_heap));

            break;
        }
        case AST_FN_DECL: {
            // const ast_fn_decl_t *fn_decl = &decl->fn_decl;
            // core_expr_t *expr;

            // printf("fn %s\n", fn_decl->name);

            // TRYCR(expr, env_get_expr(env, fn_decl->name), NULL, -1);

            // TODO: Convert to lambda

            break;
        }
        case AST_HAS_TYPE_DECL:
        case AST_CLASS_DECL:
        case AST_DATA_DECL:
        case AST_DEFAULT_DECL:
        case AST_FIXITY_DECL:
        case AST_FOREING_DECL:
        case AST_INSTANCE_DECL:
        case AST_NEWTYPE_DECL:
        case AST_TYPE_DECL:
            CGWARN("Not implemented");
            ast_print(decl, stderr);
            break;
        default:
            CGWARN("Unknown rule");
            ast_print(decl, stderr);
            break;
        }
    }

    return 0;
}

int coregen_from_ast(const ast_t *ast, env_t *env, core_expr_t *expr,
                     vector_t /* core_expr_t */ *expr_heap) {
    assert(ast != NULL);
    assert(env != NULL);
    assert(expr != NULL);
    assert(expr_heap != NULL);

    int res;

    expr->form = CORE_NO_FORM;

    switch (ast->rule) {
    case AST_NEG: {
        const ast_neg_t *neg_ast = &ast->neg;

        expr->form = CORE_APPL;
        core_appl_t *appl = &expr->appl;

        // TODO: Prealloc intrisics
        TRYCR(appl->fn, vector_alloc_elem(expr_heap), NULL, -1);
        appl->fn->name = NULL;
        appl->fn->form = CORE_INTRINSIC;
        appl->fn->intrinsic.name = "neg";

        TRYCR(appl->arg, vector_alloc_elem(expr_heap), NULL, -1);
        appl->arg->name = NULL;
        TRY(res, coregen_from_ast(neg_ast->expr, env, appl->arg, expr_heap));

        break;
    }
    case AST_FN_APPL: {
        const ast_fn_appl_t *fn_appl_ast = &ast->fn_appl;

        expr->form = CORE_APPL;
        core_appl_t *appl = &expr->appl;

        TRYCR(appl->fn, vector_alloc_elem(expr_heap), NULL, -1);
        appl->fn->name = NULL;
        TRY(res, coregen_from_ast(fn_appl_ast->fn, env, appl->fn, expr_heap));

        TRYCR(appl->arg, vector_alloc_elem(expr_heap), NULL, -1);
        appl->arg->name = NULL;
        TRY(res, coregen_from_ast(fn_appl_ast->arg, env, appl->arg, expr_heap));

        break;
    }
    case AST_OP_APPL: {
        const ast_op_appl_t *op_appl_ast = &ast->op_appl;

        core_expr_t *op_expr = env_get_expr(env, op_appl_ast->op_name);

        if (op_expr == NULL) {
            // CGFAIL("Operator not found: \"%s\"", op_appl_ast->op_name);
            // return -1;
            TRYCR(op_expr, vector_alloc_elem(expr_heap), NULL, -1);
            op_expr->name = NULL;
            op_expr->form = CORE_INTRINSIC;

            // TODO: Copy and dont leak mem
            op_expr->intrinsic.name = op_appl_ast->op_name;
            // TRYCR(op_expr->intrinsic.name, stralloc(op_appl_ast->op_name),
            // NULL, -1);
        }

        expr->form = CORE_APPL;
        core_appl_t *appl = &expr->appl;

        TRYCR(appl->fn, vector_alloc_elem(expr_heap), NULL, -1);
        appl->fn->name = NULL;
        appl->fn->form = CORE_APPL;
        core_appl_t *lhs_appl = &appl->fn->appl;
        lhs_appl->fn = op_expr;
        TRYCR(lhs_appl->arg, vector_alloc_elem(expr_heap), NULL, -1);
        lhs_appl->arg->name = NULL;
        TRY(res,
            coregen_from_ast(op_appl_ast->lhs, env, lhs_appl->arg, expr_heap));

        TRYCR(appl->arg, vector_alloc_elem(expr_heap), NULL, -1);
        appl->arg->name = NULL;
        TRY(res, coregen_from_ast(op_appl_ast->rhs, env, appl->arg, expr_heap));

        break;
    }
    case AST_LET: {
        const ast_let_t *let_ast = &ast->let;

        env_t *let_env;
        TRYCR(let_env, malloc(sizeof(env_t)), NULL, -1);
        TRY(res, env_init(let_env))
        let_env->upper_scope = env;

        TRY(res, coregen_populate_env(&let_ast->bindings, let_env, expr_heap));
        TRY(res, coregen_generate_env(&let_ast->bindings, let_env, expr_heap));

        TRY(res, coregen_from_ast(let_ast->body, let_env, expr, expr_heap));

        env_destroy(let_env);
        free(let_env);

        break;
    }
    case AST_LIT: {
        const ast_lit_t *lit_ast = &ast->lit;

        expr->form = CORE_LITERAL;
        core_literal_t *lit = &expr->literal;

        switch (lit_ast->lit_type) {
        case AST_LIT_TYPE_INT:
            lit->type = CORE_LITERAL_I64;
            lit->i64 = lit_ast->int_lit;
            break;
        default:
            CGFAIL("Unrecognized literal type");
            ast_print(ast, stderr);
            return -1;
        }

        break;
    }
    case AST_VAR: {
        const ast_var_t *var_ast = &ast->var;

        core_expr_t *var_expr = env_get_expr(env, var_ast->name);

        if (var_expr == NULL) {
            CGFAIL("\"%s\" not found", var_ast->name);
            return -1;
        }

        expr->form = CORE_INDIR;
        expr->indir.target = var_expr;

        break;
    }
    case AST_EXP_HAS_TYPE:
    case AST_LAMBDA:
    case AST_DO:
        CGFAIL("Not implemented");
        ast_print(ast, stderr);
        break;
    default:
        CGFAIL("Unrecognized AST Rule");
        ast_print(ast, stderr);
        return -1;
    }

    return 0;
}
