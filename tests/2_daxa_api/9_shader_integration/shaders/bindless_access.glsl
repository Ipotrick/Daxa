// Defines basic overloads for glsl image functions like texture(...) for bindless resources.
#define DAXA_ENABLE_IMAGE_OVERLOADS_BASIC 1

#include <daxa/daxa.inl>
#include "shared.inl"

DAXA_USE_PUSH_CONSTANT(BindlessTestPush, push)

void pass_by_value(daxa_RWBufferPtr(SomeStruct) b, daxa_RWImage2Df32 i, daxa_SamplerId s)
{
    imageStore(i, ivec2(0,0), vec4(1234,0,0,0));
    deref(b).value = 1234;
}

void pass_struct_by_value(RWHandles handles)
{
    pass_by_value(handles.my_buffer, handles.my_image, handles.my_sampler);
}

layout(local_size_x = 1) in;
void main()
{
    // Copy handles into local variables:
    daxa_RWBufferPtr(SomeStruct) b = push.handles.my_buffer;
    daxa_RWImage2Df32 i = push.handles.my_image;
    daxa_SamplerId s = push.handles.my_sampler;
    // Pass handles by value to a function:
    pass_by_value(b, i, s);
    // Create a local struct containing handles:
    RWHandles handles;
    handles.my_buffer = b;
    handles.my_image = i;
    handles.my_sampler = s;
    // Pass handles inside of struct to function:
    pass_struct_by_value(handles);
    // Write handles within struct to a buffer:
    deref(push.next_shader_input) = handles;
}