
struct FragmentData {
    float4 color;
}

#fragment_input(FragmentData);

void main() {
    #color = #fragment_input.color;
}
