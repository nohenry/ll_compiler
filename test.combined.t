
varying {
    flat int a;
}

void main() {
    #postion = float4(#vertex_index & 1, #vertex_index >> 1, 0, 1);
}

// void fragment_main() {
//     #color = float4(1, 0, 0, 1);
// }