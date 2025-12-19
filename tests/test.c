
#define OC_CORE_IMPLEMENTATION
#include "../core/core1.h"



int main(int argc, char** argv) {
    Oc_Arena arena = { 0 };

    int64_t i = -0x7FFFFFFLL;
    print("{x}\n", (uint64_t)i);
    // print("{}\n", (void*)oc_arena_alloc_aligned(&arena, 8, 8));
    // print("{}\n", (void*)oc_arena_alloc_aligned(&arena, 40, 16));

    return 0;
}


