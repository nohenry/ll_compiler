
// varying {
//     flat int a;
// }

struct Vertex {
    uint32 m;
}

void main(uint32 m) {
    #position = float4(#vertex_index & 1, (#vertex_index >> 1) * m, 0, 1);
}