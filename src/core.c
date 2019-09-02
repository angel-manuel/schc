#include "core.h"

#include <assert.h>
#include <inttypes.h>

#include "env.h"
#include "util.h"

#define INDENT 2
#define FINDENT 2

int core_print_indent(const core_expr_t *expr, FILE *fp, int indent);

int core_print(const core_expr_t *expr, FILE *fp) {
    assert(expr != NULL);
    assert(fp != NULL);

    int res;

    TRY(res, core_print_indent(expr, fp, 0));
    TRYNEG(res, fprintf(fp, "\n"));

    return res;
}

void core_destroy(core_expr_t *expr) {
    assert(expr != NULL);

    switch (expr->form) {
    case CORE_INDIR:
        core_destroy(expr->indir.target);
        break;
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
    case CORE_INTRINSIC:
    case CORE_LITERAL:
    case CORE_NO_FORM:
    default:
        break;
    }
}

int core_print_indent(const core_expr_t *expr, FILE *fp, int indent) {
    assert(expr != NULL);
    assert(fp != NULL);
    assert(indent >= 0);

    int res = 0;

    switch (expr->form) {
    case CORE_NO_FORM:
        TRYNEG(res, fprintf(fp, "NO_FORM"));
        break;
    case CORE_INDIR:
        TRY(res, core_print_indent(expr->indir.target, fp, indent));
        break;
    case CORE_INTRINSIC:
        TRYNEG(res, fprintf(fp, "#%s", expr->intrinsic.name));
        break;
    case CORE_APPL: {
        const core_appl_t *appl = &expr->appl;

        TRYNEG(res, fprintf(fp, "APPL {\n"));

        TRYNEG(res, fprintf(fp, "%*sfn = ", indent + FINDENT, ""));
        // TRY(res, core_print_indent(appl->fn, fp, indent + INDENT));
        TRY(res, core_print_indent(appl->fn, fp, indent + INDENT));
        TRYNEG(res, fprintf(fp, "\n"));

        TRYNEG(res, fprintf(fp, "%*sarg = ", indent + FINDENT, ""));
        // TRY(res, core_print_indent(appl->arg, fp, indent + INDENT));
        TRY(res, core_print_indent(appl->arg, fp, indent + INDENT));
        TRYNEG(res, fprintf(fp, "\n"));

        TRYNEG(res, fprintf(fp, "%*s}", indent, ""));

        break;
    }
    case CORE_LAMBDA: {
        const core_lambda_t *lambda = &expr->lambda;
        const char *typename;

        TRYCR(typename, type_to_str(lambda->type), NULL, -1);

        TRYNEG(res, fprintf(fp, "LAMBDA {\n"));

        TRYNEG(res, fprintf(fp, "%*sarg = %s : %s\n", indent + FINDENT, "",
                            "<arg>", typename));

        TRYNEG(res, fprintf(fp, "%*sbody = ", indent + FINDENT, ""));
        TRY(res, core_print_indent(lambda->body, fp, indent + INDENT));
        TRYNEG(res, fprintf(fp, "\n"));

        TRYNEG(res, fprintf(fp, "%*s}", indent, ""));

        break;
    }
    case CORE_LITERAL: {
        const core_literal_t *literal = &expr->literal;

        switch (literal->type) {
        case CORE_LITERAL_I64:
            TRYNEG(res, fprintf(fp, "%" PRIi64 "i64", literal->i64));
            break;
        default:
            TRYNEG(res, fprintf(fp, "LITERAL { unknown }"));
        }

        break;
    }
    case CORE_COND: {
        const core_cond_t *cond = &expr->cond;

        TRYNEG(res, fprintf(fp, "COND {\n"));

        TRYNEG(res, fprintf(fp, "%*scond = ", indent + FINDENT, ""));
        TRY(res, core_print_indent(cond->cond, fp, indent + INDENT));
        TRYNEG(res, fprintf(fp, "\n"));

        TRYNEG(res, fprintf(fp, "%*sthen_branch = ", indent + FINDENT, ""));
        TRY(res, core_print_indent(cond->then_branch, fp, indent + INDENT));
        TRYNEG(res, fprintf(fp, "\n"));

        TRYNEG(res, fprintf(fp, "%*selse_branch = ", indent + FINDENT, ""));
        TRY(res, core_print_indent(cond->else_branch, fp, indent + INDENT));
        TRYNEG(res, fprintf(fp, "\n"));

        TRYNEG(res, fprintf(fp, "%*s}", indent, ""));
        break;
    }
    default:
        fprintf(fp, "Form #%d", expr->form);
    }

    return res;
}
