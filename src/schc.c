#include <errno.h>
#include <stdio.h>

#include "ast.h"
#include "core.h"
#include "coregen.h"
#include "data/hashmap.h"
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

    parser_t parser;
    parser_init(&parser);

    ast_t ast;

    if (parser_parse(&parser, &ast) == -1) {
        fprintf(stderr, "Parse error(%d, %d): %s unexpected\n", yylineno,
                yycolumn, strtoken(parser.token));
        return 1;
    }

    yylex_destroy();

    puts("AST:");
    puts("========================================");
    ast_print(&ast, stdout);
    puts("========================================");
    puts("");

    vector_t /* core_expr_t */ expr_heap;
    vector_init_with_cap(&expr_heap, sizeof(core_expr_t), 100000);

    env_t env, intrinsics_env;
    env_init(&env);
    env_init(&intrinsics_env);

    intrinsics_load(&intrinsics_env, &expr_heap);
    env.upper_scope = &intrinsics_env;

    if (coregen_from_module_ast(&ast, &env, &expr_heap) == -1) {
        fprintf(stderr, "Coregen error\n");
        return 1;
    }

    puts("EXPRs:");
    puts("========================================");

    const vector_t /* const char* */ *keys = hashmap_keys(&env.scope);

    for (size_t i = 0; i < keys->len; ++i) {
        const char *expr_name = *((const char **)vector_get_ref(keys, i));
        const core_expr_t *expr =
            *((const core_expr_t **)hashmap_get(&env.scope, expr_name));

        core_print(expr, stdout);
        puts("");
    }

    puts("========================================");
    puts("");

    vector_destroy(&expr_heap);

    env_destroy(&env);

    ast_destroy(&ast);

    parser_destroy(&parser);

    // printf("input = %p\n", input);
    fclose(input);

    return 0;
}

void usage(const char *name) {
    fprintf(stderr, "Usage: %s <input file>\n", name);
}