#pragma once

#include <daxa/daxa.inl>
#include <daxa/utils/task_graph.inl>

DAXA_DECL_TASK_HEAD_BEGIN(TestHead)
DAXA_TH_BUFFER_PTR(COMPUTE_SHADER_READ_WRITE, daxa::u32*, buf)
DAXA_DECL_TASK_HEAD_END

struct Test
{
    TestHead::AttachmentShaderBlob attachments;
    daxa::u32 i;
    daxa::u32* ptr0;
    Ptr<daxa::u32> ptr1;
};