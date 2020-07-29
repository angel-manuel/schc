
#include <ast.h>
#include <env.h>
#include <lexer.h>
#include <parser.h>

#include <core.h>
#include <coregen.h>
#include <intrinsics/intrinsics.h>

typedef size_t yy_size_t;
typedef struct yy_buffer_state *YY_BUFFER_STATE;
extern YY_BUFFER_STATE yy_scan_string(char *str);

char PROGRAM[] = "              \n\
main = putStrLn (show (f 3))    \n\
                                \n\
f x = x * 2                     \n\
";

int main() {
    parser_t parser;

    yy_scan_string(PROGRAM);
    parser_init(&parser);

    ast_t ast;

    parser_parse(&parser, &ast);

    ast_print(&ast, stdout);

    yylex_destroy();

    linalloc_t linalloc;
    linalloc_init(&linalloc);

    env_t env, intrinsics_env;
    env_init(&env);
    env_init(&intrinsics_env);

    intrinsics_load(&intrinsics_env, &linalloc);
    env.upper_scope = &intrinsics_env;

    coregen_from_module_ast(&ast, &env, &linalloc);

    parser_destroy(&parser);

    vector_t /* const char * */ module_scope;
    vector_init(&module_scope, sizeof(const char *));

    env_list_scope(&env, &module_scope, 0);

    for (size_t i = 0; i < module_scope.len; ++i) {
        const char *varname = *(const char **)vector_get_ref(&module_scope, i);

        const core_expr_t *expr = env_get_expr(&env, varname);

        printf("var %s\n", varname);
        core_print(expr, stdout);
    }

    env_destroy(&env);
    ast_destroy(&ast);
    vector_destroy(&module_scope);
    linalloc_destroy(&linalloc);

    return 0;
}
