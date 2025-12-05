native void write_int(int64 d);
native void write_float32(float32 d);
native void write_float64(float64 d);

void do_integer_casting() {
    int8 a = 123;
    uint b = cast(uint)a;

    write_int(b);

    a = -10;
    write_int(a);

    b = 1000;
    a = cast(int8)b;
    write_int(a);

    int16 a1 = -123;
    uint16 b1 = cast(uint16)a1;
    write_int(b1);

    int32 a2 = -456;
    write_int(cast(int64)a2);

    int64 a3 = 1243245;
    write_int(a3);
    int8 b3 = cast(int8)a3;
    write_int(b3);
}

void do_floating_casting() {
    float32 small = 13.5;
    float64 big = cast(float64)small;
    write_float32(small);
    write_float64(big);

    int64 from_small = cast(int64)small;
    write_int(from_small);

    int32 from_big = cast(int32)big;
    write_int(from_big);

    uint16 uib = 11356;
    float32 ff = cast(float32)uib;
    write_float32(ff);
    float64 ff1 = cast(float64)uib;
    write_float64(ff1);

    int16 ib = -12035;
    ff = cast(float32)ib;
    write_float32(ff);
    ff1 = cast(float64)ib;
    write_float64(ff1);

    uint8 byte = 98;
    ff = cast(float32)byte;
    write_float32(ff);
    ff1 = cast(float64)byte;
    write_float64(ff1);

    float cvt_to_byte = 87.4;
    int8 bbb = cast(int8)cvt_to_byte;
    write_int(bbb);
}

void main() {
    do_integer_casting();
    do_floating_casting();
}