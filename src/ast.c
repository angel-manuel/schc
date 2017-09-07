#include "ast.h"

#include <assert.h>
#include <stdio.h>

#define INDENT 2

void ast_print_indent(const ast_t *ast, FILE *fp, int indent);

void ast_print(const ast_t *ast, FILE *fp) {
    ast_print_indent(ast, fp, 0);
    fprintf(fp, "\n");
}

void ast_print_indent(const ast_t *ast, FILE *fp, int indent) {
    assert(ast != NULL);
    assert(fp != NULL);

    int i;

    switch (ast->rule) {
        case AST_FN_APPL:
            fprintf(fp, "%*sFN_APPL = {\n", indent, "");
            fprintf(fp, "%*s  fn = \n", indent, "");
            ast_print_indent(ast->fn_appl.fn, fp, indent + 2 + INDENT);
            fprintf(fp, "\n");

            fprintf(fp, "%*s  arg = [\n", indent, "");
            ast_print_indent(ast->fn_appl.arg, fp, indent + 2 + INDENT);
            fprintf(fp, "\n");

            fprintf(fp, "%*s}", indent, "");
            break;
        case AST_IF:
            fprintf(fp, "%*sIF {\n", indent, "");

            fprintf(fp, "%*s  cond = \n", indent, "");
            ast_print_indent(ast->if_exp.cond, fp, indent + 2 + INDENT);
            fprintf(fp, "\n");

            fprintf(fp, "%*s  then_branch = \n", indent, "");
            ast_print_indent(ast->if_exp.then_branch, fp, indent + 2 + INDENT);
            fprintf(fp, "\n");

            fprintf(fp, "%*s  else_branch = \n", indent, "");
            ast_print_indent(ast->if_exp.else_branch, fp, indent + 2 + INDENT);
            fprintf(fp, "\n");

            fprintf(fp, "%*s}", indent, "");
            break;
        case AST_LET:
            fprintf(fp, "%*sLET", indent, "");
            break;
        case AST_VAR:
            fprintf(fp, "%*sVAR { %s }", indent, "", ast->var.name);
            break;
        case AST_LIT:
            fprintf(fp, "%*sLIT { number = %d }", indent, "", ast->lit.int_lit);
            break;
        case AST_FN_DECL:
            fprintf(fp, "%*sFN_DECL {\n", indent, "");
            fprintf(fp, "%*s  name = %s\n", indent, "", ast->fn_decl.name);

            fprintf(fp, "%*s  args = [", indent, "");
            for (i = 0; i < ast->fn_decl.vars.len; ++i) {
                fprintf(fp, (i + 1 < ast->fn_decl.vars.len) ? "%s " : "%s]\n",
                    *((char**)vector_get_ref(&ast->fn_decl.vars, i)));
            }

            fprintf(fp, "%*s  body =\n", indent, "");
            ast_print_indent(ast->fn_decl.body, fp, indent + 2 + INDENT);
            fprintf(fp, "\n");

            fprintf(fp, "%*s}", indent, "");
            break;
        case AST_VAL_DECL:
            fprintf(fp, "%*sVAL_DECL {\n", indent, "");
            fprintf(fp, "%*s  name = %s\n", indent, "", ast->val_decl.name);

            fprintf(fp, "%*s  value =\n", indent, "");
            ast_print_indent(ast->val_decl.body, fp, indent + 2 + INDENT);

            fprintf(fp, "%*s}", indent, "");
            break;
        case AST_MODULE:
            fprintf(fp, "%*sMODULE\n", indent, "");

            for (i = 0; i < ast->module.decls.len; ++i) {
                ast_print_indent((const ast_t*)vector_get_ref(&ast->module.decls, i), fp, indent + INDENT);
            }

            break;
        default:
            fprintf(fp, "%*sRule #%d", indent, "", ast->rule);
    };
}
