#include "env.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

#include "util.h"

#define ENV_INITIAL_CAPACITY 1024

int env_init(env_t *env) {
    return env_init_with_allocator(env, &default_allocator);
}

int env_init_with_allocator(env_t *env, allocator_t *allocator) {
    assert(env != NULL);
    assert(allocator != NULL);

    int res;

    env->upper_scope = NULL;

    TRY(res, hashmap_init_with_cap_and_allocator(
                 &env->scope, sizeof(core_expr_t *), ENV_INITIAL_CAPACITY,
                 allocator, NULL));

    return 0;
}

void env_destroy(env_t *env) {
    assert(env != NULL);

    if (env->upper_scope != NULL) {
        env_destroy(env->upper_scope);
        env->upper_scope = NULL;
    }

    hashmap_destroy(&env->scope);
}

core_expr_t *env_get_expr(env_t *env, const char *symbol) {
    assert(env != NULL);
    assert(symbol != NULL);

    core_expr_t **expr = (core_expr_t **)hashmap_get(&env->scope, symbol);

    if (expr == NULL) {
        if (env->upper_scope) {
            core_expr_t *ret = env_get_expr(env->upper_scope, symbol);
            return ret;
        } else {
            return NULL;
        }
    } else {
        return *expr;
    }
}

int env_put_expr(env_t *env, const char *symbol, core_expr_t *expr) {
    assert(env != NULL);
    assert(symbol != NULL);

    int res;

    TRY(res, hashmap_put(&env->scope, symbol, &expr));

    return 0;
}

int env_list_scope(const env_t *env, vector_t /* const char * */ *out_scope,
                   int recursive) {
    assert(env != NULL);
    assert(out_scope != NULL);

    int res = 0;

    for (size_t i = 0; i < env->scope.keys.len; i++) {
        const char *scope_var =
            *(const char **)vector_get_ref(&env->scope.keys, i);

        void *memres;
        TRYCR(memres, vector_push_back(out_scope, &scope_var), NULL, -1);
    }

    if (recursive && env->upper_scope != NULL) {
        TRY(res, env_list_scope(env->upper_scope, out_scope, recursive));
    }

    return res;
}

int env_print_scope(const env_t *env, int recursive, FILE *fp) {
    assert(env != NULL);
    assert(fp != NULL);

    int res = 0;

    vector_t /* const char * */ scope;
    vector_init(&scope, sizeof(const char *));

    env_list_scope(env, &scope, recursive);

    for (size_t i = 0; i < scope.len; ++i) {
        const char *varname = *(const char **)vector_get_ref(&scope, i);

        TRYNEG(res, fprintf(fp, "%s\n", varname));
    }

    vector_destroy(&scope);

    return res;
}
