#ifndef SCHC_UTIL_H_
#define SCHC_UTIL_H_

#include <signal.h>

#ifdef NDEBUG
#define DEBUG_PRINTF(...)                                                      \
    do {                                                                       \
    } while (0)
#define DEBUG_PRINTS(...)                                                      \
    do {                                                                       \
    } while (0)
#else
#define DEBUG_PRINTF(M, ...)                                                   \
    do {                                                                       \
        fprintf(stderr, "DEBUG %s: %d: " M "\n", __FILE__, __LINE__, ##__VA_ARGS__);    \
    } while (0)
#define DEBUG_PRINTS(val)                                                      \
    do {                                                                       \
        fprintf(stderr; #val " = %s\n", val)                                   \
    } while (0)
#endif /*NDEBUG*/

#define TRYCR(var, exp, cond, ret)                                             \
    do {                                                                       \
        (var) = (exp);                                                         \
        if ((var) == (cond)) {                                                 \
            raise(SIGINT);                                                     \
            return (ret);                                                      \
        }                                                                      \
    } while (0)

#define TRYNEG(var, exp)                                                       \
    do {                                                                       \
        (var) = (exp);                                                         \
        if ((var) < 0) {                                                       \
            raise(SIGINT);                                                     \
            return -1;                                                         \
        }                                                                      \
    } while (0)

#define TRYC(var, exp, cond) TRYCR((var), (exp), (cond), (cond))
#define TRY(var, exp) TRYCR((var), (exp), -1, -1)

char *stralloc(const char *src);

#endif /*SCHC_UTIL_H_*/