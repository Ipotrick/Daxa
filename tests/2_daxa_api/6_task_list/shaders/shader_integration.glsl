#include <daxa/daxa.inl>

#extension GL_EXT_debug_printf : enable

#include "shader_integration.inl"

layout(local_size_x = 16, local_size_y = 16) in;
void main()
{
    const ivec2 index = ivec2(gl_GlobalInvocationID.xy);
    uint d = image.value;
    const float set_value = deref(settings).set_value;
    imageStore(daxa_image2D(image), index, vec4(set_value, 0, 0, 0));
    if (gl_GlobalInvocationID.x == 0 && gl_GlobalInvocationID.y == 0)
    {
        debugPrintfEXT("buffer ptr: %llx, image view id value: %u\n", daxa_u64(settings), d);
        debugPrintfEXT("value: %f\n", set_value);
        debugPrintfEXT("image: %u\n", image.value);
    }
}
