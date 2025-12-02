native void write_int(int64 d);

void main() {
    int[4] a;
    a[0] = 123;
    write_int(a[0]);
    a[1] = 66;
    a[2] = 54;
    a[3] = 76;
    write_int(a[1]);
    write_int(a[2]);
    write_int(a[3]);


    int64[5] b;
    int8 a2 = -34;
    b[3] = cast(int64)a2;
    write_int(b[3]);

    int64* bb = &b[3];
    write_int(*bb);
    *bb = 56;
    write_int(*bb);
    write_int(b[3]);

    // uint32[5] aaa = [1, 2, 3, 4, 5];
}