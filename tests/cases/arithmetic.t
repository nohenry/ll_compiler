native void write_int(int64 d);
native void write_float32(float32 d);
native void write_float64(float64 d);

void main() {
    int a = 1;
    int b = 2;
    int c = 3;
    int d = 4;

    int e = a + b + c;
    write_int(e);

    // write_int(d * b);

    // e = (a + b) * (c + d);
    // write_int(e);

    // write_int(d / b);
    // write_int(e / b);

    // uint64 large_int = 0x451287362387879F;
    // write_int(cast(int64)large_int);

    // float32 flt = 123.456;
    // float32 flt1 = 45;
    // float32 flt2 = flt + flt1;
    // write_float32(flt);
    // write_float32(flt1);
    // write_float32(flt2);




    // float32 a_f32 = 1;
    // float32 b_f32 = 2;
    // float32 c_f32 = 3;
    // float32 d_f32 = 4;

    // float32 e_f32 = a_f32 + b_f32 + c_f32;
    // write_float32(e_f32);

    // write_float32(d_f32 * b_f32);

    // e_f32 = (a_f32 + b_f32) * (c_f32 + d_f32);
    // write_float32(e_f32);

    // write_float32(d_f32 / b_f32);
    // write_float32(e_f32 / b_f32);


    // float64 a_f64 = 1;
    // float64 b_f64 = 2;
    // float64 c_f64 = 3;
    // float64 d_f64 = 4;

    // float64 e_f64 = a_f64 + b_f64 + c_f64;
    // write_float64(e_f64);

    // write_float64(d_f64 * b_f64);

    // e_f64 = (a_f64 + b_f64) * (c_f64 + d_f64);
    // write_float64(e_f64);

    // write_float64(d_f64 / b_f64);
    // write_float64(e_f64 / b_f64);
}