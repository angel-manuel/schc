#ifndef SCHC_CORE_ENV_H_
#define SCHC_CORE_ENV_H_

#include <stdint.h>
#include <stdio.h>

#define ENV_DICT_SIZE 32000

typedef uint64_t env_id_t;

typedef struct env_ {
    env_id_t next_id;
    char *dict[ENV_DICT_SIZE];
} env_t;

int env_init(env_t *env);
void env_destroy(env_t *env);

const char *env_get(const env_t *env, env_id_t id);
int env_put(env_t *env, const char *str, env_id_t *out_id);

#endif /*SCHC_CORE_ENV_H_*/
