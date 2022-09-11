#include <errno.h>
#include <stdio.h>

#include "ast.h"
#include "core.h"
#include "coregen.h"
#include "data/hashmap.h"
#include "data/linalloc.h"
#include "intrinsics/intrinsics.h"
#include "lexer.h"
#include "parser.h"

void usage();

int main(int argc, char *argv[]) {
    puts("Simple C Haskell Compiler");

    if (argc < 2) {
        usage(argv[0]);
        return 1;
    }

    const char *input_filename = argv[1];

    FILE *input = fopen(input_filename, "r");
    if (input == NULL) {
        fprintf(stderr, "Could not open file '%s': ", input_filename);
        perror("");
        return 1;
    }

    yyin = input;
    yyout = stdout;

    // int token;
    // while((token = yylex()) != 0) {
    //   printf("%-20s %s\n", strtoken(token), yytext);
    // }

    allocator_t parser_allocator;
    linalloc_t parser_linalloc;
    linalloc_init(&parser_linalloc);
    linalloc_allocator(&parser_linalloc, &parser_allocator);

    parser_t parser;
    parser_init(&parser);

    ast_t ast;

    if (parser_parse(&parser, &ast, &parser_allocator) == -1) {
        fprintf(stderr, "Parse error(%d, %d): %s unexpected\n", yylineno,
                yycolumn, strtoken(parser.token));
        fclose(input);
        return 1;
    }

    yylex_destroy();
    parser_destroy(&parser);

    puts("AST:");
    puts("========================================");
    ast_print(&ast, stdout);
    puts("========================================");
    puts("");

    allocator_t core_allocator;
    linalloc_t linalloc;
    linalloc_init(&linalloc);
    linalloc_allocator(&linalloc, &core_allocator);

    env_t env, intrinsics_env;
    env_init_with_allocator(&env, &core_allocator);
    env_init_with_allocator(&intrinsics_env, &core_allocator);

    intrinsics_load(&intrinsics_env, &core_allocator);
    env.upper_scope = &intrinsics_env;

    if (coregen_from_module_ast(&ast, &env, &core_allocator) == -1) {
        fprintf(stderr, "Coregen error\n");
        fclose(input);
        return 1;
    }

    ast_destroy(&ast);
    linalloc_destroy(&parser_linalloc);

    puts("EXPRs:");
    puts("========================================");

    const vector_t /* const char* */ *keys = hashmap_keys(&env.scope);

    for (size_t i = 0; i < keys->len; ++i) {
        const char *expr_name = *((const char **)vector_get_ref(keys, i));
        const core_expr_t *expr =
            *((const core_expr_t **)hashmap_get(&env.scope, expr_name));

        printf("%s => ", expr_name);
        core_print(expr, stdout);
        puts("");
    }

    puts("========================================");
    puts("");

    env_destroy(&env);

    linalloc_destroy(&linalloc);

    fclose(input);

    return 0;
}

void usage(const char *name) {
    fprintf(stderr, "Usage: %s <input file>\n", name);
}