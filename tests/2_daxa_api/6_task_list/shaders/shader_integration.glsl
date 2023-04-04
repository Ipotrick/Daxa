#define DAXA_ENABLE_IMAGE_OVERLOADS_BASIC 1
#include <daxa/daxa.inl>

#extension GL_EXT_debug_printf : enable

#include "shader_integration.inl"

layout(local_size_x = 16, local_size_y = 16) in;
void main()
{
    debugPrintfEXT("buffer ptr: %ull\n", daxa_u64(settings));
    debugPrintfEXT("image: %u\n", image.id.value);
    const ivec3 index = ivec3(gl_GlobalInvocationID);
    const float set_value = deref(settings).set_value;
    imageStore(image, index, vec4(set_value,0,0,0));
    debugPrintfEXT("set_value: %f\n", set_value);
}