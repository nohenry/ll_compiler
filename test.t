// let d = a;
// let a = 123;
// let b = a;
// let c = b;

float4 get_value(float a, float b) {
    return float4(1, a, b, 2);
}

// void main() {
//     float4 f;
//     float2 f2 = float2(8, 9);
//     float4 f1 = float4(1, f2, 4);
//     float4 aaa = f + f1;
//     float4 aba = get_value(100, 200);
//     float4 b = f1.rgba;

// }


// void main() {
//     float f = 1.0;
//     int i = cast(int)f;
//     int64 ii = i;
//     uint64 uii = cast(uint64)ii;

//     bool8 b = true;
//     if b {
//         ii = 4;
//     } else {
//         ii = 19;
//     }

//     for int i = 0; i < 100; i += 1 {
//         ii += 34;
//         if i == 34 {
//             break;
//         }
//     }

// }

struct BarBar {
    int8 a;
    float b;
    float4 c; 
    // float4[3] d; 
    // bool64 b64;
}

struct SmallStruct {
    int struc;
}

struct Foobar {
    int i;
    int j;
    // SmallStruct s;
    BarBar bar;
}

struct StructWithPtr {
    int jj;
    int kk;
    Foobar* foobar;
}

// void test1() {
//     int[5][3] a = [[1, 2, 3, 4, 5], [8, 9, 10, 11, 12], [13, 14, 15, 16, 17]];
//     a[2][4] = 50;
//     // int ccc = a.length;
//     // int value = a[2][3];
//     // Foobar b;
//     // int8 value1 = b.bar.a;
//     // float4 f = b.bar.d[2];
//     // float4[3] ddd = b.bar.d;

//     int i = 34;
//     int* p = &i;
//     int j = *p;
//     *p = 100;

//     int[8]*[3] kk;
//     kk[2][3][4];
//     kk[2][3][4] = 8;

//     Foobar[8]*[3] f;
//     f[10][20][30].bar.d[3];
//     f[10][20][30].bar.d[3] = float4(1, 2, 3, 4);
//     f[10][20][30].bar.d[3] = float4(1, 2, 3, 4);
// }

// void test2() {
//     Foobar[8] f1;
//     Foobar[8]*[3] f;
//     *f[2] = f1;

//     Foobar* f2;
//     Foobar** f3;
//     *f3 = f2;
//     f2.j = 100;

//     int i = 123;
//     (&i)[100] = 4;

//     StructWithPtr[3] struct_with_ptr;
//     struct_with_ptr[2].foobar.bar.b64;
// }

struct Vertex {
    float3 position;
}

struct FragmentData {
    float4 color;
}

FragmentData main(Vertex* vertex_buffer) {
    #position = vertex_buffer[#vertex_index].position.xyz1;
    // Vertex v = *vertex_buffer;

    FragmentData fd; 
    fd.color = float4(1, 0, 1, 1);
    return fd;
}
