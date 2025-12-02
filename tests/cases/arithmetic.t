native void write_int(int64 d);

void main() {
    int a = 1;
    int b = 2;
    int c = 3;
    int d = 4;

    int e = a + b + c;
    write_int(e);

    write_int(d * b);

    e = (a + b) * (c + d);
    write_int(e);

    write_int(d / b);
    write_int(e / b);
}