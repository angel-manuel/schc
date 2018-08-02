#include "env.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

#include "util.h"

int env_init(env_t *env) {
    assert(env != NULL);

    env->next_id = 0;

    return 0;
}

void env_destroy(env_t *env) {
    assert(env != NULL);

    for (uint64_t i = 0; i < env->next_id; ++i) {
        if (env->dict[i] != NULL) {
            free(env->dict[i]);
            env->dict[i] = NULL;
        }
    }
}

const char *env_get(const env_t *env, env_id_t id) {
    assert(env != NULL);
    assert(id < ENV_DICT_SIZE);

    if (id < env->next_id) {
        return env->dict[id];
    } else {
        return NULL;
    }
}

int env_put(env_t *env, const char *str, env_id_t *out_id) {
    assert(env != NULL);
    assert(str != NULL);
    assert(out_id != NULL);

    if (env->next_id >= ENV_DICT_SIZE) {
        return -1;
    }

    char *new_str;
    TRYCR(new_str, stralloc(str), NULL, -1);

    *out_id = env->next_id++;

    env->dict[*out_id] = new_str;

    return 0;
}
