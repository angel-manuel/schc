
#include <stdio.h>

#include <data/hashmap.h>

int main() {
    int one = 1, two = 2, three = 3, tmp;
    hashmap_t map;

    hashmap_init(&map, sizeof(int));

    printf("len(map) -> %zd\n", map.len);

    hashmap_put(&map, "aaa12345", &one);
    printf("aaa12345 = 1\n");
    hashmap_put(&map, "bbb", &two);
    printf("bbb = 2\n");

    printf("len(map) -> %zd\n", map.len);

    tmp = *((int*)hashmap_get(&map, "aaa12345"));
    printf("aaa12345 -> %d\n", tmp);
    tmp = *((int*)hashmap_get(&map, "bbb"));
    printf("bbb -> %d\n", tmp);

    printf("ccc @ %p\n", hashmap_get(&map, "ccc"));

    hashmap_put(&map, "aaa12345", &three);
    printf("aaa12345 = 3\n");

    printf("len(map) -> %zd\n", map.len);

    tmp = *((int*)hashmap_get(&map, "aaa12345"));
    printf("aaa12345 -> %d\n", tmp);
    tmp = *((int*)hashmap_get(&map, "bbb"));
    printf("bbb -> %d\n", tmp);

    printf("ccc @ %p\n", hashmap_get(&map, "ccc"));

    for (int i = 0; i < 1500; i++) {
        char key[10];

        sprintf(key, "k%d", i);

        hashmap_put(&map, key, &i);
        printf("%s = %d\n", key, i);
    }

    tmp = *((int*)hashmap_get(&map, "k42"));
    printf("k42 -> %d\n", tmp);
    tmp = *((int*)hashmap_get(&map, "k1337"));
    printf("k1337 -> %d\n", tmp);

    hashmap_destroy(&map);

    return 0;
}
