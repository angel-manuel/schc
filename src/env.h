#ifndef SCHC_CORE_ENV_H_
#define SCHC_CORE_ENV_H_

#include <stdint.h>
#include <stdio.h>

#include "data/allocator.h"
#include "data/hashmap.h"
#include "data/vector.h"

typedef struct env_ {
    struct env_ *upper_scope;
    hashmap_t /* core_expr_t* */ scope;
} env_t;

#include "core.h"

int env_init(env_t *env);
int env_init_with_allocator(env_t *env, allocator_t *allocator);
void env_destroy(env_t *env);

core_expr_t *env_get_expr(env_t *env, const char *symbol);
int env_put_expr(env_t *env, const char *symbol, core_expr_t *expr);
int env_list_scope(const env_t *env, vector_t /* const char * */ *out_scope,
                   int recursive);

int env_print_scope(const env_t *env, int recursive, FILE *fp);

#endif /*SCHC_CORE_ENV_H_*/
