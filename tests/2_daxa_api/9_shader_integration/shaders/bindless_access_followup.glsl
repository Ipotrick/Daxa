#extension GL_EXT_debug_printf : enable

#include <daxa/daxa.inl>
#include "shared.inl"

DAXA_DECL_PUSH_CONSTANT(BindlessTestFollowPush, push)

void pass_by_value(daxa_BufferPtr(SomeStruct) b, daxa_ImageViewId i, daxa_SamplerId s)
{
    const float buf_fetch = deref(b).value;
    // Ids can also be cast to glsl resources like textureXX and samplerXX.
    const float tex_sample = texture(daxa_sampler2D(i,s),vec2(0,0)).r;
    const float tex_fetch = texelFetch(daxa_texture2D(i), ivec2(0,0), 0).r;
    debugPrintfEXT("buffer fetch: %f, texture sample: %f, texture fetch %f\n, ", buf_fetch, tex_sample, tex_fetch);
}

void pass_struct_by_value(Handles handles)
{
    // pass handles as values into a function:
    pass_by_value(handles.my_buffer, handles.my_image, handles.my_sampler);
}

layout(local_size_x = 1) in;
void main()
{
    // read resource handles from buffer:
    Handles handles = deref(push.shader_input);
    // pass handles as struct into a function:
    pass_struct_by_value(handles);
}