#pragma once

#include <daxa/daxa.inl>

struct DrawVertex
{
    daxa_f32 x;
    daxa_f32 y;
    daxa_f32 z;
};
DAXA_DECL_BUFFER_PTR(DrawVertex)

struct GpuInput
{
    daxa_f32mat4x4 view_mat;
};
DAXA_DECL_BUFFER_PTR(GpuInput)

struct DrawPush
{
    daxa_RWBufferPtr(GpuInput) input_buffer;
    daxa_u64 vbuffer;
};
