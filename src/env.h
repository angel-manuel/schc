#ifndef SCHC_CORE_ENV_H_
#define SCHC_CORE_ENV_H_

#include <stdint.h>
#include <stdio.h>

#include "core.h"
#include "data/hashmap.h"
#include "data/vector.h"

typedef uint64_t env_id_t;

typedef struct env_ {
    struct env_ *upper_scope;
    hashmap_t /* core_expr_t* */ scope;
} env_t;

int env_init(env_t *env);
void env_destroy(env_t *env);

core_expr_t *env_get_expr(env_t *env, const char *symbol);
int env_put_expr(env_t *env, const char *symbol, core_expr_t *expr);

#endif /*SCHC_CORE_ENV_H_*/
