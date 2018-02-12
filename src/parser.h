#ifndef SCHC_PARSER_H_
#define SCHC_PARSER_H_

#include "ast.h"
#include "stack.h"

typedef struct parser_ {
    int token;
    char *ptext;
    int new_indent_accepted;
    stack_t/*int*/ indent_stack;
    int lock_next_line;
    int line;
} parser_t;

int parser_init(parser_t *parser);
void parser_destroy(parser_t *parser);

int parser_parse(parser_t *parser, ast_t *root);

#endif/*SCHC_PARSER_H_*/