
// varying {
//     flat int a;
// }

void main() {
    #position = float4(#vertex_index & 1, #vertex_index >> 1, 0, 1);
}