#ifndef SCHC_PARSER_H_
#define SCHC_PARSER_H_

#include "ast.h"

typedef struct parser_ {
    int token;
    char *ptext;
} parser_t;

int parser_init(parser_t *parser);
void parser_destroy(parser_t *parser);

int parser_parse(parser_t *parser, ast_t *root);

#endif/*SCHC_PARSER_H_*/