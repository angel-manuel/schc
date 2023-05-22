#include "ast.h"

#include <assert.h>
#include <stdio.h>

#define INDENT 4
#define FINDENT 2

#define FREE(mem) ALLOCATOR_FREE(allocator, (mem))

void ast_print_indent(const ast_t *node, FILE *fp, int indent);
void ast_print_vec_indent(const vector_t /*ast_t*/ *nodes, FILE *fp,
                          int indent);

void ast_print(const ast_t *node, FILE *fp) {
    ast_print_indent(node, fp, 0);
    fprintf(fp, "\n");
}

void ast_destroy(ast_t *node, const allocator_t *allocator) {
    assert(node != NULL);
    assert(allocator != NULL);

    switch (node->rule) {
    case AST_NO_RULE:
        break;
    case AST_MODULE: {
        ast_module_t *module = &node->module;

        if (module->modid != NULL) {
            FREE(module->modid);
        }

        for (int i = 0; i < module->exports.len; ++i) {
            const ast_export_t *export = vector_get_ref(&module->exports, i);
            FREE(export->exportid);
        }
        vector_destroy(&module->exports);

        ast_destroy(module->body, allocator);
        FREE(module->body);
        break;
    }
    case AST_BODY: {
        ast_body_t *body = &node->body;

        for (int i = 0; i < body->topdecls.len; ++i) {
            ast_destroy((ast_t *)vector_get_ref(&body->topdecls, i), allocator);
        }
        vector_destroy(&body->topdecls);
        break;
    }
    case AST_NEG: {
        ast_neg_t *neg = &node->neg;

        ast_destroy(neg->expr, allocator);
        break;
    }
    case AST_FN_APPL: {
        ast_fn_appl_t *fn_appl = &node->fn_appl;

        ast_destroy(fn_appl->arg, allocator);
        FREE(fn_appl->arg);
        ast_destroy(fn_appl->fn, allocator);
        FREE(fn_appl->fn);
        break;
    }
    case AST_OP_APPL: {
        ast_op_appl_t *op_appl = &node->op_appl;

        FREE(op_appl->op_name);
        ast_destroy(op_appl->lhs, allocator);
        FREE(op_appl->lhs);
        ast_destroy(op_appl->rhs, allocator);
        FREE(op_appl->rhs);
        break;
    }
    case AST_IF: {
        ast_if_t *if_exp = &node->if_exp;

        ast_destroy(if_exp->else_branch, allocator);
        FREE(if_exp->else_branch);
        ast_destroy(if_exp->then_branch, allocator);
        FREE(if_exp->then_branch);
        ast_destroy(if_exp->cond, allocator);
        FREE(if_exp->cond);

        break;
    }
    case AST_DO: {
        ast_do_t *do_exp = &node->do_exp;

        for (int i = 0; i < do_exp->steps.len; ++i) {
            ast_destroy((ast_t *)vector_get_ref(&do_exp->steps, i), allocator);
        }
        vector_destroy(&do_exp->steps);

        break;
    }
    case AST_LET: {
        ast_let_t *let = &node->let;

        if (let->body != NULL) {
            ast_destroy(let->body, allocator);
            FREE(let->body);
        }

        for (int i = 0; i < let->bindings.len; ++i) {
            ast_destroy((ast_t *)vector_get_ref(&let->bindings, i), allocator);
        }
        vector_destroy(&let->bindings);

        break;
    }
    case AST_VAR: {
        ast_var_t *var = &node->var;

        FREE(var->name);

        break;
    }
    case AST_CON: {
        ast_con_t *con = &node->con;

        FREE(con->name);

        break;
    }
    case AST_LIT: {
        ast_lit_t *lit = &node->lit;

        if (lit->lit_type == AST_LIT_TYPE_STR) {
            FREE(lit->str_lit);
        }
        break;
    }
    case AST_FIXITY_DECL: {
        ast_fixity_decl_t *fixity_decl = &node->fixity_decl;

        FREE(fixity_decl->op);

        break;
    }
    case AST_FN_DECL: {
        ast_fn_decl_t *fn_decl = &node->fn_decl;

        ast_destroy(fn_decl->body, allocator);
        FREE(fn_decl->body);

        for (int i = 0; i < fn_decl->vars.len; ++i) {
            FREE(*(char **)vector_get_ref(&fn_decl->vars, i));
        }
        vector_destroy(&fn_decl->vars);

        FREE(fn_decl->name);

        break;
    }
    case AST_VAL_DECL: {
        ast_val_decl_t *val_decl = &node->val_decl;

        ast_destroy(val_decl->body, allocator);
        FREE(val_decl->body);

        FREE(val_decl->name);

        break;
    }
    case AST_HAS_TYPE_DECL: {
        ast_has_type_decl_t *has_type_decl = &node->has_type_decl;

        ast_destroy(has_type_decl->type_exp, allocator);
        FREE(has_type_decl->type_exp);

        FREE(has_type_decl->symbol_name);

        break;
    }
    default:
        break;
    }
}

