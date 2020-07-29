#ifndef SCHC_COREGEN_H_
#define SCHC_COREGEN_H_

#include "ast.h"
#include "core.h"
#include "env.h"

#include "data/hashmap.h"
#include "data/linalloc.h"

int coregen_from_module_ast(const ast_t *ast, env_t *env, linalloc_t *alloc);

#endif /*SCHC_COREGEN_H_*/