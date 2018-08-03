
#include <ast.h>
#include <lexer.h>
#include <parser.h>

#include <core.h>

typedef size_t yy_size_t;
typedef struct yy_buffer_state *YY_BUFFER_STATE;
extern YY_BUFFER_STATE yy_scan_string(char *str);

char PROGRAM[] = "              \n\
main = do                       \n\
    putStrLn (show 2)           \n\
";

int main() {
    parser_t parser;

    yy_scan_string(PROGRAM);
    parser_init(&parser);

    ast_t ast;

    parser_parse(&parser, &ast);

    parser_destroy(&parser);
    yylex_destroy();

    ast_print(&ast, stdout);

    env_t env;
    env_init(&env);

    core_expr_t program, putStrLn, show2, show, lit2;
    program.form = CORE_APPL;
    program.appl.fn = &putStrLn;
    program.appl.arg = &show2;

    putStrLn.form = CORE_INTRINSIC;
    putStrLn.intrinsic.name = "putStrLn";

    show2.form = CORE_APPL;
    show2.appl.fn = &show;
    show2.appl.arg = &lit2;

    show.form = CORE_INTRINSIC;
    show.intrinsic.name = "show";

    lit2.form = CORE_LITERAL;
    lit2.literal.type = CORE_LITERAL_I64;
    lit2.literal.i64 = 2;

    core_from_ast(&ast, &env, &program);
    core_print(&env, &program, stdout);

    // core_destroy(&program);
    env_destroy(&env);
    ast_destroy(&ast);

    return 0;
}
