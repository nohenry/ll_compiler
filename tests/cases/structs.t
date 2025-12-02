native void write_int(int64 d);

struct SubStruct {
    int c;
    int d;
}

struct Foobar {
    SubStruct s;
    int a;
    int b;
}

void main() {
    Foobar foo;
    foo.a = 123;
    write_int(foo.a);

    foo.s.d = 69;
    write_int(foo.s.d);
}