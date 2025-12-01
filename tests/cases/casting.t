native void write_int(int64 d);

void main() {
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