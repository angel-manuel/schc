#include <stdio.h>
#include <string.h>
#include <time.h>

#include <data/linalloc.h>

#include <test.h>

static char *test_linalloc() {
    linalloc_t linalloc;

    char test_str[16] = "Hello test!";
    char *ptrs[100];

    srand(time(NULL));

    test_assert("Linalloc initialized", !linalloc_init(&linalloc));

    for (int i = 0; i < 100; i++) {
        const int next_size =
            abs((rand() * 4) % (65536 * 4)) + sizeof(test_str);

        char *mem = (char *)linalloc_alloc(&linalloc, (size_t)next_size);

        test_assert("Linalloc alloc", mem != NULL);

        memcpy(mem, test_str, sizeof(test_str));
        ptrs[i] = mem;
    }

    for (int i = 0; i < 100; i++) {
        char *mem = ptrs[i];

        test_assert("Mem not changed", !strcmp(mem, test_str));
    }

    linalloc_destroy(&linalloc);

    return NULL;
}

int main() {
    test_run(test_linalloc);

    return 0;
}