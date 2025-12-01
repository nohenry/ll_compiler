native void write_int(int d);

void main() {
    int8 a = 123;
    uint b = a;

    write_int(b);

    a = -10;
    write_int(a);

    b = 1000;
    a = b;
    write_int(a);
}