#include "parser.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "lexer.h"
#include "util.h"

#define PFAIL(fail) fprintf(stderr, "Parser FAIL: %s\n", fail);

#define TRYP(res, exp)      \
do {                        \
(res) = (exp);              \
if ((res) == -1) {          \
    PFAIL(#exp);            \
    return -1;              \
} else if ((res) == 0) {    \
    return 0;               \
}                           \
} while (0);

int accept_anywhere(parser_t *parser, int token);
int accept(parser_t *parser, int token);
int accept_exact_indent(parser_t *parser, int token);
int maybe(int res);
int soft(int res);
int hard(int res);

int operator(parser_t *parser);

int root(parser_t *parser, ast_t *node);
int module(parser_t *parser, ast_t *node);
    int exports(parser_t *parser, vector_t/*ast_export_t*/ *exports);
        int export(parser_t *parser, ast_export_t *ex);
    int body(parser_t *parser, ast_t *node);
        int declaration(parser_t *parser, ast_t *node);
            int function(parser_t *parser, char *decl_name, ast_t *node);
            int value(parser_t *parser, char *decl_name, ast_t *node);
            int has_type(parser_t *parser, char *decl_name, ast_t *node);

int expression(parser_t *parser, ast_t *node);
    int unary_neg(parser_t *parser, ast_t *node);
    int fexpression(parser_t *parser, ast_t *node);
        int aexpression(parser_t *parser, ast_t *node);
            int if_exp(parser_t *parser, ast_t *node);
            int var(parser_t *parser, ast_t *node);
            int con(parser_t *parser, ast_t *node);
            int lit(parser_t *parser, ast_t *node);
    int bindings(parser_t *parser, vector_t *binds);


void no_indent(parser_t *parser);
void open_indent(parser_t *parser);
void close_indent(parser_t *parser);

int parser_init(parser_t *parser) {
    assert(parser != NULL);

    int res;

    parser->token = -1;
    parser->ptext = NULL;
    parser->new_indent_accepted = 0;
    TRY(res, stack_init(&parser->indent_stack, sizeof(int)));

    return 0;
}

void parser_destroy(parser_t *parser) {
    assert(parser != NULL);

    if (parser->ptext != NULL) {
        free(parser->ptext);
    }

    stack_destroy(&parser->indent_stack);
}

int parser_parse(parser_t *parser, ast_t *root) {
    assert(parser != NULL);

    parser->token = yylex();
    TRYCR(parser->ptext, stralloc(yytext), NULL, -1);

    int res = module(parser, root);

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

int accept_anywhere(parser_t *parser, int token) {
    assert(parser != NULL);
    assert(token != 0);
    
    if (parser->token == token) {
        if (parser->ptext != NULL) {
            free(parser->ptext);
            parser->ptext = NULL;
        }

        printf("%-20s%s\n", strtoken(token), yytext);
        TRYCR(parser->ptext, stralloc(yytext), NULL, -1);
        parser->token = yylex();
        return token;
    }

    return -1;
}

int accept(parser_t *parser, int token) {
    assert(parser != NULL);

    const int *stack_ind = stack_peek(&parser->indent_stack);
    int indent = yycolumn - strlen(yytext);

    if (stack_ind == NULL || indent > *stack_ind) {
        return accept_anywhere(parser, token);
    }

    return -1;
}

int accept_exact_indent(parser_t *parser, int token) {
    assert(parser != NULL);

    int res;
    const int *stack_ind = stack_peek(&parser->indent_stack);
    int indent = yycolumn - strlen(yytext);

    if (parser->new_indent_accepted || indent == *stack_ind) {

        TRYP(res, accept_anywhere(parser, token));

        if (parser->new_indent_accepted) {
            printf("Indent: %d\n", indent);
            stack_push(&parser->indent_stack, &indent);
            parser->new_indent_accepted = 0;
        }

        return res;
    }

    return -1;
}

int maybe(int res) {
    return res == 0 ? TOK_NO_TOK : res;
}

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

    parser->new_indent_accepted = 1;
}

void close_indent(parser_t *parser) {
    assert(parser != NULL);

    stack_pop(&parser->indent_stack, NULL);

    int *curr_indent = (int*)stack_peek(&parser->indent_stack);
    if (curr_indent == NULL) {
        printf("Indent: NULL\n");
    } else {
        printf("Indent: %d\n", *curr_indent);
    }
}

// Rules

int operator(parser_t *parser) {
    return 
        soft(accept(parser, TOK_OP)) ||
        soft(accept(parser, '-')) ||
        soft(accept(parser, ':')) ||
        soft(accept(parser, '=')) ||
        soft(accept(parser, '\\')) ||
        soft(accept(parser, '|')) ||
        soft(accept(parser, '@')) ||
        soft(accept(parser, TOK_OP_RANGE)) ||
        soft(accept(parser, TOK_OP_HASTYPE)) ||
        soft(accept(parser, TOK_OP_L_ARROW)) ||
        soft(accept(parser, TOK_OP_R_ARROW)) ||
        soft(accept(parser, TOK_OP_R_FAT_ARROW))
        ;
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
    TRYCR(res, vector_init(&module->exports, sizeof(ast_export_t)), -1, 0);

    TRYP(res, maybe(soft(accept(parser, TOK_MODULE))));

    if(res != TOK_NO_TOK) {
        TRYP(res, accept(parser, TOK_CONID));
        module->modid = parser_get_text(parser);

        TRYP(res, exports(parser, &module->exports));

        TRYP(res, accept(parser, TOK_WHERE));

    }

    open_indent(parser);

    TRYCR(module->body, (ast_t*)malloc(sizeof(ast_t)), NULL, -1);
    TRYP(res, body(parser, module->body));
    
    close_indent(parser);

    node->rule = AST_MODULE;
    return res;
}

int exports(parser_t *parser, vector_t/*ast_export_t*/ *exports) {
    assert(parser != NULL);
    assert(exports != NULL);

    int res;

    TRYP(res, soft(accept(parser, '(')));

    TRYP(res, maybe(export(parser, (ast_export_t*)vector_alloc_elem(exports))));
    while((res != TOK_NO_TOK)) {
        TRYP(res, maybe(soft(accept(parser, ','))));
        if(res == TOK_NO_TOK) {
            break;
        }
        TRYP(res, export(parser, (ast_export_t*)vector_alloc_elem(exports)));
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

    TRY(res, vector_init(&body->topdecls, sizeof(ast_t)));

    do {
        TRYP(res, maybe(declaration(parser, (ast_t*)vector_alloc_elem(&body->topdecls))));
    } while (res != TOK_NO_TOK);

    body->topdecls.len--;

    node->rule = AST_BODY;
    return res;
}

int declaration(parser_t *parser, ast_t *node) {
    assert(parser != NULL);
    assert(node != NULL);

    int res;

    TRYP(res, soft(accept_exact_indent(parser, TOK_VARID)));

    char *decl_name = parser_get_text(parser);

    TRYP(res, hard(
            function(parser, decl_name, node) ||
            value(parser, decl_name, node) ||
            has_type(parser, decl_name, node)));
    
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
    TRY(res, vector_init(&fn_decl->vars, sizeof(char*)));

    do {
        char *var_name = parser_get_text(parser);
        vector_push_back(&fn_decl->vars, &var_name);
        TRY(res, maybe(soft(accept(parser, TOK_VARID))));
    } while (res != TOK_NO_TOK);

    TRYP(res, accept(parser, '='));

    TRYCR(fn_decl->body, (ast_t*)malloc(sizeof(ast_t)), NULL, -1);
    TRYP(res, expression(parser, fn_decl->body));
   
    TRYP(res, maybe(soft(accept(parser, TOK_WHERE))));
    if (res != TOK_NO_TOK) {
        open_indent(parser);

        ast_t *body;
        TRYCR(body, (ast_t*)malloc(sizeof(ast_t)), 0, -1);
        memcpy(body, fn_decl->body, sizeof(ast_t));

        ast_let_t *let = &fn_decl->body->let;
        let->body = body;
        TRY(res, vector_init(&let->bindings, sizeof(ast_t)));

        TRYP(res, bindings(parser, &let->bindings));
        
        close_indent(parser);

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

    TRYCR(val_decl->body, (ast_t*)malloc(sizeof(ast_t)), NULL, -1);
    TRYP(res, expression(parser, val_decl->body));
    
    TRYP(res, maybe(soft(accept(parser, TOK_WHERE))));
    if (res != TOK_NO_TOK) {
        open_indent(parser);

        ast_t *body;
        TRYCR(body, (ast_t*)malloc(sizeof(ast_t)), 0, -1);
        memcpy(body, val_decl->body, sizeof(ast_t));

        ast_let_t *let = &val_decl->body->let;
        let->body = body;
        TRY(res, vector_init(&let->bindings, sizeof(ast_t)));

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

    TRYCR(has_type_decl->type_exp, (ast_t*)malloc(sizeof(ast_t)), NULL, -1);
    TRYP(res, expression(parser, has_type_decl->type_exp));

    node->rule = AST_HAS_TYPE_DECL;
    return res;
}

int bindings(parser_t *parser, vector_t/*ast_t*/ *binds) {
    assert(parser != NULL);
    assert(binds != NULL);

    int res;
    ast_t *decl;

    do {
        TRYCR(decl, (ast_t*)vector_alloc_elem(binds), NULL, -1);
        TRYP(res, maybe(declaration(parser, decl)));
    } while (res != TOK_NO_TOK);

    binds->len--;

    if (binds->len == 0) {
        return -1;
    }

    return res;
}
    
int expression(parser_t *parser, ast_t *node) {
    assert(parser != NULL);
    assert(node != NULL);

    int res;

    TRYP(res, 
            if_exp(parser, node) ||
            fexpression(parser, node));
    
    TRYP(res, maybe(operator(parser)));
    if (res != TOK_NO_TOK) {
        ast_t *lhs;
        TRYCR(lhs, (ast_t*)malloc(sizeof(ast_t)), NULL, -1);
        memcpy(lhs, node, sizeof(ast_t));
        
        ast_op_appl_t *op_appl = &node->op_appl;

        op_appl->op_name = parser_get_text(parser);
        op_appl->lhs = lhs;
        TRYCR(op_appl->rhs, (ast_t*)malloc(sizeof(ast_t)), NULL, -1);
        TRYP(res, expression(parser, op_appl->rhs));

        node->rule = AST_OP_APPL;
    }

    return res;
}

int if_exp(parser_t *parser, ast_t *node) {
    assert(parser != NULL);
    assert(node != NULL);

    int res;

    TRYP(res, soft(accept(parser, TOK_IF)));

    ast_if_t *if_expr = &node->if_exp;

    TRYCR(if_expr->cond, (ast_t*)malloc(sizeof(ast_t)), NULL, -1);
    TRYP(res, expression(parser, if_expr->cond));

    TRYP(res, accept_anywhere(parser, TOK_THEN));

    TRYCR(if_expr->then_branch, (ast_t*)malloc(sizeof(ast_t)), NULL, -1);
    TRYP(res, expression(parser, if_expr->then_branch));
    
    TRYP(res, accept_anywhere(parser, TOK_ELSE));
    
    TRYCR(if_expr->else_branch, (ast_t*)malloc(sizeof(ast_t)), NULL, -1);
    TRYP(res, expression(parser, if_expr->else_branch));
    
    node->rule = AST_IF;
    return res;
}

int unary_neg(parser_t *parser, ast_t *node) {
    assert(parser != NULL);
    assert(node != NULL);

    int res;

    TRYP(res, soft(accept(parser, '-')));
    
    ast_neg_t *neg = &node->neg;

    TRYCR(neg->expr, (ast_t*)malloc(sizeof(ast_t)), NULL, -1);
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
    TRYCR(rhs, (ast_t*)malloc(sizeof(ast_t)), NULL, -1);

    TRYP(res, maybe(aexpression(parser, rhs)));
    while (res != TOK_NO_TOK) {
        ast_t *lhs;
        TRYCR(lhs, (ast_t*)malloc(sizeof(ast_t)), NULL, -1);
        memcpy(lhs, node, sizeof(ast_t));

        ast_fn_appl_t *fn_appl = &node->fn_appl;
        fn_appl->fn = lhs;
        fn_appl->arg = rhs;

        node->rule = AST_FN_APPL;
        TRYCR(rhs, (ast_t*)malloc(sizeof(ast_t)), NULL, -1);
    
        TRYP(res, maybe(aexpression(parser, rhs)));
    }

    free(rhs);

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
        TRYP(res,
                var(parser, node) ||
                con(parser, node) ||
                lit(parser, node));
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
        node->con.name = stralloc("()");
        node->rule = AST_CON;
        return res;
    }

    TRYP(res, soft(accept(parser, TOK_CONID)));
    node->con.name = parser_get_text(parser);

    node->rule = AST_CON;
    return res;
}

int lit(parser_t *parser, ast_t *node) {
    assert(parser != NULL);
    assert(node != NULL);

    int res;

    TRYP(res, soft(accept(parser, TOK_NUMBER)));
    char *number_str = parser_get_text(parser);
    
    ast_lit_t *lit = &node->lit;
    lit->int_lit = atoi(number_str);

    free(number_str);

    node->rule = AST_LIT;
    return res;
}
