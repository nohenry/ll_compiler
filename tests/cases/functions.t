native void write_int(int d);

int get_data() {
    return 4;
}

int add(int a, int b) {
    return a + b;
}

void main() {
    write_int(get_data());
    write_int(add(3, 8));
}