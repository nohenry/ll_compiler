#include "core1.h"
int main() {

    Oc_Arena arena = {0};
    for (int i = 0; i < 200; i++) {
        int* a = oc_arena_alloc(&arena, sizeof(int));
        // print("{}\n", a);
    }

    Oc_String_Builder builder;
    oc_sb_init(&builder, &arena);
    oc_sb_append_char_str(&builder, "Hello, World!\n");
    oc_sb_append_char_str(&builder, "ok bruhv");
    wprint(&builder, "hi {}", 69);

    string s = oc_sb_to_string(&builder);
    print("the sb created: '{}'\n", s);

    print("hello world {} {} {} {}\n", 69, 420.6969f, (char)'c', "wow, it's a string");

    return 0;
}