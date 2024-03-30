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


// First argument is the heads name. Its convention to end the name with a capital H
// The secon argument is the number of attachments.
DAXA_DECL_TASK_HEAD_BEGIN(DrawToSwapchainH) 
// The following line declares an image attachment.
// The task image use is COLOR_ATTACHMENT, the required image view type is REGULAR_2D and the name is color_target.
// Note that this attachment has no suffix _ID or _INDEX. This means its NOT in the shader struct!
// This is useful to not pollute the struct with resources that cant be used in shaders like color attachments.
DAXA_TH_IMAGE(COLOR_ATTACHMENT, REGULAR_2D, color_target)
// The following line declares a buffer attachment.
// The task buffer use is VERTEX_SHADER_READ, the shader pointer type is daxa_BufferPtr(MyVertex) and the name is vertices.
DAXA_TH_BUFFER_PTR(VERTEX_SHADER_READ, daxa_BufferPtr(MyVertex), vertices)
DAXA_DECL_TASK_HEAD_END

struct MyPushConstant
{
    // This makro declares a struct of attachments in the shader and an aligned byte array in c++.
    DAXA_TH_BLOB(DrawToSwapchainH, attachments)
};
