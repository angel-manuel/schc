#ifndef SCHC_LEXER_H_
#define SCHC_LEXER_H_

#include <stdio.h>

extern int yylex();
extern int yylex_destroy();
extern char *yytext;
extern FILE *yyin;
extern FILE *yyout;
extern int yylineno;
extern int yycolumn;

typedef enum token_ {
    TOK_ERROR = -1,
    TOK_NO_TOK = 128,
    TOK_ANY,
    TOK_VARID,
    TOK_CONID,
    TOK_NUMBER,
    TOK_OP,

    TOK_OP_RANGE,
    TOK_OP_HASTYPE,
    TOK_OP_L_ARROW,
    TOK_OP_R_ARROW,
    TOK_OP_R_FAT_ARROW,

    TOK_CASE,
    TOK_CLASS,
    TOK_DATA,
    TOK_DEFAULT,
    TOK_DERIVING,
    TOK_DO,
    TOK_ELSE,
    TOK_FOREIGN,
    TOK_IF,
    TOK_IMPORT,
    TOK_IN,
    TOK_INFIX,
    TOK_INFIXL,
    TOK_INFIXR,
    TOK_INSTANCE,
    TOK_LET,
    TOK_MODULE,
    TOK_NEWTYPE,
    TOK_OF,
    TOK_THEN,
    TOK_TYPE,
    TOK_WHERE,
    TOK_UNIT,
} token_t;

const char *strtoken(int token);

#endif /*SCHC_LEXER_H_*/
