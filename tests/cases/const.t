native void write_int(int64 d);
native void write_float32(float32 d);
native void write_float64(float64 d);

struct Big_Data {
    int64 a;
    int64 b;
}

int64 get_array_size(int64 a) {
    Big_Data bd;
    bd.a = 60;

    int64[100] aa;
    aa[1] = 50;
    aa[1] += bd.a;

    int64 b = aa[1];
    int64* pb = &b;
    return 3 + 4 + a + *pb;
}

Big_Data get_big_data() {
    Big_Data bd;
    bd.a = 60;
    bd.b = 120;
    return bd;
}

float64[4] get_array_data() {
    float64[4] result = [5.0, 6.0, 7.0, 8.0];
    return result;
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

    int64 value1 = const get_array_size(123);
    write_int(value1);

    const {
        const int64 value = const get_array_size(123);
        write_int(value);

        write_int(880);
    }

    Big_Data get_struct_from_const = const get_big_data();
    write_int(get_struct_from_const.a);
    write_int(get_struct_from_const.b);

    float64[4] arraay_data = const get_array_data();
    write_float64(arraay_data[0]);
    write_float64(arraay_data[1]);
    write_float64(arraay_data[2]);
    write_float64(arraay_data[3]);


    const let Ima_Type = int;
    Ima_Type bb = 123;
    write_int(cast(int64)bb);

    const let Ima_Type_Ptr = Ima_Type*;
    Ima_Type_Ptr bb1 = &bb;
    write_int(cast(int64)*bb1);

    const let Ima_Type_Array = Ima_Type[8];
    Ima_Type_Array bb2;
    bb2[2] = bb;
    write_int(cast(int64)bb2[2]);
}
