#include "intrinsics.h"

#include <assert.h>

#include "../data/allocator.h"
#include "../env.h"
#include "../util.h"

int intrinsics_load(env_t *env) {
    assert(env != NULL);

    int res;

    core_expr_t expr;

    expr.name = "putStrLn";
    expr.form = CORE_INTRINSIC;
    expr.intrinsic.name = "putStrLn";

    TRY(res, env_put_expr(env, "putStrLn", &expr));

    expr.name = "show";
    expr.form = CORE_INTRINSIC;
    expr.intrinsic.name = "show";

    TRY(res, env_put_expr(env, "show", &expr));

    expr.name = "div";
    expr.form = CORE_INTRINSIC;
    expr.intrinsic.name = "div";

    TRY(res, env_put_expr(env, "div", &expr));

    expr.name = "+";
    expr.form = CORE_INTRINSIC;
    expr.intrinsic.name = "plus";

    TRY(res, env_put_expr(env, "+", &expr));

    expr.name = "-";
    expr.form = CORE_INTRINSIC;
    expr.intrinsic.name = "minus";

    TRY(res, env_put_expr(env, "-", &expr));

    expr.name = "*";
    expr.form = CORE_INTRINSIC;
    expr.intrinsic.name = "mult";

    TRY(res, env_put_expr(env, "*", &expr));

    expr.name = ">=";
    expr.form = CORE_INTRINSIC;
    expr.intrinsic.name = "gte";

    TRY(res, env_put_expr(env, ">=", &expr));

    expr.name = "<=";
    expr.form = CORE_INTRINSIC;
    expr.intrinsic.name = "lte";

    TRY(res, env_put_expr(env, "<=", &expr));

    expr.name = "==";
    expr.form = CORE_INTRINSIC;
    expr.intrinsic.name = "eq";

    TRY(res, env_put_expr(env, "==", &expr));

    return res;
}