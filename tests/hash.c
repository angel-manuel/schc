
#include <stdio.h>

#include <data/hashmap.h>
#include <data/linalloc.h>

#include <test.h>

allocator_t *allocator;

static char *test_hashmap_init_with_cap() {
    hashmap_t map;

    test_assert("Hashmap is initialized",
                !hashmap_init_with_cap_and_allocator(&map, sizeof(int), 2048,
                                                     allocator));

    test_assert("Hashmap has chosen capacity", map.cap == 2048);
    test_assert("Hashmap has chosen elem_size", map.elem_size == sizeof(int));
    test_assert("Hashmap has length 0", map.len == 0);

    hashmap_destroy(&map);

    test_assert("Hashmap is initialized",
                hashmap_init_with_cap(&map, sizeof(int), 3000) == 0);

    test_assert("Hashmap has next power of 2 capacity", map.cap == 4096);

    hashmap_destroy(&map);

    return NULL;
}

static char *test_hashmap_simple_get_and_put() {
    int i42 = 42;
    int i12 = 12;
    int i1337 = 1337;
    hashmap_t map;

    test_assert("Hashmap is initialized",
                !hashmap_init_with_cap_and_allocator(&map, sizeof(int), 1024,
                                                     allocator));

    test_assert("put(hello, 42)", !hashmap_put(&map, "hello", &i42));
    test_assert("", map.len == 1);
    test_assert("put(world, 1337)", !hashmap_put(&map, "world", &i1337));
    test_assert("", map.len == 2);

    test_assert("get(hello) == 42", *((int *)hashmap_get(&map, "hello")) == 42);
    test_assert("get(world) == 1337",
                *((int *)hashmap_get(&map, "world")) == 1337);

    test_assert("put(hello, 12)", !hashmap_put(&map, "hello", &i12));
    test_assert("", map.len == 2);

    test_assert("get(hello) == 12", *((int *)hashmap_get(&map, "hello")) == 12);
    test_assert("get(foo) == null", hashmap_get(&map, "foo") == NULL);

    hashmap_destroy(&map);

    return NULL;
}

static char *test_hashmap_growth() {
    hashmap_t map;

    test_assert("Hashmap initialized with 256 capacity",
                !hashmap_init_with_cap_and_allocator(&map, sizeof(int), 256,
                                                     allocator));

    for (int i = 0; i < 1500; i++) {
        char key[10];

        sprintf(key, "k%d", i);

        test_assert("put(k#, #)", !hashmap_put(&map, key, &i));
    }

    test_assert("get(k42) == 42", *((int *)hashmap_get(&map, "k42")) == 42);
    test_assert("get(k1337) == 1337",
                *((int *)hashmap_get(&map, "k1337")) == 1337);
    test_assert("get(foo) == null", hashmap_get(&map, "foo") == NULL);

    test_assert("map has 1500 length", map.len == 1500);
    test_assert("map has 4096 capacity", map.cap == 4096);

    hashmap_destroy(&map);

    return NULL;
}

int main() {
    allocator = &default_allocator;

    test_run(test_hashmap_init_with_cap);
    test_run(test_hashmap_simple_get_and_put);
    test_run(test_hashmap_growth);

    linalloc_t linalloc;
    linalloc_init(&linalloc);

    allocator_t special_allocator;
    linalloc_allocator(&linalloc, &special_allocator);
    allocator = &special_allocator;

    test_run(test_hashmap_init_with_cap);
    test_run(test_hashmap_simple_get_and_put);
    test_run(test_hashmap_growth);

    linalloc_destroy(&linalloc);

    return 0;
}
