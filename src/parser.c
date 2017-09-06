#include "parser.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "lexer.h"
#include "util.h"

#define TRYP(res, exp) TRYCR((res), (exp), 0, 0)

int accept(parser_t *parser, int token);
int maybe(int res);

int module(parser_t *parser, ast_t *node);
    int declaration(parser_t *parser, ast_t *node);
        int function(parser_t *parser, char *decl_name, ast_t *node);
        int value(parser_t *parser, char *decl_name, ast_t *node);
    int expression(parser_t *parser, ast_t *node);
        int if_exp(parser_t *parser, ast_t *node);
        // int fn_appl_or_var(parser_t *parser, ast_t *node);
        int lit(parser_t *parser, ast_t *node);

int parser_init(parser_t *parser) {
    assert(parser != NULL);

    return 0;
}

void parser_destroy(parser_t *parser) {
    assert(parser != NULL);
}

int parser_parse(parser_t *parser, ast_t *root) {
    assert(parser != NULL);

    parser->token = yylex();
    TRYCR(parser->ptext, stralloc(yytext), NULL, 0);

    int res = module(parser, root);

    if (!res) {
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

int accept(parser_t *parser, int token) {
    assert(parser != NULL);
    assert(token != 0);
    
    if (parser->token == token) {
        if (parser->ptext != NULL) {
            free(parser->ptext);
            parser->ptext = NULL;
        }

        printf("%-20s%s\n", strtoken(token), yytext);
        TRYCR(parser->ptext, stralloc(yytext), NULL, 0);
        parser->token = yylex();
        return token;
    }

    return 0;
}

int maybe(int res) {
    return res == 0 ? TOK_NO_TOK : res;
}

int module(parser_t *parser, ast_t *node) {
    assert(parser != NULL);
    assert(node != NULL);

    int res;
    ast_module_t *module = &node->module;

    vector_init(&module->decls, sizeof(ast_t));

    ast_t decl;
    decl.rule = AST_NO_RULE;

    while (parser->token) {
        TRYP(res, accept(parser, TOK_NEWLINE) || declaration(parser, &decl));
        if (decl.rule != AST_NO_RULE) {
            vector_push_back(&module->decls, &decl);
            decl.rule = AST_NO_RULE;
        }
    }

    node->rule = AST_MODULE;
    return 1;
}

int declaration(parser_t *parser, ast_t *node) {
    assert(parser != NULL);
    assert(node != NULL);

    int res;

    TRYP(res, accept(parser, TOK_VARID));

    char *decl_name = parser_get_text(parser);

    TRYP(res, function(parser, decl_name, node) || value(parser, decl_name, node));

    return res;
}

int function(parser_t *parser, char *decl_name, ast_t *node) {
    assert(parser != NULL);
    assert(decl_name != NULL);
    assert(node != NULL);

    int res;

    TRYP(res, accept(parser, TOK_VARID));

    ast_fn_decl_t *fn_decl = &node->fn_decl;
    
    fn_decl->name = decl_name;
    TRY(res, vector_init(&fn_decl->vars, sizeof(char*)));

    do {
        char *var_name = parser_get_text(parser);
        vector_push_back(&fn_decl->vars, &var_name);
    } while (accept(parser, TOK_VARID));

    TRYP(res, accept(parser, '='));

    TRYCR(fn_decl->body, (ast_t*)malloc(sizeof(ast_t)), NULL, 0);
    TRYP(res, expression(parser, fn_decl->body));

    node->rule = AST_FN_DECL;
    return res;
}

int value(parser_t *parser, char *decl_name, ast_t *node) {
    assert(parser != NULL);
    assert(decl_name != NULL);
    assert(node != NULL);

    int res;
    
    TRYP(res, accept(parser, '='));

    ast_val_decl_t *val_decl = &node->val_decl; 
    
    val_decl->name = decl_name;

    TRYCR(val_decl->body, (ast_t*)malloc(sizeof(ast_t)), NULL, 0);
    TRYP(res, expression(parser, val_decl->body));

    node->rule = AST_VAL_DECL;
    return res;
}

int expression(parser_t *parser, ast_t *node) {
    assert(parser != NULL);
    assert(node != NULL);

    int res;

    if (accept(parser, '(')) {
        TRYP(res, expression(parser, node));
        TRYP(res, accept(parser, ')'));
    } else {
        TRYP(res, if_exp(parser, node)
                // || fn_appl_or_var(parser, node)
                || lit(parser, node));
    }

    return res;
}

int if_exp(parser_t *parser, ast_t *node) {
    assert(parser != NULL);
    assert(node != NULL);

    int res;

    TRYP(res, accept(parser, TOK_IF));

    ast_if_t *if_expr = &node->if_exp; 

    TRYCR(if_expr->cond, (ast_t*)malloc(sizeof(ast_t)), NULL, 0);
    TRYP(res, expression(parser, if_expr->cond));
    
    TRYP(res, accept(parser, TOK_THEN));

    TRYCR(if_expr->then_branch, (ast_t*)malloc(sizeof(ast_t)), NULL, 0);
    TRYP(res, expression(parser, if_expr->then_branch));
    
    TRYP(res, accept(parser, TOK_ELSE));
    
    TRYCR(if_expr->else_branch, (ast_t*)malloc(sizeof(ast_t)), NULL, 0);
    TRYP(res, expression(parser, if_expr->else_branch));
    
    node->rule = AST_IF;
    return res;
}

// int fn_appl_or_var(parser_t *parser, ast_t *node) {
//     assert(parser != NULL);
//     assert(node != NULL);

//     int res;

//     TRYP(res, accept(parser, TOK_VARID));

//     char *name = parser_get_text(parser);

//     return res;
// }

int lit(parser_t *parser, ast_t *node) {
    assert(parser != NULL);
    assert(node != NULL);

    int res;

    TRYP(res, accept(parser, TOK_NUMBER));
    char *number_str = parser_get_text(parser);
    
    ast_lit_t *lit = &node->lit;
    lit->int_lit = atoi(number_str);

    free(number_str);

    node->rule = AST_LIT;
    return res;
}
