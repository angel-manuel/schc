#ifndef SCHC_COREGEN_H_
#define SCHC_COREGEN_H_

#include "ast.h"
#include "core.h"
#include "env.h"

#include "data/allocator.h"
#include "data/hashmap.h"

int coregen_from_module_ast(const ast_t *ast, env_t *env);

#endif /*SCHC_COREGEN_H_*/