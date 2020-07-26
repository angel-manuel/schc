#include "intrinsics.h"

#include <assert.h>

#include "../data/vector.h"
#include "../env.h"
#include "../util.h"

int intrinsics_load(env_t *env, vector_t /* core_expr_t */ *expr_heap) {
    assert(env != NULL);
    assert(expr_heap != NULL);

    int res;
    core_expr_t *expr;

    core_expr_t putStrLn;
    putStrLn.name = "putStrLn";
    putStrLn.form = CORE_INTRINSIC;
    putStrLn.intrinsic.name = "putStrLn";

    TRYCR(expr, (core_expr_t *)vector_push_back(expr_heap, &putStrLn), NULL,
          -1);
    TRY(res, env_put_expr(env, "putStrLn", expr));

    core_expr_t show;
    show.name = "show";
    show.form = CORE_INTRINSIC;
    show.intrinsic.name = "show";

    TRYCR(expr, (core_expr_t *)vector_push_back(expr_heap, &show), NULL, -1);
    TRY(res, env_put_expr(env, "show", expr));

    core_expr_t div;
    div.name = "div";
    div.form = CORE_INTRINSIC;
    div.intrinsic.name = "div";

    TRYCR(expr, (core_expr_t *)vector_push_back(expr_heap, &div), NULL, -1);
    TRY(res, env_put_expr(env, "div", expr));

    return res;
}