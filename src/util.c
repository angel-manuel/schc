#include "util.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

char *stralloc(const char *src) {
    assert(src != NULL);

    size_t srclen = strlen(src);
    char *dst = (char *)malloc(srclen + 1);

    if (dst == NULL) {
        return NULL;
    }

    memcpy(dst, src, srclen + 1);

    return dst;
}