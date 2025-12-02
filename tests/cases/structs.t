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

void do_smth_with_struct(Foobar f) {
    write_int(f.a);
}

void main() {
    Foobar foo;
    foo.a = 123;
    write_int(foo.a);

    foo.s.d = 69;
    write_int(foo.s.d);

    do_smth_with_struct(foo);
    Foobar foo1 = foo;
    // write_int(foo1.a);
}