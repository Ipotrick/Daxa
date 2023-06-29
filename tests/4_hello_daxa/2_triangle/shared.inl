#pragma once

// Includes the daxa api to the shader
#include <daxa/daxa.inl>

struct MyVertex
{
    daxa_f32vec3 position;
    daxa_f32vec3 color;
};

// Allows the shader to use pointers to MyVertex
DAXA_DECL_BUFFER_PTR(MyVertex)

struct MyPushConstant
{
    daxa_BufferPtr(MyVertex) my_vertex_ptr;
};
