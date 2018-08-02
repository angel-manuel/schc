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

void core_destroy(core_expr_t *expr) { assert(expr != NULL); }

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
    case CORE_APPL:
        break;
    case CORE_LAMBDA:
        break;
    case CORE_LITERAL:
        break;
    case CORE_COND:
        break;
    default:
        fprintf(fp, "%*sForm #%d", indent, "", expr->form);
    }

    return res;
}