void ast_print_indent(const ast_t *node, FILE *fp, int indent) {
    assert(node != NULL);
    assert(fp != NULL);

    int i;

    switch (node->rule) {
    case AST_NO_RULE:
        fprintf(fp, "%*sNO_RULE", indent, "");
        break;
    case AST_MODULE: {
        const ast_module_t *module = &node->module;

        fprintf(fp, "%*sMODULE {\n", indent, "");
        if (module->modid != NULL) {
            fprintf(fp, "%*smodid = %s\n", indent + FINDENT, "", module->modid);
        }

        if (module->exports.len) {
            fprintf(fp, "%*sexports = [", indent + FINDENT, "");

            for (int i = 0; i < module->exports.len; ++i) {
                const ast_export_t *export =
                    vector_get_ref(&module->exports, i);
                fprintf(fp, (i + 1 < module->exports.len) ? "%s " : "%s]\n",
                        export->exportid);
            }
        }

        fprintf(fp, "%*sbody = {\n", indent + FINDENT, "");
        ast_print_indent(module->body, fp, indent + INDENT);
        fprintf(fp, "\n%*s}\n", indent + FINDENT, "");

        fprintf(fp, "%*s}", indent, "");

        break;
    }
    case AST_BODY: {
        const ast_body_t *body = &node->body;

        fprintf(fp, "%*sBODY {\n", indent, "");

        fprintf(fp, "%*stopdecls = ", indent + FINDENT, "");
        ast_print_vec_indent(&body->topdecls, fp, indent + FINDENT);
        fprintf(fp, "\n");

        fprintf(fp, "%*s}", indent, "");

        break;
    }
    case AST_NEG: {
        const ast_neg_t *neg = &node->neg;

        fprintf(fp, "%*sNEG {\n", indent, "");
        ast_print_indent(neg->expr, fp, indent + INDENT);
        fprintf(fp, "\n");
        fprintf(fp, "%*s}", indent, "");
        break;
    }
    case AST_FN_APPL: {
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
    case AST_OP_APPL: {
        const ast_op_appl_t *op_appl = &node->op_appl;

        fprintf(fp, "%*sOP_APPL {\n", indent, "");
        fprintf(fp, "%*sop_name = %s\n", indent + FINDENT, "",
                node->op_appl.op_name);

        fprintf(fp, "%*slhs = {\n", indent + FINDENT, "");
        ast_print_indent(op_appl->lhs, fp, indent + INDENT);
        fprintf(fp, "\n%*s}\n", indent + FINDENT, "");

        fprintf(fp, "%*srhs = {\n", indent + FINDENT, "");
        ast_print_indent(op_appl->rhs, fp, indent + INDENT);
        fprintf(fp, "\n%*s}\n", indent + FINDENT, "");

        fprintf(fp, "%*s}", indent, "");
        break;
    }
    case AST_IF: {
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
    case AST_DO: {
        const ast_do_t *do_exp = &node->do_exp;

        fprintf(fp, "%*sDO {\n", indent, "");
        fprintf(fp, "%*ssteps = ", indent + FINDENT, "");
        ast_print_vec_indent(&do_exp->steps, fp, indent + FINDENT);
        fprintf(fp, "\n%*s}", indent, "");

        break;
    }
    case AST_LET: {
        const ast_let_t *let = &node->let;

        fprintf(fp, "%*sLET {\n", indent, "");
        fprintf(fp, "%*sbindings = ", indent + FINDENT, "");
        ast_print_vec_indent(&let->bindings, fp, indent + FINDENT);
        fprintf(fp, "\n");

        if (let->body != NULL) {
            fprintf(fp, "%*sbody = {\n", indent + FINDENT, "");
            ast_print_indent(let->body, fp, indent + INDENT);
            fprintf(fp, "\n%*s}\n", indent + FINDENT, "");
        }

        fprintf(fp, "%*s}", indent, "");

        break;
    }
    case AST_VAR:
        fprintf(fp, "%*sVAR { %s }", indent, "", node->var.name);
        break;
    case AST_CON:
        fprintf(fp, "%*sCON { %s }", indent, "", node->con.name);
        break;
    case AST_LIT: {
        const ast_lit_t *lit = &node->lit;

        switch (lit->lit_type) {
        case AST_LIT_TYPE_INT:
            fprintf(fp, "%*sLIT { number = %d }", indent, "", lit->int_lit);
            break;
        case AST_LIT_TYPE_STR:
            fprintf(fp, "%*sLIT { string = %s }", indent, "", lit->str_lit);
            break;
        default:
            fprintf(fp, "%*sLIT { unknown }", indent, "");
        }

        break;
    }
    case AST_FIXITY_DECL:
        fprintf(fp, "%*sINFIX%c %d %s", indent, "",
                node->fixity_decl.associativity, node->fixity_decl.fixity,
                node->fixity_decl.op);
        break;
    case AST_FN_DECL: {
        const ast_fn_decl_t *fn_decl = &node->fn_decl;

        fprintf(fp, "%*sFN_DECL {\n", indent, "");
        fprintf(fp, "%*sname = %s\n", indent + FINDENT, "", fn_decl->name);

        fprintf(fp, "%*sargs = [", indent + FINDENT, "");
        for (i = 0; i < fn_decl->vars.len; ++i) {
            fprintf(fp, (i + 1 < fn_decl->vars.len) ? "%s " : "%s]\n",
                    *((char **)vector_get_ref(&fn_decl->vars, i)));
        }

        fprintf(fp, "%*sbody = {\n", indent + FINDENT, "");
        ast_print_indent(fn_decl->body, fp, indent + INDENT);
        fprintf(fp, "\n%*s}\n", indent + FINDENT, "");

        fprintf(fp, "%*s}", indent, "");
        break;
    }
    case AST_VAL_DECL: {
        const ast_val_decl_t *val_decl = &node->val_decl;

        fprintf(fp, "%*sVAL_DECL {\n", indent, "");
        fprintf(fp, "%*sname = %s\n", indent + FINDENT, "", val_decl->name);

        fprintf(fp, "%*svalue = {\n", indent + FINDENT, "");
        ast_print_indent(val_decl->body, fp, indent + INDENT);
        fprintf(fp, "\n%*s}\n", indent + FINDENT, "");

        fprintf(fp, "%*s}", indent, "");
        break;
    }
    case AST_HAS_TYPE_DECL: {
        const ast_has_type_decl_t *has_type_decl = &node->has_type_decl;

        fprintf(fp, "%*sHAS_TYPE {\n", indent, "");
        fprintf(fp, "%*ssymbol_name = %s\n", indent + FINDENT, "",
                has_type_decl->symbol_name);
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

void ast_print_vec_indent(const vector_t /*ast_t*/ *nodes, FILE *fp,
                          int indent) {
    assert(nodes != NULL);
    assert(fp != NULL);

    fprintf(fp, "[\n");
    for (int i = 0; i < nodes->len; ++i) {
        ast_print_indent((const ast_t *)vector_get_ref(nodes, i), fp,
                         indent + FINDENT);
        fprintf(fp, i + 1 == nodes->len ? "\n" : ",\n");
    }
    fprintf(fp, "%*s]", indent, "");
}