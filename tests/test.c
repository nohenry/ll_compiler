
#define OC_CORE_IMPLEMENTATION
#include "../core/core1.h"

#include "../src/common.h"
#include "../src/common.c"

int main(int argc, char **argv)
{
    Oc_Arena arena = {0};

    Hash_Map(string, uint32) f = {0};
    hash_map_put(&arena, &f, lit("foobar"), 42);

    uint32* p = hash_map_get(&arena, &f, lit("foobar"));
    if (p) print("found: {}\n", *p);


    // print("{}\n", (void*)oc_arena_alloc_aligned(&arena, 8, 8));
    // print("{}\n", (void*)oc_arena_alloc_aligned(&arena, 40, 16));

    return 0;
}
