#include "core.h"

#include <assert.h>
#include <inttypes.h>

#include "env.h"
#include "util.h"

#define INDENT 4
#define FINDENT 2

int core_print_indent(const env_t *env, const core_expr_t *expr, FILE *fp,
                      int indent);

int core_from_ast(const ast_t *ast, env_t *env, core_expr_t *expr) {
    assert(ast != NULL);
    assert(env != NULL);
    assert(expr != NULL);

    return 0;
}

int core_print(const env_t *env, const core_expr_t *expr, FILE *fp) {
    assert(expr != NULL);
    assert(fp != NULL);

    return core_print_indent(env, expr, fp, 0);
}

void core_destroy(core_expr_t *expr) {
    assert(expr != NULL);

    switch (expr->form) {
    case CORE_APPL: {
        core_appl_t *appl = &expr->appl;

        core_destroy(appl->fn);
        free(appl->fn);
        core_destroy(appl->arg);
        free(appl->arg);

        break;
    }
    case CORE_LAMBDA: {
        core_lambda_t *lambda = &expr->lambda;

        core_destroy(lambda->body);
        free(lambda->body);

        break;
    }
    case CORE_COND: {
        core_cond_t *cond = &expr->cond;

        core_destroy(cond->cond);
        free(cond->cond);
        core_destroy(cond->then_branch);
        free(cond->then_branch);
        core_destroy(cond->else_branch);
        free(cond->else_branch);

        break;
    }
    case CORE_LITERAL:
    case CORE_VALUE:
    case CORE_NO_FORM:
    default:
        break;
    }
}

int core_print_indent(const env_t *env, const core_expr_t *expr, FILE *fp,
                      int indent) {
    assert(expr != NULL);
    assert(fp != NULL);
    assert(indent >= 0);

    int res = 0;

    switch (expr->form) {
    case CORE_NO_FORM:
        TRYNEG(res, fprintf(fp, "%*sNO_FORM", indent, ""));
        break;
    case CORE_VALUE: {
        env_id_t name_id = expr->value.name;
        const char *name;

        TRYCR(name, env_get(env, name_id), NULL, -1);

        TRYNEG(res, fprintf(fp, "%*sVALUE {", indent, ""));
        TRYNEG(res, fprintf(fp, "%*sid = %" PRIu64 "", indent + FINDENT, "",
                            name_id));
        TRYNEG(res, fprintf(fp, "%*svalue = %s", indent + FINDENT, "", name));
        TRYNEG(res, fprintf(fp, "%*s}", indent, ""));
        break;
    }
    case CORE_APPL: {
        const core_appl_t *appl = &expr->appl;

        TRYNEG(res, fprintf(fp, "%*sAPPL {", indent, ""));

        TRYNEG(res, fprintf(fp, "%*sfn = {\n", indent + FINDENT, ""));
        TRY(res, core_print_indent(env, appl->fn, fp, indent + INDENT));
        TRYNEG(res, fprintf(fp, "\n%*s}\n", indent + FINDENT, ""));

        TRYNEG(res, fprintf(fp, "%*sarg = {\n", indent + FINDENT, ""));
        TRY(res, core_print_indent(env, appl->arg, fp, indent + INDENT));
        TRYNEG(res, fprintf(fp, "\n%*s}\n", indent + FINDENT, ""));

        TRYNEG(res, fprintf(fp, "%*s}", indent, ""));

        break;
    }
    case CORE_LAMBDA: {
        const core_lambda_t *lambda = &expr->lambda;
        const char *name, *typename;

        TRYCR(name, env_get(env, lambda->name), NULL, -1);
        TRYCR(typename, type_to_str(lambda->type), NULL, -1);

        TRYNEG(res, fprintf(fp, "%*sLAMBDA {", indent, ""));

        TRYNEG(res, fprintf(fp, "%*sarg = %s : %s", indent + FINDENT, "", name,
                            typename));

        TRYNEG(res, fprintf(fp, "%*sbody = {\n", indent + FINDENT, ""));
        TRY(res, core_print_indent(env, lambda->body, fp, indent + INDENT));
        TRYNEG(res, fprintf(fp, "\n%*s}\n", indent + FINDENT, ""));

        TRYNEG(res, fprintf(fp, "%*s}", indent, ""));

        break;
    }
    case CORE_LITERAL: {
        const core_literal_t *literal = &expr->literal;

        switch (literal->type) {
        case CORE_LITERAL_I64:
            TRYNEG(res, fprintf(fp, "%*sLITERAL { int = %" PRIi64 " }", indent,
                                "", literal->i64));
            break;
        default:
            TRYNEG(res, fprintf(fp, "%*sLITERAL { unknown }", indent, ""));
        }

        break;
    }
    case CORE_COND: {
        const core_cond_t *cond = &expr->cond;

        TRYNEG(res, fprintf(fp, "%*sCOND {", indent, ""));

        TRYNEG(res, fprintf(fp, "%*scond = {\n", indent + FINDENT, ""));
        TRY(res, core_print_indent(env, cond->cond, fp, indent + INDENT));
        TRYNEG(res, fprintf(fp, "\n%*s}\n", indent + FINDENT, ""));

        TRYNEG(res, fprintf(fp, "%*sthen_branch = {\n", indent + FINDENT, ""));
        TRY(res,
            core_print_indent(env, cond->then_branch, fp, indent + INDENT));
        TRYNEG(res, fprintf(fp, "\n%*s}\n", indent + FINDENT, ""));

        TRYNEG(res, fprintf(fp, "%*selse_branch = {\n", indent + FINDENT, ""));
        TRY(res,
            core_print_indent(env, cond->else_branch, fp, indent + INDENT));
        TRYNEG(res, fprintf(fp, "\n%*s}\n", indent + FINDENT, ""));

        TRYNEG(res, fprintf(fp, "%*s}", indent, ""));
        break;
    }
    default:
        fprintf(fp, "%*sForm #%d", indent, "", expr->form);
    }

    return res;
}
