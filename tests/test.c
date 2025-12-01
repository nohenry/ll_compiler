
#define OC_CORE_IMPLEMENTATION
#include "../core/core1.h"
#include <stdio.h>
#include <stdint.h>

void foo (int* a) {

}

#define FOO(a, n) a

int main(int argc, char** argv) {
    signed char a = -10;
    unsigned int b = a;

    printf("%u\n", b);

    b = 1000;
    a = b;

    printf("%u\n", a);

    // print("{}\n", b);

    return 0;
}


