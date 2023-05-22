
#include <ast.h>
#include <env.h>
#include <lexer.h>
#include <parser.h>

#include <core.h>
#include <coregen.h>
#include <intrinsics/intrinsics.h>

#include <util.h>

typedef size_t yy_size_t;
typedef struct yy_buffer_state *YY_BUFFER_STATE;
extern YY_BUFFER_STATE yy_scan_string(char *str);

char PROGRAM[] = "              \n\
main = putStrLn (show (f 3))    \n\
                                \n\
f x = x * 2                     \n\
                                \n\
                                \n\
";

int main() {
    parser_t parser;

    yy_scan_string(PROGRAM);
    parser_init(&parser);

    allocator_t parser_allocator;
    parser_allocator = default_allocator;
    /*
    linalloc_t parser_linalloc;
    linalloc_init(&parser_linalloc);
    linalloc_allocator(&parser_linalloc, &parser_allocator);
    */

    ast_t ast;
    parser_parse(&parser, &ast, &parser_allocator);

    ast_print(&ast, stdout);

    yylex_destroy();

    allocator_t core_allocator;
    core_allocator = default_allocator;
    /*
    linalloc_t linalloc;
    linalloc_init(&linalloc);
    linalloc_allocator(&linalloc, &core_allocator);
    */

    env_t env, intrinsics_env;
    env_init_with_allocator(&env, &core_allocator);
    env_init_with_allocator(&intrinsics_env, &core_allocator);

    intrinsics_load(&intrinsics_env);
    env.upper_scope = &intrinsics_env;

    coregen_from_module_ast(&ast, &env);

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
    ast_destroy(&ast, &parser_allocator);
    vector_destroy(&module_scope);
    // linalloc_destroy(&parser_linalloc);
    // linalloc_destroy(&linalloc);

    return 0;
}
