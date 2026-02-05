#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ast.h"
#include "codegen.h"
#include "core.h"
#include "coregen.h"
#include "data/hashmap.h"
#include "data/linalloc.h"
#include "intrinsics/intrinsics.h"
#include "lexer.h"
#include "parser.h"

void usage(const char *name);

typedef struct {
    const char *input_file;
    const char *output_file;  // -o: C output file
    const char *binary_file;  // -b: binary output file
    int emit_c;
    int compile_binary;
    int verbose;
} options_t;

static int parse_args(int argc, char *argv[], options_t *opts) {
    opts->input_file = NULL;
    opts->output_file = NULL;
    opts->binary_file = NULL;
    opts->emit_c = 0;
    opts->compile_binary = 0;
    opts->verbose = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            opts->output_file = argv[++i];
            opts->emit_c = 1;
        } else if (strcmp(argv[i], "-b") == 0 && i + 1 < argc) {
            opts->binary_file = argv[++i];
            opts->compile_binary = 1;
        } else if (strcmp(argv[i], "-v") == 0) {
            opts->verbose = 1;
        } else if (argv[i][0] != '-') {
            opts->input_file = argv[i];
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            return -1;
        }
    }

    return opts->input_file ? 0 : -1;
}

int main(int argc, char *argv[]) {
    options_t opts;

    if (parse_args(argc, argv, &opts) == -1) {
        usage(argv[0]);
        return 1;
    }

    if (opts.verbose) {
        puts("Simple C Haskell Compiler");
    }

    const char *input_filename = opts.input_file;

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

    if (opts.verbose) {
        puts("AST:");
        puts("========================================");
        ast_print(&ast, stdout);
        puts("========================================");
        puts("");
    }

    allocator_t core_allocator;
    linalloc_t linalloc;
    linalloc_init(&linalloc);
    linalloc_allocator(&linalloc, &core_allocator);

    env_t env, intrinsics_env;
    env_init_with_allocator(&env, &core_allocator);
    env_init_with_allocator(&intrinsics_env, &core_allocator);

    intrinsics_load(&intrinsics_env);
    env.upper_scope = &intrinsics_env;

    if (coregen_from_module_ast(&ast, &env) == -1) {
        fprintf(stderr, "Coregen error\n");
        fclose(input);
        return 1;
    }

    ast_destroy(&ast, &parser_allocator);
    linalloc_destroy(&parser_linalloc);

    if (opts.verbose) {
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
    }

    // Code generation
    if (opts.emit_c || opts.compile_binary) {
        const char *c_output = opts.output_file;
        char tmp_c_file[256] = {0};

        // If compiling to binary but no C output specified, use temp file
        if (opts.compile_binary && !opts.emit_c) {
            snprintf(tmp_c_file, sizeof(tmp_c_file), "/tmp/schc_%d.c", getpid());
            c_output = tmp_c_file;
        }

        FILE *output = fopen(c_output, "w");
        if (output == NULL) {
            fprintf(stderr, "Could not open output file '%s': ", c_output);
            perror("");
            env_destroy(&env);
            linalloc_destroy(&linalloc);
            fclose(input);
            return 1;
        }

        codegen_ctx_t codegen;
        codegen_init(&codegen, output);
        codegen_emit_program(&codegen, &env);

        fclose(output);

        if (opts.verbose && opts.emit_c) {
            printf("Generated: %s\n", c_output);
        }

        // Compile to binary if requested
        if (opts.compile_binary) {
            const char *cc = getenv("CC");
            if (cc == NULL) cc = "gcc";

            // Get the directory where schc executable is located for runtime
            char cmd[1024];
            snprintf(cmd, sizeof(cmd),
                     "%s -o %s %s -Isrc src/runtime/runtime.c -std=c99",
                     cc, opts.binary_file, c_output);

            if (opts.verbose) {
                printf("Compiling: %s\n", cmd);
            }

            int ret = system(cmd);
            if (ret != 0) {
                fprintf(stderr, "Compilation failed with code %d\n", ret);
                if (tmp_c_file[0]) remove(tmp_c_file);
                env_destroy(&env);
                linalloc_destroy(&linalloc);
                fclose(input);
                return 1;
            }

            // Clean up temp file
            if (tmp_c_file[0]) {
                remove(tmp_c_file);
            }

            if (opts.verbose) {
                printf("Binary: %s\n", opts.binary_file);
            }
        }
    }

    env_destroy(&env);

    linalloc_destroy(&linalloc);

    fclose(input);

    return 0;
}

void usage(const char *name) {
    fprintf(stderr, "Usage: %s [options] <input.hs>\n", name);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -o <file.c>   Emit generated C code to file\n");
    fprintf(stderr, "  -b <binary>   Compile to binary (uses $CC or gcc)\n");
    fprintf(stderr, "  -v            Verbose output (show AST, Core)\n");
}