
// varying {
//     flat int a;
// }

void main() {
    #position = float4(cast(float32) (#vertex_index & 1), cast(float32) (#vertex_index >> 1), 0, 1);
}