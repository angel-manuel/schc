#include "env.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

#include "util.h"

#define ENV_INITIAL_CAPACITY 1024

#define ALLOC(size) ALLOCATOR_ALLOC(env->allocator, (size))
#define FREE(mem) ALLOCATOR_FREE(env->allocator, (mem))

int env_put_expr_no_alloc(env_t *env, const char *symbol, core_expr_t *owned_expr);

int env_init(env_t *env) {
    return env_init_with_allocator(env, &default_allocator);
}

int env_init_with_allocator(env_t *env, allocator_t *allocator) {
    assert(env != NULL);
    assert(allocator != NULL);

    int res;

    env->upper_scope = NULL;
    env->allocator = allocator;

    TRY(res, hashmap_init_with_cap_and_allocator(
                 &env->scope, sizeof(core_expr_t *), ENV_INITIAL_CAPACITY,
                 allocator));

    return 0;
}

void env_destroy(env_t *env) {
    assert(env != NULL);

    size_t vlen = env->scope.cap;

    for (size_t i = 0; i < vlen; ++i) {
        hashmap_location_t *loc = hashmap_get_entry(&env->scope, i, NULL);

        core_expr_t *expr = *(core_expr_t **)loc->data;

        if (loc != NULL && loc->key != NULL) {
            core_destroy(expr, env->allocator);
            FREE(expr);
        }
    }

    hashmap_destroy(&env->scope);

    if (env->upper_scope != NULL) {
        env_destroy(env->upper_scope);
        env->upper_scope = NULL;
    }
}

core_expr_t *env_get_expr(env_t *env, const char *symbol) {
    assert(env != NULL);
    assert(symbol != NULL);

    core_expr_t **expr = (core_expr_t **)hashmap_get(&env->scope, symbol);

    if (expr == NULL) {
        if (env->upper_scope) {
            return env_get_expr(env->upper_scope, symbol);
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
    assert(expr != NULL);

    core_expr_t *owned_expr;
    TRYCR(owned_expr, ALLOC(sizeof(core_expr_t)), NULL, -1);
    *owned_expr = *expr;

    return env_put_expr_no_alloc(env, symbol, owned_expr);
}

int env_put_expr_no_alloc(env_t *env, const char *symbol, core_expr_t *owned_expr) {
    assert(env != NULL);
    assert(symbol != NULL);
    assert(owned_expr != NULL);

    int res;

    TRY(res, hashmap_put(&env->scope, symbol, &owned_expr));

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
