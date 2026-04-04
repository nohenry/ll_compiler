#version 460
// https://www.vincentparizet.com/blog/posts/vulkan_bindless_descriptors/
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_scalar_block_layout : require
// GL_EXT_buffer_reference2 adds pointer math support with addresses.
// GL_ARB_gpu_shader_int64 and GL_EXT_shader_explicit_arithmetic_types_int64 allows to use uint64_t in GLSL. You can cast addresses to uint64_t and cast uint64_t to any buffer type.
// GL_EXT_buffer_reference_uvec2 allows to do arithmetics on addresses with uvec2 for devices that don’t support uint64_t.

// forward declaration
layout(buffer_reference) buffer blockType;
layout(buffer_reference) buffer blockType1;

// complete reference type definition
layout(buffer_reference, std140, buffer_reference_align = 16) buffer blockType {
    int x;
    int y;
    vec4 f1;
    vec2 f2;
    int e;
    // vec4 a;
    // blockType next;
};

layout(buffer_reference, std430, buffer_reference_align = 16) buffer blockType1 {
    int x;
    int y;
    vec3 f1;
    vec2 f2;
    int e;
    // vec4 a;
    // blockType next;
};

// A normal block declaration that includes a reference to blockType
// layout(set = 4, binding = 0) buffer rootBlock {
//     bool b;
//     blockType root;
// } r;

layout(push_constant, scalar) uniform rootBlock {
    bool b;
    blockType root1;
    blockType1 root2;
} r;

// int function_array(int f[5]) {
//     return f[4];
// }

void main()
{
    blockType b = r.root1;
    blockType1 b1 = r.root2;
    int i = gl_VertexIndex;
    gl_Position = vec4(1, b1.x, 3, 4);
    // b = b.next.next.next.next.next;

    // int j = int(r.b);
    // int a[] = {1, 2, 3, 4, 5};
    // int c[3][5] = {{1, 2, 3, 4, 5}, {6, 7, 8, 9, 10}, {11, 12, 13, 14, 15}};
    // function_array(a) + c[2][3];

    // b += bbb;
    // ...
    // use b.x;
}
// void main() {
//     blockType b = r.root;
// }