#pragma once

// Includes the Daxa API to the shader
#include <daxa/daxa.inl>
#include <daxa/utils/task_graph.inl>

struct MyVertex
{
    daxa_f32vec3 position;
    daxa_f32vec3 color;
};

// Allows the shader to use pointers to MyVertex
DAXA_DECL_BUFFER_PTR(MyVertex)

struct MyPushConstant
{
    daxa_BufferPtr(MyVertex) vertices;
};
