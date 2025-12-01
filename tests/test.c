
#define OC_CORE_IMPLEMENTATION
#include "../core/core1.h"
#include <stdio.h>
#include <stdint.h>

void foo (int* a) {

}

#define FOO(a, n) a

int main(int argc, char** argv) {

    int64_t dst_width = 16;
    int64_t src_width = 8;
    int64_t cmp = (uint64_t)((int64_t)(1uLL << 63uLL) >> (dst_width - 1)) >> (64ll - dst_width);
    int64_t high_cmp = cmp & ~(1ull << (dst_width - 1));
    int64_t low_cmp = 1ull << (dst_width - 1);

    printf("%lld\n", high_cmp);
    printf("%lld\n", low_cmp);

    return 0;
}


