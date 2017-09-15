#include "ast.h"

#include <assert.h>
#include <stdio.h>

#define INDENT 4
#define FINDENT 2

void ast_print_indent(const ast_t *node, FILE *fp, int indent);
void ast_print_vec_indent(const vector_t/*ast_t*/ *nodes, FILE *fp, int indent);

void ast_print(const ast_t *node, FILE *fp) {
    ast_print_indent(node, fp, 0);
    fprintf(fp, "\n");
}

void ast_print_indent(const ast_t *node, FILE *fp, int indent) {
    assert(node != NULL);
    assert(fp != NULL);

    int i;

    switch (node->rule) {
        case AST_NO_RULE:
            fprintf(fp, "%*sNO_RULE", indent, "");
            break;
        case AST_MODULE:
        {
            const ast_module_t *module = &node->module;

            fprintf(fp, "%*sMODULE {\n", indent, "");
            if (module->modid != NULL) {
                fprintf(fp, "%*smodid = %s\n", indent + FINDENT, "", module->modid);
            }
            if (module->exports != NULL) {
                fprintf(fp, "%*sexports =\n", indent + FINDENT, "");
                ast_print_indent(module->exports, fp, indent + INDENT);
                fprintf(fp, "%*s\n", indent + FINDENT, "");
            }
            fprintf(fp, "%*sbody = {\n", indent + FINDENT, "");
            ast_print_indent(module->body, fp, indent + INDENT);
            fprintf(fp, "\n%*s}\n", indent + FINDENT, "");

            fprintf(fp, "%*s}", indent, "");
            
            break;
        } 
        case AST_EXPORTS:
        {
            const ast_exports_t *exports = &node->exports;

            fprintf(fp, "%*sEXPORTS = [", indent, "");
            for(int i = 0; i < exports->exports.len; ++i) {
                const ast_export_t *export = vector_get_ref(&exports->exports, i);
                fprintf(fp, (i + 1 < exports->exports.len) ? "%s ": "%s]", export->exportid);
            }

            break;
        }
        case AST_BODY:
        {
            const ast_body_t *body = &node->body;

            fprintf(fp, "%*sBODY {\n", indent, "");

            fprintf(fp, "%*stopdecls = ", indent + FINDENT, "");
            ast_print_vec_indent(&body->topdecls, fp, indent + FINDENT);
            fprintf(fp, "\n");

            fprintf(fp, "%*s}", indent, "");

            break;
        }
        case AST_NEG:
        {
            const ast_neg_t *neg = &node->neg;

            fprintf(fp, "%*sNEG {\n", indent, "");
            ast_print_indent(neg->expr, fp, indent + INDENT);
            fprintf(fp, "\n");
            fprintf(fp, "%*s}", indent, "");
            break;
        }
        case AST_FN_APPL:
        {
            const ast_fn_appl_t *fn_appl = &node->fn_appl;

            fprintf(fp, "%*sFN_APPL {\n", indent, "");
            fprintf(fp, "%*sfn = {\n", indent + FINDENT, "");
            ast_print_indent(fn_appl->fn, fp, indent + INDENT);
            fprintf(fp, "\n%*s}\n", indent + FINDENT, "");

            fprintf(fp, "%*sarg = {\n", indent + FINDENT, "");
            ast_print_indent(fn_appl->arg, fp, indent + INDENT);
            fprintf(fp, "\n%*s}\n", indent + FINDENT, "");

            fprintf(fp, "%*s}", indent, "");
            break;
        }
        case AST_OP_APPL:
        {
            const ast_op_appl_t *op_appl = &node->op_appl;

            fprintf(fp, "%*sOP_APPL {\n", indent, "");
            fprintf(fp, "%*sop_name = %s\n", indent + FINDENT, "", node->op_appl.op_name);

            fprintf(fp, "%*slhs = {\n", indent + FINDENT, "");
            ast_print_indent(op_appl->lhs, fp, indent + INDENT);
            fprintf(fp, "\n%*s}\n", indent + FINDENT, "");

            fprintf(fp, "%*srhs = {\n", indent + FINDENT, "");
            ast_print_indent(op_appl->rhs, fp, indent + INDENT);
            fprintf(fp, "\n%*s}\n", indent + FINDENT, "");
        
            fprintf(fp, "%*s}", indent, "");
            break;
        }
        case AST_IF:
        {
            const ast_if_t *if_exp = &node->if_exp;

            fprintf(fp, "%*sIF {\n", indent, "");

            fprintf(fp, "%*scond = {\n", indent + FINDENT, "");
            ast_print_indent(if_exp->cond, fp, indent + INDENT);
            fprintf(fp, "\n%*s}\n", indent + FINDENT, "");

            fprintf(fp, "%*sthen_branch = {\n", indent + FINDENT, "");
            ast_print_indent(if_exp->then_branch, fp, indent + INDENT);
            fprintf(fp, "\n%*s}\n", indent + FINDENT, "");

            fprintf(fp, "%*selse_branch = {\n", indent + FINDENT, "");
            ast_print_indent(if_exp->else_branch, fp, indent + INDENT);
            fprintf(fp, "\n%*s}\n", indent + FINDENT, "");

            fprintf(fp, "%*s}", indent, "");
            break;
        }
        case AST_LET:
        {
            const ast_let_t *let = &node->let;

            fprintf(fp, "%*sLET {\n", indent, "");
            fprintf(fp, "%*sbindings = ", indent + FINDENT, "");
            ast_print_vec_indent(&let->bindings, fp, indent + FINDENT);
            fprintf(fp, "\n");

            fprintf(fp, "%*sbody = {\n", indent + FINDENT, "");
            ast_print_indent(let->body, fp, indent + INDENT);
            fprintf(fp, "\n%*s}\n", indent + FINDENT, "");

            fprintf(fp, "%*s}", indent, "");

            break;
        }
        case AST_VAR:
            fprintf(fp, "%*sVAR { %s }", indent, "", node->var.name);
            break;
        case AST_CON:
            fprintf(fp, "%*sCON { %s }", indent, "", node->con.name);
            break;
        case AST_LIT:
            fprintf(fp, "%*sLIT { number = %d }", indent, "", node->lit.int_lit);
            break;
        case AST_FIXITY_DECL:
            fprintf(fp, "%*sINFIX%c %d %s", indent, "",
                node->fixity_decl.associativity, node->fixity_decl.fixity, node->fixity_decl.op);
            break;
        case AST_FN_DECL:
        {
            const ast_fn_decl_t *fn_decl = &node->fn_decl;

            fprintf(fp, "%*sFN_DECL {\n", indent, "");
            fprintf(fp, "%*sname = %s\n", indent + FINDENT, "", fn_decl->name);

            fprintf(fp, "%*sargs = [", indent + FINDENT, "");
            for (i = 0; i < fn_decl->vars.len; ++i) {
                fprintf(fp, (i + 1 < fn_decl->vars.len) ? "%s " : "%s]\n",
                    *((char**)vector_get_ref(&fn_decl->vars, i)));
            }

            fprintf(fp, "%*sbody = {\n", indent + FINDENT, "");
            ast_print_indent(fn_decl->body, fp, indent + INDENT);
            fprintf(fp, "\n%*s}\n", indent + FINDENT, "");

            fprintf(fp, "%*s}", indent, "");
            break;
        }
        case AST_VAL_DECL:
        {
            const ast_val_decl_t *val_decl = &node->val_decl;

            fprintf(fp, "%*sVAL_DECL {\n", indent, "");
            fprintf(fp, "%*sname = %s\n", indent + FINDENT, "", val_decl->name);

            fprintf(fp, "%*svalue = {\n", indent + FINDENT, "");
            ast_print_indent(val_decl->body, fp, indent + INDENT);
            fprintf(fp, "\n%*s}\n", indent + FINDENT, "");

            fprintf(fp, "%*s}", indent, "");
            break;
        }
        case AST_HAS_TYPE_DECL:
        {
            const ast_has_type_decl_t *has_type_decl = &node->has_type_decl;

            fprintf(fp, "%*sHAS_TYPE {\n", indent, "");
            fprintf(fp, "%*ssymbol_name = %s\n", indent + FINDENT, "", has_type_decl->symbol_name);
            fprintf(fp, "%*stype = {\n", indent + FINDENT, "");
            ast_print_indent(has_type_decl->type_exp, fp, indent + INDENT);
            fprintf(fp, "\n%*s}\n", indent + FINDENT, "");
            fprintf(fp, "%*s}", indent, "");
            break;
        }
        default:
            fprintf(fp, "%*sRule #%d", indent, "", node->rule);
    };
}

void ast_print_vec_indent(const vector_t/*ast_t*/ *nodes, FILE *fp, int indent) {
    assert(nodes != NULL);
    assert(fp != NULL);

    fprintf(fp, "[\n");
    for (int i = 0; i < nodes->len; ++i) {
        ast_print_indent((const ast_t*)vector_get_ref(nodes, i), fp, indent + FINDENT);
        fprintf(fp, i + 1 == nodes->len ? "\n" : ",\n");
    }
    fprintf(fp, "%*s]", indent, "");
}