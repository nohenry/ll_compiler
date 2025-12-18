native void write_int(int64 d);

void pass_array_to_function(int64[6] array) {
    write_int(array[0]);
    write_int(array[4]);
}

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

    int[:] slice = a;
    write_int(slice[0]);

    int[:] copy_slice = slice;
    write_int(copy_slice[0]);
    copy_slice[1] = 67;
    write_int(copy_slice[1]);

    int[:] new_slice = slice[1:3];
    write_int(new_slice[0]);

    int64[6] to_fn = [8, 10, 12, 14, 16, 18];
    pass_array_to_function(to_fn);
}