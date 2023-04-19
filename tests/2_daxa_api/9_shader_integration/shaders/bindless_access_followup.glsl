// Defines basic overloads for glsl image functions like texture(...) for bindless resources.
#define DAXA_ENABLE_IMAGE_OVERLOADS_BASIC 1

#extension GL_EXT_debug_printf : enable

#include <daxa/daxa.inl>
#include "shared.inl"

DAXA_USE_PUSH_CONSTANT(BindlessTestFollowPush, push)

void pass_by_value(daxa_BufferPtr(SomeStruct) b, daxa_Image2Df32 i, daxa_SamplerId s)
{
    // use bindless handles to load values:
    const float buf_fetch = deref(b).value;
    const float img_fetch = texture(i,s,vec2(0,0)).r;
    debugPrintfEXT("buffer fetch: %f, image fetch: %f\n", buf_fetch, img_fetch);
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