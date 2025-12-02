native void write_int(int64 d);

// int get_data() {
//     return 4;
// }

// int add(int a, int b) {
//     return a + b;
// }

// int sub(int a, int b) {
//     return a - b;
// }

// int default_args(int a = 1, int b = 78) {
//     return a * b;
// }

int many_args(int a, int b, int c, int d, int e, int f) {
    return a * b * c * d * e * f;
}

void main() {
    // write_int(get_data());
    // write_int(add(3, 8));
    // write_int(sub(a = 8, b = 3));
    // write_int(sub(b = 3, a = 8));
    // write_int(default_args());
    // write_int(default_args(2));
    // write_int(default_args(2, 8));
    // write_int(default_args(a = 3));
    // write_int(default_args(a = 3, b = 78));
    // write_int(default_args(b = 2, a = 7));

    // write_int(4.add(1));

    write_int(many_args(1, 2, 3, 4, 5, 6));
}