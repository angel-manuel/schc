#define MY_OBJECT parser->

#include "parser.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "util.h"

#define PALLOC(size) ALLOCATOR_ALLOC(parser->allocator, (size))
#define PSTRALLOC(str) ALLOCATOR_STRALLOC(parser->allocator, (str))
#define PREALLOC(ptr, size) ALLOCATOR_REALLOC(parser->allocator, (ptr), (size))
#define PFREE(mem) ALLOCATOR_FREE(parser->allocator, (mem))

#define PFAIL(fail)                                                            \
    fprintf(stderr, "Parser FAIL(%s:%d): %s, got %s\n", __FILE__, __LINE__,    \
            fail, strtoken(parser->token));

#define TRYP(res, exp)                                                         \
    do {                                                                       \
        (res) = (exp);                                                         \
        if ((res) == -1) {                                                     \
            PFAIL(#exp);                                                       \
            return -1;                                                         \
        } else if ((res) == 0) {                                               \
            return 0;                                                          \
        }                                                                      \
    } while (0);

int accept(parser_t *parser, token_t token);
int maybe(int res);
int soft(int res);
int hard(int res);

int operator(parser_t *parser);

int root(parser_t *parser, ast_t *node);
int module(parser_t *parser, ast_t *node);
int exports(parser_t *parser, vector_t /*ast_export_t*/ *exports);
int export(parser_t *parser, ast_export_t *ex);
int body(parser_t *parser, ast_t *node);
int declaration(parser_t *parser, ast_t *node);
int function(parser_t *parser, char *decl_name, ast_t *node);
int value(parser_t *parser, char *decl_name, ast_t *node);
int has_type(parser_t *parser, char *decl_name, ast_t *node);
int data(parser_t *parser, ast_t *node);
int data_constructor(parser_t *parser, ast_t *node);

int identable(parser_t *parser, vector_t /*ast_t*/ *nodes,
              int (*element_parser)(parser_t *, ast_t *));

int expression(parser_t *parser, ast_t *node);
int unary_neg(parser_t *parser, ast_t *node);
int fexpression(parser_t *parser, ast_t *node);
int aexpression(parser_t *parser, ast_t *node);
int let_exp(parser_t *parser, ast_t *node);
int if_exp(parser_t *parser, ast_t *node);
int do_step(parser_t *parser, ast_t *node);
int do_exp(parser_t *parser, ast_t *node);
int do_let_exp(parser_t *parser, ast_t *node);
int var(parser_t *parser, ast_t *node);
int con(parser_t *parser, ast_t *node);
int atype(parser_t *parser, ast_t *node);
int btype(parser_t *parser, ast_t *node);
int type(parser_t *parser, ast_t *node);

int lit(parser_t *parser, ast_t *node);
int number(parser_t *parser, ast_t *node);
int string(parser_t *parser, ast_t *node);

int bindings(parser_t *parser, vector_t *binds);

void open_indent(parser_t *parser);
void open_free_indent(parser_t *parser);
void continue_indent(parser_t *parser);
void close_indent(parser_t *parser);

int parser_init(parser_t *parser) {
    assert(parser != NULL);

    int res;

    parser->token = -1;
    parser->ptext = NULL;
    parser->flags = PARSER_NONE;
    TRY(res, stack_init(&parser->indent_stack, sizeof(int)));

    return 0;
}

void parser_destroy(parser_t *parser) {
    assert(parser != NULL);

    stack_destroy(&parser->indent_stack);
}

int parser_parse(parser_t *parser, ast_t *root, allocator_t *allocator) {
    assert(parser != NULL);
    assert(root != NULL);
    assert(allocator != NULL);

    parser->allocator = allocator;
    parser->token = yylex();
    TRYCR(parser->ptext, PSTRALLOC(yytext), NULL, -1);

    int res = module(parser, root);

    if (parser->ptext != NULL) {
        PFREE(parser->ptext);
    }

    parser->allocator = NULL;

    if (res == -1) {
        return -1;
    }

    return 0;
}

char *parser_get_text(parser_t *parser) {
    assert(parser != NULL);

    char *text = parser->ptext;
    parser->ptext = NULL;
    return text;
}

// Parsing

int accept(parser_t *parser, token_t token) {
    assert(parser != NULL);
    assert(token != 0);

    int indent = yycolumn - strlen(yytext);

    if (parser->token == token) {
        if (parser->flags & PARSER_NEW_INDENT_ACCEPTED) {
            int ret;
            parser->flags &= !PARSER_NEW_INDENT_ACCEPTED;
            TRY(ret, stack_push(&parser->indent_stack, &indent));
            printf("Indent: %d\n", indent);
        } else {
            const int *indent_level = stack_peek(&parser->indent_stack);
            if (indent_level != NULL) {
                if (indent < *indent_level) {
                    return -1;
                } else if (indent == *indent_level) {
                    if (parser->flags & PARSER_CONTINUE_INDENT) {
                        parser->flags &= !PARSER_CONTINUE_INDENT;
                    } else {
                        return -1;
                    }
                }
            }
        }

        if (parser->ptext != NULL) {
            PFREE(parser->ptext);
            parser->ptext = NULL;
        }

        printf("%-20s%s\n", strtoken(token), yytext);
        TRYCR(parser->ptext, PSTRALLOC(yytext), NULL, -1);
        parser->token = yylex();

        return token;
    }

    return -1;
}

int maybe(int res) { return res == 0 ? TOK_NO_TOK : res; }

int soft(int res) {
    if (res == -1) {
        return 0;
    }

    return res;
}

int hard(int res) {
    if (res <= 0) {
        return -1;
    }

    return res;
}

void no_indent(parser_t *parser) {
    assert(parser != NULL);

    int no_indent = -1;
    stack_push(&parser->indent_stack, &no_indent);
}

void open_indent(parser_t *parser) {
    assert(parser != NULL);

    parser->flags |= PARSER_NEW_INDENT_ACCEPTED;
}

void open_free_indent(parser_t *parser) {
    assert(parser != NULL);

    int free_indent = -1;

    stack_push(&parser->indent_stack, &free_indent);

    printf("Indent: free\n");
}

void continue_indent(parser_t *parser) {
    assert(parser != NULL);

    parser->flags |= PARSER_CONTINUE_INDENT;
}

void close_indent(parser_t *parser) {
    assert(parser != NULL);

    stack_pop(&parser->indent_stack, NULL);

    int *curr_indent = (int *)stack_peek(&parser->indent_stack);
    if (curr_indent == NULL) {
        printf("Indent: NULL\n");
    } else {
        printf("Indent: %d\n", *curr_indent);
    }

    parser->flags &= !PARSER_CONTINUE_INDENT;
}

int identable(parser_t *parser, vector_t /*ast_t*/ *nodes,
              int (*element_parser)(parser_t *, ast_t *)) {
    assert(parser != NULL);
    assert(nodes != NULL);
    assert(element_parser != NULL);

    int res;

    TRYP(res, maybe(soft(accept(parser, '{'))));

    if (res != TOK_NO_TOK) {
        open_free_indent(parser);
        while (res != TOK_NO_TOK) {
            ast_t *new_node;
            TRYCR(new_node, (ast_t *)vector_alloc_elem(nodes), NULL, -1);
            TRYP(res, element_parser(parser, new_node));
            TRYP(res, maybe(soft(accept(parser, ';'))));
        }

        TRYP(res, accept(parser, '}'));
        close_indent(parser);
    } else {
        open_indent(parser);

        for (;;) {
            ast_t *new_node;
            TRYCR(new_node, (ast_t *)vector_alloc_elem(nodes), NULL, -1);
            TRYP(res, maybe(soft(element_parser(parser, new_node))));

            if (res == TOK_NO_TOK) {
                break;
            } else {
                continue_indent(parser);
            }
        };

        nodes->len--;

        close_indent(parser);
    }

    res = TOK_ANY;

    return res;
}

// Rules

int operator(parser_t *parser) {
    return soft(accept(parser, TOK_OP)) || soft(accept(parser, '-')) ||
           soft(accept(parser, ':')) || soft(accept(parser, '=')) ||
           soft(accept(parser, '\\')) || soft(accept(parser, '|')) ||
           soft(accept(parser, '@')) || soft(accept(parser, TOK_OP_RANGE)) ||
           soft(accept(parser, TOK_OP_HASTYPE)) ||
           soft(accept(parser, TOK_OP_L_ARROW)) ||
           soft(accept(parser, TOK_OP_R_ARROW)) ||
           soft(accept(parser, TOK_OP_R_FAT_ARROW));
}

int root(parser_t *parser, ast_t *node) {
    assert(parser != NULL);
    assert(node != NULL);

    int res;

    TRYP(res, module(parser, node));

    return res;
}

int module(parser_t *parser, ast_t *node) {
    assert(parser != NULL);
    assert(node != NULL);

    int res;
    ast_module_t *module = &node->module;
    module->modid = NULL;
    TRY(res, vector_init_with_allocator(&module->exports, sizeof(ast_export_t),
                                        parser->allocator));

    TRYP(res, maybe(soft(accept(parser, TOK_MODULE))));

    if (res != TOK_NO_TOK) {
        TRYP(res, accept(parser, TOK_CONID));
        module->modid = parser_get_text(parser);

        TRYP(res, maybe(exports(parser, &module->exports)));

        TRYP(res, accept(parser, TOK_WHERE));
    }

    TRYCR(module->body, (ast_t *)PALLOC(sizeof(ast_t)), NULL, -1);
    TRYP(res, body(parser, module->body));

    node->rule = AST_MODULE;
    return res;
}

int exports(parser_t *parser, vector_t /*ast_export_t*/ *exports) {
    assert(parser != NULL);
    assert(exports != NULL);

    int res;

    TRYP(res, soft(accept(parser, '(')));

    TRYP(res,
         maybe(export(parser, (ast_export_t *)vector_alloc_elem(exports))));
    while ((res != TOK_NO_TOK)) {
        TRYP(res, maybe(soft(accept(parser, ','))));
        if (res == TOK_NO_TOK) {
            break;
        }
        TRYP(res, export(parser, (ast_export_t *)vector_alloc_elem(exports)));
    }

    TRYP(res, accept(parser, ')'));

    return res;
}

int export(parser_t *parser, ast_export_t *ex) {
    assert(parser != NULL);
    assert(ex != NULL);

    int res;

    TRYP(res, accept(parser, TOK_VARID));
    ex->exportid = parser_get_text(parser);

    return res;
}

int body(parser_t *parser, ast_t *node) {
    assert(parser != NULL);
    assert(node != NULL);

    int res;
    ast_body_t *body = &node->body;

    TRY(res, vector_init_with_allocator(&body->topdecls, sizeof(ast_t),
                                        parser->allocator));

    TRYP(res, identable(parser, &body->topdecls, declaration));

    node->rule = AST_BODY;
    return res;
}

int declaration(parser_t *parser, ast_t *node) {
    assert(parser != NULL);
    assert(node != NULL);

    int res;

    TRYP(res, soft(accept(parser, TOK_VARID)) | soft(accept(parser, TOK_DATA)));

    switch (res) {
    case TOK_VARID: {
        char *decl_name = parser_get_text(parser);

        TRYP(res, hard(function(parser, decl_name, node) ||
                       value(parser, decl_name, node) ||
                       has_type(parser, decl_name, node)));
        break;
    }
    case TOK_DATA: {
        TRYP(res, hard(data(parser, node)));

        break;
    }
    default:
        PFAIL("Expected declaration");
        return -1;
    }

    return res;
}

int function(parser_t *parser, char *decl_name, ast_t *node) {
    assert(parser != NULL);
    assert(decl_name != NULL);
    assert(node != NULL);

    int res;

    TRYP(res, soft(accept(parser, TOK_VARID)));

    ast_fn_decl_t *fn_decl = &node->fn_decl;

    fn_decl->name = decl_name;
    TRY(res, vector_init_with_allocator(&fn_decl->vars, sizeof(char *),
                                        parser->allocator));

    do {
        void *resp;

        char *var_name = parser_get_text(parser);
        TRYCR(resp, vector_push_back(&fn_decl->vars, &var_name), NULL, -1);
        TRY(res, maybe(soft(accept(parser, TOK_VARID))));
    } while (res != TOK_NO_TOK);

    TRYP(res, accept(parser, '='));

    TRYCR(fn_decl->body, (ast_t *)PALLOC(sizeof(ast_t)), NULL, -1);
    TRYP(res, expression(parser, fn_decl->body));

    TRYP(res, maybe(soft(accept(parser, TOK_WHERE))));
    if (res != TOK_NO_TOK) {
        ast_t *body;
        TRYCR(body, (ast_t *)PALLOC(sizeof(ast_t)), 0, -1);
        memcpy(body, fn_decl->body, sizeof(ast_t));

        ast_let_t *let = &fn_decl->body->let;
        let->body = body;
        TRY(res, vector_init_with_allocator(&let->bindings, sizeof(ast_t),
                                            parser->allocator));

        TRYP(res, bindings(parser, &let->bindings));

        fn_decl->body->rule = AST_LET;
    }

    node->rule = AST_FN_DECL;
    return res;
}

int value(parser_t *parser, char *decl_name, ast_t *node) {
    assert(parser != NULL);
    assert(decl_name != NULL);
    assert(node != NULL);

    int res;

    TRYP(res, soft(accept(parser, '=')));

    ast_val_decl_t *val_decl = &node->val_decl;

    val_decl->name = decl_name;

    TRYCR(val_decl->body, (ast_t *)PALLOC(sizeof(ast_t)), NULL, -1);
    TRYP(res, expression(parser, val_decl->body));

    TRYP(res, maybe(soft(accept(parser, TOK_WHERE))));
    if (res != TOK_NO_TOK) {
        open_indent(parser);

        ast_t *body;
        TRYCR(body, (ast_t *)PALLOC(sizeof(ast_t)), 0, -1);
        memcpy(body, val_decl->body, sizeof(ast_t));

        ast_let_t *let = &val_decl->body->let;
        let->body = body;
        TRY(res, vector_init_with_allocator(&let->bindings, sizeof(ast_t),
                                            parser->allocator));

        TRYP(res, bindings(parser, &let->bindings));

        close_indent(parser);

        val_decl->body->rule = AST_LET;
    }

    node->rule = AST_VAL_DECL;
    return res;
}

int has_type(parser_t *parser, char *decl_name, ast_t *node) {
    assert(parser != NULL);
    assert(decl_name != NULL);
    assert(node != NULL);

    int res;

    TRYP(res, soft(accept(parser, TOK_OP_HASTYPE)));

    ast_has_type_decl_t *has_type_decl = &node->has_type_decl;

    has_type_decl->symbol_name = decl_name;

    TRYCR(has_type_decl->type_exp, (ast_t *)PALLOC(sizeof(ast_t)), NULL, -1);
    TRYP(res, expression(parser, has_type_decl->type_exp));

    node->rule = AST_HAS_TYPE_DECL;
    return res;
}

int data(parser_t *parser, ast_t *node) {
    assert(parser != NULL);
    assert(node != NULL);

    int res;

    ast_data_decl_t *data_decl = &node->data_decl;

    TRYP(res, hard(accept(parser, TOK_CONID)));

    data_decl->data_name = parser_get_text(parser);

    TRYP(res, hard(accept(parser, '=')));

    TRY(res, vector_init_with_allocator(&data_decl->constructors, sizeof(ast_t),
                                        parser->allocator));

    open_indent(parser);

    while (res != TOK_NO_TOK) {
        ast_t *new_node;
        TRYCR(new_node, (ast_t *)vector_alloc_elem(&data_decl->constructors),
              NULL, -1);
        TRYP(res, data_constructor(parser, new_node));
        TRYP(res, maybe(soft(accept(parser, '|'))));
    }

    close_indent(parser);

    res = ')';
    node->rule = AST_DATA_DECL;
    return res;
}

int data_constructor(parser_t *parser, ast_t *node) {
    assert(parser != NULL);
    assert(node != NULL);

    int res;
    void *resptr;

    TRYP(res, hard(accept(parser, TOK_CONID)));

    ast_data_constructor_t *data_constr = &node->data_constructor;

    data_constr->constructor_name = parser_get_text(parser);

    TRY(res, vector_init_with_cap_and_allocator(
                 &data_constr->types, sizeof(ast_t), 0, parser->allocator));

    for (;;) {
        ast_t type_arg;

        TRYP(res, maybe(atype(parser, &type_arg)));

        if (res == TOK_NO_TOK) {
            res = ')';
            break;
        }

        TRYCR(resptr, vector_push_back(&data_constr->types, &type_arg), NULL,
              -1);
    }

    node->rule = AST_DATA_CONSTRUCTOR;
    return res;
}

int bindings(parser_t *parser, vector_t /*ast_t*/ *binds) {
    assert(parser != NULL);
    assert(binds != NULL);

    int res;

    TRYP(res, identable(parser, binds, declaration));

    return res;
}

int expression(parser_t *parser, ast_t *node) {
    assert(parser != NULL);
    assert(node != NULL);

    int res, tmp;

    TRYP(res, let_exp(parser, node) || if_exp(parser, node) ||
                  do_exp(parser, node) || fexpression(parser, node));

    TRYP(tmp, maybe(operator(parser)));
    if (tmp != TOK_NO_TOK) {
        res = tmp;
        ast_t *lhs;
        TRYCR(lhs, (ast_t *)PALLOC(sizeof(ast_t)), NULL, -1);
        memcpy(lhs, node, sizeof(ast_t));

        ast_op_appl_t *op_appl = &node->op_appl;

        op_appl->op_name = parser_get_text(parser);
        op_appl->lhs = lhs;
        TRYCR(op_appl->rhs, (ast_t *)PALLOC(sizeof(ast_t)), NULL, -1);
        TRYP(res, expression(parser, op_appl->rhs));

        node->rule = AST_OP_APPL;
    }

    return res;
}

int let_exp(parser_t *parser, ast_t *node) {
    assert(parser != NULL);
    assert(node != NULL);

    int res;

    TRYP(res, soft(accept(parser, TOK_LET)));

    ast_let_t *let_expr = &node->let;

    TRY(res, vector_init_with_allocator(&let_expr->bindings, sizeof(ast_t),
                                        parser->allocator));

    TRYP(res, bindings(parser, &let_expr->bindings));

    TRYP(res, accept(parser, TOK_IN));

    TRYCR(let_expr->body, (ast_t *)PALLOC(sizeof(ast_t)), NULL, -1);

    TRYP(res, expression(parser, let_expr->body));

    node->rule = AST_LET;

    return res;
}

int if_exp(parser_t *parser, ast_t *node) {
    assert(parser != NULL);
    assert(node != NULL);

    int res;

    TRYP(res, soft(accept(parser, TOK_IF)));

    ast_if_t *if_expr = &node->if_exp;

    TRYCR(if_expr->cond, (ast_t *)PALLOC(sizeof(ast_t)), NULL, -1);
    TRYP(res, expression(parser, if_expr->cond));

    TRYP(res, accept(parser, TOK_THEN));

    TRYCR(if_expr->then_branch, (ast_t *)PALLOC(sizeof(ast_t)), NULL, -1);
    TRYP(res, expression(parser, if_expr->then_branch));

    TRYP(res, accept(parser, TOK_ELSE));

    TRYCR(if_expr->else_branch, (ast_t *)PALLOC(sizeof(ast_t)), NULL, -1);
    TRYP(res, expression(parser, if_expr->else_branch));

    node->rule = AST_IF;
    return res;
}

int do_step(parser_t *parser, ast_t *node) {
    assert(parser != NULL);
    assert(node != NULL);

    int res;

    TRYP(res, do_let_exp(parser, node) || expression(parser, node));

    return res;
}

int do_exp(parser_t *parser, ast_t *node) {
    assert(parser != NULL);
    assert(node != NULL);

    int res;

    TRYP(res, soft(accept(parser, TOK_DO)));

    ast_do_t *do_exp = &node->do_exp;

    TRY(res, vector_init_with_allocator(&do_exp->steps, sizeof(ast_t),
                                        parser->allocator));

    TRYP(res, identable(parser, &do_exp->steps, do_step));

    node->rule = AST_DO;
    return res;
}

int do_let_exp(parser_t *parser, ast_t *node) {
    assert(parser != NULL);
    assert(node != NULL);

    int res;

    TRYP(res, soft(accept(parser, TOK_LET)));

    ast_let_t *let = &node->let;

    TRY(res, vector_init_with_allocator(&let->bindings, sizeof(ast_t),
                                        parser->allocator));
    let->body = NULL;

    TRYP(res, bindings(parser, &let->bindings));

    node->rule = AST_LET;

    return res;
}

int unary_neg(parser_t *parser, ast_t *node) {
    assert(parser != NULL);
    assert(node != NULL);

    int res;

    TRYP(res, soft(accept(parser, '-')));

    ast_neg_t *neg = &node->neg;

    TRYCR(neg->expr, (ast_t *)PALLOC(sizeof(ast_t)), NULL, -1);
    TRYP(res, expression(parser, neg->expr));

    node->rule = AST_NEG;
    return res;
}

int fexpression(parser_t *parser, ast_t *node) {
    assert(parser != NULL);
    assert(node != NULL);

    int res;

    TRYP(res, soft(aexpression(parser, node)));

    ast_t *rhs;
    TRYCR(rhs, (ast_t *)PALLOC(sizeof(ast_t)), NULL, -1);

    TRYP(res, maybe(aexpression(parser, rhs)));
    while (res != TOK_NO_TOK) {
        ast_t *lhs;
        TRYCR(lhs, (ast_t *)PALLOC(sizeof(ast_t)), NULL, -1);
        memcpy(lhs, node, sizeof(ast_t));

        ast_fn_appl_t *fn_appl = &node->fn_appl;
        fn_appl->fn = lhs;
        fn_appl->arg = rhs;

        node->rule = AST_FN_APPL;
        TRYCR(rhs, (ast_t *)PALLOC(sizeof(ast_t)), NULL, -1);

        TRYP(res, maybe(aexpression(parser, rhs)));
    }

    PFREE(rhs);

    return res;
}

int aexpression(parser_t *parser, ast_t *node) {
    assert(parser != NULL);
    assert(node != NULL);

    int res;

    TRYP(res, maybe(soft(accept(parser, '('))));

    if (res != TOK_NO_TOK) {
        no_indent(parser);

        TRYP(res, expression(parser, node));
        TRYP(res, accept(parser, ')'));

        close_indent(parser);
    } else {
        TRYP(res, var(parser, node) || con(parser, node) || lit(parser, node));
    }

    return res;
}

int var(parser_t *parser, ast_t *node) {
    assert(parser != NULL);
    assert(node != NULL);

    int res;

    TRYP(res, soft(accept(parser, TOK_VARID)));
    node->var.name = parser_get_text(parser);

    node->rule = AST_VAR;
    return res;
}

int con(parser_t *parser, ast_t *node) {
    assert(parser != NULL);
    assert(node != NULL);

    int res;

    TRYP(res, maybe(soft(accept(parser, TOK_UNIT))));

    if (res != TOK_NO_TOK) {
        node->con.name = PSTRALLOC("()");
        node->rule = AST_CON;
        return res;
    }

    TRYP(res, soft(accept(parser, TOK_CONID)));
    node->con.name = parser_get_text(parser);

    node->rule = AST_CON;
    return res;
}

int atype(parser_t *parser, ast_t *node) {
    assert(parser != NULL);
    assert(node != NULL);

    int res;
    void *resptr;

    TRYP(res, soft(accept(parser, TOK_VARID)) |     // tyvar
                  soft(accept(parser, TOK_CONID)) | // type con
                  soft(accept(parser, '(')) |       // tuple
                  soft(accept(parser, '['))         // list
    );

    switch (res) {
    case TOK_VARID: {
        ast_typevar_t *typevar = &node->typevar;
        typevar->tyvar = parser_get_text(parser);
        node->rule = AST_TYPEVAR;
        break;
    }
    case TOK_CONID: {
        ast_type_t *type_node = &node->type;
        type_node->tycon = parser_get_text(parser);

        TRY(res, vector_init_with_cap_and_allocator(
                     &type_node->args, sizeof(ast_t), 0, parser->allocator));
        res = ')';

        node->rule = AST_TYPE;
        break;
    }
    case '[': {
        ast_type_t *type_node = &node->type;
        TRYCR(type_node->tycon, PSTRALLOC("[]"), NULL, -1);

        TRY(res, vector_init_with_cap_and_allocator(
                     &type_node->args, sizeof(ast_t), 1, parser->allocator));

        ast_t *elem_type;
        TRYCR(elem_type, vector_alloc_elem(&type_node->args), NULL, -1);
        TRYP(res, type(parser, elem_type));

        TRYP(res, accept(parser, ']'));
        node->rule = AST_TYPE;
        break;
    }
    case '(': {
        ast_type_t *type_node = &node->type;
        ast_t first_type;

        TRYP(res, type(parser, &first_type));

        TRYP(res, maybe(accept(parser, ',')));
        if (res == ',') {
            TRYCR(type_node->tycon, PSTRALLOC("()"), NULL, -1);
            TRY(res, vector_init_with_allocator(&type_node->args, sizeof(ast_t),
                                                parser->allocator));
            TRYCR(resptr, vector_push_back(&type_node->args, &first_type), NULL,
                  -1);

            do {
                ast_t *next_type;
                TRYCR(next_type, vector_alloc_elem(&type_node->args), NULL, -1);

                TRYP(res, type(parser, next_type));
                TRYP(res, maybe(accept(parser, ',')));
            } while (res != TOK_NO_TOK);
        } else {
            memcpy(node, &first_type, sizeof(ast_t));
        }

        TRYP(res, accept(parser, ')'));
        node->rule = AST_TYPE;
        break;
    }
    }

    return res;
}

int btype(parser_t *parser, ast_t *node) {
    assert(parser != NULL);
    assert(node != NULL);

    int res;
    void *resptr;

    TRYP(res, atype(parser, node));

    ast_type_t *type_node = &node->type;

    for (;;) {
        ast_t type_arg;

        TRYP(res, maybe(atype(parser, &type_arg)));

        if (res == TOK_NO_TOK) {
            res = ')';
            break;
        }

        TRYCR(resptr, vector_push_back(&type_node->args, &type_arg), NULL, -1);
    }

    return res;
}

int type(parser_t *parser, ast_t *node) {
    assert(parser != NULL);
    assert(node != NULL);

    int res;
    void *resptr;

    TRYP(res, btype(parser, node));

    TRYP(res, maybe(accept(parser, TOK_OP_L_ARROW))); // ->

    if (res != TOK_NO_TOK) {
        ast_t lhs;

        memcpy(&lhs, node, sizeof(ast_t));

        ast_type_t *node_type = &node->type;

        TRYCR(node_type->tycon, PSTRALLOC("->"), NULL, -1);
        TRY(res, vector_init_with_cap_and_allocator(
                     &node_type->args, sizeof(ast_t), 2, parser->allocator));
        TRYCR(resptr, vector_push_back(&node_type->args, &lhs), NULL, -1);

        ast_t *rhs;
        TRYCR(rhs, vector_alloc_elem(&node_type->args), NULL, -1);

        TRYP(res, type(parser, rhs));
    } else {
        res = ')';
    }

    return res;
}

int lit(parser_t *parser, ast_t *node) {
    assert(parser != NULL);
    assert(node != NULL);

    int res;

    TRYP(res, number(parser, node) || string(parser, node));

    return res;
}

int number(parser_t *parser, ast_t *node) {
    int res;

    TRYP(res, soft(accept(parser, TOK_NUMBER)));
    char *number_str = parser_get_text(parser);

    ast_lit_t *lit = &node->lit;
    lit->lit_type = AST_LIT_TYPE_INT;
    lit->int_lit = atoi(number_str);

    PFREE(number_str);

    node->rule = AST_LIT;
    return res;
}

int string(parser_t *parser, ast_t *node) {
    int res;

    TRYP(res, soft(accept(parser, TOK_STRING)));
    char *str = parser_get_text(parser);

    ast_lit_t *lit = &node->lit;
    lit->lit_type = AST_LIT_TYPE_STR;
    lit->str_lit = str;

    node->rule = AST_LIT;
    return res;
}
