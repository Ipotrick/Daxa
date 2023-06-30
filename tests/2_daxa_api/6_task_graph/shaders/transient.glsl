#extension GL_EXT_debug_printf : enable

#define DAXA_ENABLE_IMAGE_OVERLOADS_BASIC 1
#include <daxa/daxa.inl>
#include "transient.inl"

#if defined(TEST_IMAGE)

DAXA_DECL_PUSH_CONSTANT(TestImagePush, push)
layout(local_size_x = 4, local_size_y = 4, local_size_z = 4) in;
void main()
{
    const uvec3 gindex = gl_GlobalInvocationID;
    if (gindex.x < push.size.x && gindex.y < push.size.y && gindex.z < push.size.z)
    {
        const float fetch = texelFetch(daxa_texture2D(push.test_image), ivec3(gindex), 0).x;
        const bool correct = fetch == push.value;
        if (!correct)
        {
            debugPrintfEXT("ERROR! index (%u,%u,%u) contains value (%f), it should contain (%f)\n", gindex.x, gindex.y, gindex.z, fetch, push.value);
        }
    }
}

#else

DAXA_DECL_PUSH_CONSTANT(TestBufferPush, push)
layout(local_size_x = 128) in;
void main()
{
    const uint gindex = gl_GlobalInvocationID.x;
    if (gindex.x < push.size)
    {
        const uint fetch = deref(push.test_buffer[gindex]);
        const bool correct = fetch == push.value;
        if (!correct)
        {
            debugPrintfEXT("ERROR! index (%u) contains value (%u), it should contain (%u)\n", gindex.x, fetch, push.value);
        }
    }
}

#endif