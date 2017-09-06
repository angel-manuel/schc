#ifndef SCHC_UTIL_H_
#define SCHC_UTIL_H_

#define TRYCR(var, exp, cond, ret)  \
(var) = (exp);                        \
if ((var) == (cond)) {              \
    return (ret);                   \
} 

#define TRYC(var, exp, cond) TRYCR((var), (exp), (cond), (cond))
#define TRY(var, exp) TRYCR((var), (exp), -1, -1)


char *stralloc(const char *src);

#endif/*SCHC_UTIL_H_*/