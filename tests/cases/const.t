native void write_int(int64 d);
native void write_float32(float32 d);
native void write_float64(float64 d);

int64 get_array_size(int64 a) {
    int64 b = 50;
    int64* pb = &b;
    return 3 + 4 + a + *pb;
}

void main() {
    const int64 value = 8 + 4; // folded
    write_int(value);
    
    const int64 value = const get_array_size(123);
    write_int(value);

    int[get_array_size(80)] array;
    int[array.length] array1;
    write_int(cast(int64)array.length);
    write_int(cast(int64)array1.length);
}
