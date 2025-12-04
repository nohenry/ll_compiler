native void write_int(int64 d);

struct SmallStruct {
    int c;
    int d;
}

struct Foobar {
    SmallStruct* s;
    int a;
    int b;
}

void do_smth_with_struct(Foobar f) {
    write_int(f.a);
}

void struct_ref(Foobar* f) {
    Foobar ff = *f;
    write_int(ff.a);
    write_int(f.a);
}

void small_struct(SmallStruct ss) {
    write_int(ss.c);
    write_int(ss.d);
}

void small_struct_ref(SmallStruct* ssp) {
    SmallStruct ss = *ssp;
    write_int(ss.c);
    write_int(ss.d);
    write_int(ssp.d);
}

void pass_struct_over_stack(int a, int b, int c, int d, int e, int f, SmallStruct ss) {
    write_int(ss.c);
    write_int(ss.d);
}

void pass_struct_over_stack_ref(int a, int b, int c, int d, int e, int f, SmallStruct* ss) {
    write_int(ss.c);
    write_int(ss.d);
}

void modify_struct(Foobar f) {
    f.b = 145;
}

Foobar return_struct() {
    Foobar result;
    result.a = 10;
    result.b = 20;
    return result;
}

SmallStruct return_small_struct() {
    SmallStruct result;
    result.c = 100;
    result.d = 200;
    return result;
}

void main() {
    Foobar foo;
    foo.a = 123;
    write_int(foo.a);

    foo.s.d = 69;
    write_int(foo.s.d);

    do_smth_with_struct(foo);
    foo.do_smth_with_struct();
    Foobar foo1 = foo;
    write_int(foo1.a);

    Foobar* pfoo1 = &foo;
    Foobar foo2 = *pfoo1;
    write_int(foo2.s.d);

    struct_ref(&foo);
    foo.struct_ref();

    foo.s.c = 43;
    foo.s.d = 12;
    small_struct(*foo.s);
    // foo.s.small_struct();
    small_struct_ref(foo.s);
    foo.s.small_struct_ref();

    foo.s.c = 156;
    foo.s.d = 87;
    pass_struct_over_stack(2, 4, 6, 8, 10, 12, *foo.s);
    pass_struct_over_stack_ref(2, 4, 6, 8, 10, 12, foo.s);

    foo.b = 90;
    modify_struct(foo);
    write_int(foo.b);

    Foobar returned = return_struct();
    write_int(returned.a);
    write_int(returned.b);

    SmallStruct returned_ss = return_small_struct();
    write_int(returned_ss.c);
    write_int(returned_ss.d);
    returned.s = &returned_ss;

    write_int(returned.s.c);
    write_int(returned.s.d);
}