#ifndef SCHC_COREGEN_H_
#define SCHC_COREGEN_H_

#include "ast.h"
#include "core.h"
#include "env.h"

#include "data/hashmap.h"

int coregen_from_module_ast(const ast_t *ast, env_t *env,
                            vector_t /* core_expr_t */ *expr_heap);

#endif /*SCHC_COREGEN_H_*/