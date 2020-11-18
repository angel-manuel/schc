#include "coregen.h"

#include <assert.h>
#include <string.h>

#include "data/vector.h"
#include "util.h"

#define ALLOC(x) ALLOCATOR_ALLOC(allocator, (x))

#define CGFAIL(fmt, ...)                                                       \
    fprintf(stderr, "Coregen FAIL(%s:%d): " fmt "\n", __FILE__, __LINE__,      \
            ##__VA_ARGS__);
#define CGWARN(fmt, ...)                                                       \
    fprintf(stderr, "Coregen WARN(%s:%d): " fmt "\n", __FILE__, __LINE__,      \
            ##__VA_ARGS__);

int coregen_populate_env(const vector_t /* core_ast_t */ *decls, env_t *env,
                         allocator_t *allocator);
int coregen_generate_env(const vector_t /* core_ast_t */ *decls, env_t *env,
                         allocator_t *allocator);
int coregen_from_ast(const ast_t *ast, env_t *env, core_expr_t *expr,
                     allocator_t *allocator);

int coregen_from_module_ast(const ast_t *ast, env_t *env,
                            allocator_t *allocator) {
    assert(ast != NULL);
    assert(env != NULL);
    assert(allocator != NULL);

    int res;

    if (ast->rule == AST_MODULE) {
        assert(ast->module.body->rule == AST_BODY);

        const vector_t *decls = &ast->module.body->body.topdecls;

        TRY(res, coregen_populate_env(decls, env, allocator));
        TRY(res, coregen_generate_env(decls, env, allocator));
    } else {
        CGFAIL("Not a module");
        return -1;
    }

    return 0;
}

int coregen_populate_env(const vector_t /* core_ast_t */ *decls, env_t *env,
                         allocator_t *allocator) {
    assert(decls != NULL);
    assert(env != NULL);
    assert(allocator != NULL);

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
            TRYCR(new_expr, ALLOC(sizeof(core_expr_t)), NULL, -1);

            new_expr->name = val_decl->name;

            TRY(res, env_put_expr(env, val_decl->name, new_expr));

            break;
        }
        case AST_FN_DECL: {
            const ast_fn_decl_t *fn_decl = &decl->fn_decl;

            core_expr_t *new_expr;
            TRYCR(new_expr, ALLOC(sizeof(core_expr_t)), NULL, -1);

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
                         allocator_t *allocator) {
    assert(decls != NULL);
    assert(env != NULL);
    assert(allocator != NULL);

    int res;

    for (size_t i = 0; i < decls->len; ++i) {
        const ast_t *decl = (const ast_t *)vector_get_ref(decls, i);

        switch (decl->rule) {
        case AST_VAL_DECL: {
            const ast_val_decl_t *val_decl = &decl->val_decl;
            core_expr_t *expr;

            TRYCR(expr, env_get_expr(env, val_decl->name), NULL, -1);

            TRY(res, coregen_from_ast(val_decl->body, env, expr, allocator));

            break;
        }
        case AST_FN_DECL: {
            const ast_fn_decl_t *fn_decl = &decl->fn_decl;
            core_expr_t *expr;

            TRYCR(expr, env_get_expr(env, fn_decl->name), NULL, -1);

            expr->form = CORE_LAMBDA;
            expr->name = fn_decl->name;
            // TRYCR(expr->name, stralloc(fn_decl->name), NULL, -1);
            core_lambda_t *lambda = &expr->lambda;

            TRY(res, env_init_with_allocator(&lambda->args, allocator));
            lambda->args.upper_scope = env;

            for (size_t i = 0; i < fn_decl->vars.len; ++i) {
                const char *varname =
                    *(const char **)vector_get_ref(&fn_decl->vars, i);

                core_expr_t *var_expr;
                TRYCR(var_expr, ALLOC(sizeof(core_expr_t)), NULL, -1);

                var_expr->name = varname;
                // TRYCR(var_expr->name, stralloc(varname), NULL, -1);
                var_expr->form = CORE_PLACEHOLDER;

                TRY(res, env_put_expr(&lambda->args, varname, var_expr));
            }

            TRYCR(lambda->body, ALLOC(sizeof(core_expr_t)), NULL, -1);

            TRY(res, coregen_from_ast(fn_decl->body, &lambda->args,
                                      lambda->body, allocator));

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
                     allocator_t *allocator) {
    assert(ast != NULL);
    assert(env != NULL);
    assert(expr != NULL);
    assert(allocator != NULL);

    int res;

    expr->form = CORE_NO_FORM;
    expr->name = NULL;

    switch (ast->rule) {
    case AST_CON: {
        const ast_con_t *con_ast = &ast->con;

        expr->form = CORE_CONSTRUCTOR;
        core_constructor_t *constructor = &expr->constructor;

        TRYCR(constructor->name, stralloc(con_ast->name), NULL, -1);

        break;
    }
    case AST_NEG: {
        const ast_neg_t *neg_ast = &ast->neg;

        expr->form = CORE_APPL;
        core_appl_t *appl = &expr->appl;

        // TODO: Prealloc intrisics
        TRYCR(appl->fn, ALLOC(sizeof(core_expr_t)), NULL, -1);
        appl->fn->name = NULL;
        appl->fn->form = CORE_INTRINSIC;
        appl->fn->intrinsic.name = "neg";

        TRYCR(appl->arg, ALLOC(sizeof(core_expr_t)), NULL, -1);
        appl->arg->name = NULL;
        TRY(res, coregen_from_ast(neg_ast->expr, env, appl->arg, allocator));

        break;
    }
    case AST_FN_APPL: {
        const ast_fn_appl_t *fn_appl_ast = &ast->fn_appl;

        expr->form = CORE_APPL;
        core_appl_t *appl = &expr->appl;

        TRYCR(appl->fn, ALLOC(sizeof(core_expr_t)), NULL, -1);
        appl->fn->name = NULL;
        TRY(res, coregen_from_ast(fn_appl_ast->fn, env, appl->fn, allocator));

        TRYCR(appl->arg, ALLOC(sizeof(core_expr_t)), NULL, -1);
        appl->arg->name = NULL;
        TRY(res, coregen_from_ast(fn_appl_ast->arg, env, appl->arg, allocator));

        break;
    }
    case AST_OP_APPL: {
        const ast_op_appl_t *op_appl_ast = &ast->op_appl;

        core_expr_t *op_expr = env_get_expr(env, op_appl_ast->op_name);

        if (op_expr == NULL) {
            CGFAIL("Operator not found: \"%s\"", op_appl_ast->op_name);

            fprintf(stderr, "Scope:\n");
            env_print_scope(env, 1, stderr);

            return -1;
        }

        expr->form = CORE_APPL;
        core_appl_t *appl = &expr->appl;

        TRYCR(appl->fn, ALLOC(sizeof(core_expr_t)), NULL, -1);
        appl->fn->name = NULL;
        appl->fn->form = CORE_APPL;
        core_appl_t *lhs_appl = &appl->fn->appl;
        lhs_appl->fn = op_expr;
        TRYCR(lhs_appl->arg, ALLOC(sizeof(core_expr_t)), NULL, -1);
        lhs_appl->arg->name = NULL;
        TRY(res,
            coregen_from_ast(op_appl_ast->lhs, env, lhs_appl->arg, allocator));

        TRYCR(appl->arg, ALLOC(sizeof(core_expr_t)), NULL, -1);
        appl->arg->name = NULL;
        TRY(res, coregen_from_ast(op_appl_ast->rhs, env, appl->arg, allocator));

        break;
    }
    case AST_LET: {
        const ast_let_t *let_ast = &ast->let;

        env_t *let_env;
        TRYCR(let_env, malloc(sizeof(env_t)), NULL, -1);
        TRY(res, env_init_with_allocator(let_env, allocator));
        let_env->upper_scope = env;

        TRY(res, coregen_populate_env(&let_ast->bindings, let_env, allocator));
        TRY(res, coregen_generate_env(&let_ast->bindings, let_env, allocator));

        TRY(res, coregen_from_ast(let_ast->body, let_env, expr, allocator));

        let_env->upper_scope = NULL;
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
    case AST_IF: {
        const ast_if_t *if_ast = &ast->if_exp;

        expr->form = CORE_COND;
        core_cond_t *cond = &expr->cond;

        TRYCR(cond->cond, ALLOC(sizeof(core_expr_t)), NULL, -1);
        TRYCR(cond->then_branch, ALLOC(sizeof(core_expr_t)), NULL, -1);
        TRYCR(cond->else_branch, ALLOC(sizeof(core_expr_t)), NULL, -1);

        TRY(res, coregen_from_ast(if_ast->cond, env, cond->cond, allocator));
        TRY(res, coregen_from_ast(if_ast->then_branch, env, cond->then_branch,
                                  allocator));
        TRY(res, coregen_from_ast(if_ast->else_branch, env, cond->else_branch,
                                  allocator));

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
