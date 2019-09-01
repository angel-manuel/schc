#include "env.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

#include "util.h"

#define ENV_INITIAL_CAPACITY 1024

int env_init(env_t *env) {
    assert(env != NULL);

    int res;

    env->upper_scope = NULL;

    TRY(res, hashmap_init_with_cap(&env->scope, sizeof(core_expr_t *),
                                   ENV_INITIAL_CAPACITY, NULL));

    return 0;
}

void env_destroy(env_t *env) {
    assert(env != NULL);

    hashmap_destroy(&env->scope);
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

    int res;

    TRY(res, hashmap_put(&env->scope, symbol, &expr));

    return 0;
}