%{
#import <stdio.h>

#import "../src/lexer.h"

int yycolumn = 0;

#define YY_USER_ACTION { yycolumn += yyleng; }
%}
%option noyywrap
%option yylineno

VARID [a-z][A-Za-z0-9']*
CONID [A-Z][A-Za-z0-9']*
NUMBER ([0-9]+)
OP [!#$%&*+./<=>?@\\\^|-]*
LINE_COMMENT --.*

%x BLOCK_COMMENT

%%

"{-"                { BEGIN(BLOCK_COMMENT); }
<BLOCK_COMMENT>"-}" { BEGIN(INITIAL); }
<BLOCK_COMMENT>\n|.

{LINE_COMMENT}
[ \t]*

\n|\r\n|\n\r  { yycolumn = 0; return TOK_NEWLINE; }

"case"		{ return TOK_CASE; }
"class"		{ return TOK_CLASS; }
"data"		{ return TOK_DATA; }
"default"	{ return TOK_DEFAULT; }
"deriving"	{ return TOK_DERIVING; }
"do"		{ return TOK_DO; }
"else"		{ return TOK_ELSE; }
"foreign"	{ return TOK_FOREIGN; }
"if"		{ return TOK_IF; }
"import"	{ return TOK_IMPORT; }
"in"		{ return TOK_IN; }
"infix"		{ return TOK_INFIX; }
"infixl"	{ return TOK_INFIXL; }
"infixr"	{ return TOK_INFIXR; }
"instance"	{ return TOK_INSTANCE; }
"let"		{ return TOK_LET; }
"module"	{ return TOK_MODULE; }
"newtype"	{ return TOK_NEWTYPE; }
"of"		{ return TOK_OF; }
"then"		{ return TOK_THEN; }
"type"		{ return TOK_TYPE; }
"where"		{ return TOK_WHERE; }
".."		{ return TOK_RANGE; }
"::"		{ return TOK_HASTYPE; }
"<-"		{ return TOK_L_ARROW; }
"->"		{ return TOK_R_ARROW; }
"=>"		{ return TOK_R_FAT_ARROW; }

"("         { return '('; }
")"         { return ')'; }
","         { return ','; }
";"         { return ';'; }
"["         { return '['; }
"]"         { return ']'; }
"`"         { return '`'; }
"{"         { return '{'; }
"}"         { return '}'; }
"_"         { return '_'; }
"="         { return '='; }
"\\"        { return '\\'; }
"|"         { return '|'; }
"@"         { return '@'; }
"~"         { return '~'; }
"."         { return '.'; }

{VARID} {
    return TOK_VARID;
}

{CONID} {
    return TOK_CONID;
}

{NUMBER} {
    return TOK_NUMBER;
}

{OP} {
    return TOK_OP;
}

. {
    fprintf(stderr, "Parse error(%d, %d): '%s' not recognized", yylineno, yycolumn, yytext);

    return TOK_ERROR;
}

%%
