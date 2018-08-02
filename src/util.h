#ifndef SCHC_UTIL_H_
#define SCHC_UTIL_H_

#define TRYCR(var, exp, cond, ret)                                             \
    do {                                                                       \
        (var) = (exp);                                                         \
        if ((var) == (cond)) {                                                 \
            return (ret);                                                      \
        }                                                                      \
    } while (0);

#define TRYNEG(var, exp)                                                       \
    do {                                                                       \
        (var) = (exp);                                                         \
        if ((var) < 0) {                                                       \
            return -1;                                                         \
        }                                                                      \
    } while (0);

#define TRYC(var, exp, cond) TRYCR((var), (exp), (cond), (cond))
#define TRY(var, exp) TRYCR((var), (exp), -1, -1)

char *stralloc(const char *src);

#endif /*SCHC_UTIL_H_*/