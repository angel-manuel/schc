#ifndef SCHC_PARSER_H_
#define SCHC_PARSER_H_

#include "ast.h"
#include "data/allocator.h"
#include "data/stack.h"
#include "lexer.h"

typedef enum {
    PARSER_NONE = 0,
    PARSER_NEW_INDENT_ACCEPTED = 1,
    PARSER_CONTINUE_INDENT = 2,
} parser_flags_t;

typedef struct parser_ {
    token_t token;
    char *ptext;
    parser_flags_t flags;
    stack_t /*int*/ indent_stack;
    allocator_t *allocator;
} parser_t;

int parser_init(parser_t *parser);
void parser_destroy(parser_t *parser);

int parser_parse(parser_t *parser, ast_t *root, allocator_t *allocator);

#endif /*SCHC_PARSER_H_*/