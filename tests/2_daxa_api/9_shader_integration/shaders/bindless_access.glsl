#include <daxa/daxa.inl>
#include "shared.inl"

DAXA_DECL_PUSH_CONSTANT(BindlessTestPush, push)

void pass_by_value(daxa_RWBufferPtr(SomeStruct) b, daxa_ImageViewId i, daxa_SamplerId s)
{
    // Any image id can be cast to a glsl resource, by naming the glsl resource with the daxa_ prefix.
    ivec2 size = imageSize(daxa_image2D(i));
    imageStore(daxa_image2D(i), ivec2(0,0), vec4(1234,0,0,0));
    deref(b).value = 1234;
}

void pass_struct_by_value(RWHandles handles)
{
    pass_by_value(handles.my_buffer, handles.my_image, handles.my_sampler);
}

layout(local_size_x = 1) in;
void main()
{
    // Buffer ptr is a predefined buffer reference with one field called value of the given struct type.
    daxa_RWBufferPtr(SomeStruct) b = push.handles.my_buffer;
    // Ids can be passed as local variables
    daxa_ImageViewId i = push.handles.my_image;
    daxa_SamplerId s = push.handles.my_sampler;
    // Ids can be passed like any other local variables, as they are normal structs.
    pass_by_value(b, i, s);
    RWHandles handles;
    handles.my_buffer = b;
    handles.my_image = i;
    handles.my_sampler = s;
    pass_struct_by_value(handles);
    // The following like is equivalent '''to push.next_shader_input.value = handles;'''
    deref(push.next_shader_input) = handles;
}