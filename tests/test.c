
#define OC_CORE_IMPLEMENTATION
#include "../core/core1.h"



int main(int argc, char** argv) {
    Oc_Arena arena = { 0 };

    print("{}\n", (void*)oc_arena_alloc_aligned(&arena, 40, 16));
    print("{}\n", (void*)oc_arena_alloc_aligned(&arena, 8, 8));
    print("{}\n", (void*)oc_arena_alloc_aligned(&arena, 40, 16));

    return 0;
}


