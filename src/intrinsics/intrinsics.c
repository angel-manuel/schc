#include "intrinsics.h"

#include <assert.h>

#include "../data/linalloc.h"
#include "../env.h"
#include "../util.h"

#define ALLOC(x) linalloc_alloc(linalloc, (x))

int intrinsics_load(env_t *env, linalloc_t *linalloc) {
    assert(env != NULL);
    assert(linalloc != NULL);

    int res;

    core_expr_t *putStrLn;
    TRYCR(putStrLn, ALLOC(sizeof(core_expr_t)), NULL, -1);

    putStrLn->name = "putStrLn";
    putStrLn->form = CORE_INTRINSIC;
    putStrLn->intrinsic.name = "putStrLn";

    TRY(res, env_put_expr(env, "putStrLn", putStrLn));

    core_expr_t *show;
    TRYCR(show, ALLOC(sizeof(core_expr_t)), NULL, -1);

    show->name = "show";
    show->form = CORE_INTRINSIC;
    show->intrinsic.name = "show";

    TRY(res, env_put_expr(env, "show", show));

    core_expr_t *div;
    TRYCR(div, ALLOC(sizeof(core_expr_t)), NULL, -1);

    div->name = "div";
    div->form = CORE_INTRINSIC;
    div->intrinsic.name = "div";

    TRY(res, env_put_expr(env, "div", div));

    core_expr_t *plus;
    TRYCR(plus, ALLOC(sizeof(core_expr_t)), NULL, -1);

    plus->name = "+";
    plus->form = CORE_INTRINSIC;
    plus->intrinsic.name = "plus";

    TRY(res, env_put_expr(env, "+", plus));

    core_expr_t *minus;
    TRYCR(minus, ALLOC(sizeof(core_expr_t)), NULL, -1);

    minus->name = "-";
    minus->form = CORE_INTRINSIC;
    minus->intrinsic.name = "minus";

    TRY(res, env_put_expr(env, "-", minus));

    core_expr_t *mult;
    TRYCR(mult, ALLOC(sizeof(core_expr_t)), NULL, -1);

    mult->name = "*";
    mult->form = CORE_INTRINSIC;
    mult->intrinsic.name = "mult";

    TRY(res, env_put_expr(env, "*", mult));

    core_expr_t *gte;
    TRYCR(gte, ALLOC(sizeof(core_expr_t)), NULL, -1);

    gte->name = ">=";
    gte->form = CORE_INTRINSIC;
    gte->intrinsic.name = "gte";

    TRY(res, env_put_expr(env, ">=", gte));

    core_expr_t *lte;
    TRYCR(lte, ALLOC(sizeof(core_expr_t)), NULL, -1);

    lte->name = "<=";
    lte->form = CORE_INTRINSIC;
    lte->intrinsic.name = "lte";

    TRY(res, env_put_expr(env, "<=", lte));

    core_expr_t *eq;
    TRYCR(eq, ALLOC(sizeof(core_expr_t)), NULL, -1);

    eq->name = "==";
    eq->form = CORE_INTRINSIC;
    eq->intrinsic.name = "eq";

    TRY(res, env_put_expr(env, "==", eq));

    return res;
}