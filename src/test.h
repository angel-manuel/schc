#ifndef SCHC_TEST_H_
#define SCHC_TEST_H_

#include <stdlib.h>

#define S(x) #x
#define S_(x) S(x)
#define S__LINE__ S_(__LINE__)

#define COLORS

#ifdef COLORS
#define OK_COLOR "\e[32m"
#define FAIL_COLOR "\e[31m"
#define NO_COLOR "\e[0m"
#else
#define OK_COLOR ""
#define FAIL_COLOR ""
#define NO_COLOR ""
#endif /*COLORS*/

#define test_assert(message, test)                                             \
    do {                                                                       \
        if (!(test)) {                                                         \
            return ("Assertion failed at " __FILE__ ":" S__LINE__ ". " message \
                    " (" #test ")");                                           \
        }                                                                      \
    } while (0)

#define test_run(test)                                                         \
    do {                                                                       \
        char *res = test();                                                    \
        if (res) {                                                             \
            fprintf(stderr, FAIL_COLOR "%s: FAIL\n%s\n" NO_COLOR, #test, res); \
            exit(1);                                                           \
        } else {                                                               \
            fprintf(stderr, OK_COLOR "%s: OK\n" NO_COLOR, #test);              \
        }                                                                      \
    } while (0)

#endif /*SCHC_TEST_H_*/
