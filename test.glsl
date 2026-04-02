#version 460
// https://www.vincentparizet.com/blog/posts/vulkan_bindless_descriptors/
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_buffer_reference2 : require
// GL_EXT_buffer_reference2 adds pointer math support with addresses.
// GL_ARB_gpu_shader_int64 and GL_EXT_shader_explicit_arithmetic_types_int64 allows to use uint64_t in GLSL. You can cast addresses to uint64_t and cast uint64_t to any buffer type.
// GL_EXT_buffer_reference_uvec2 allows to do arithmetics on addresses with uvec2 for devices that don’t support uint64_t.

// forward declaration
layout(buffer_reference) buffer blockType;

// complete reference type definition
layout(buffer_reference, std430, buffer_reference_align = 16) buffer blockType {
    int x;
    blockType next;
};

// A normal block declaration that includes a reference to blockType
layout(set = 4, binding = 0) buffer rootBlock {
    bool b;
    blockType root;
} r;

void main()
{
    int i;
    if (true) {
        i = 123;
    } else {
        i = 10;
    }
    for (int i = 0; i < 123; i++) {
        i += 34;
    }
    blockType b = r.root;
    // "pointer chasing" through a linked list
    b = b.next.next.next.next.next;
    b += 12;
    bool bbbb = r.b;
    // b += bbb;
    // ...
    // use b.x;
}