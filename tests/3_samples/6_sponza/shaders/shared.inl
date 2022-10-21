#pragma once

#include <daxa/daxa.inl>

DAXA_DECL_BUFFER_STRUCT(
    DrawVertex,
    {
        daxa_f32 x;
        daxa_f32 y;
        daxa_f32 z;
    });

DAXA_DECL_BUFFER_STRUCT(
    GpuInput,
    {
        daxa_f32mat4x4 view_mat;
    });

struct DrawPush
{
    daxa_BufferRef(GpuInput) input_buffer;
    daxa_u64 vbuffer;
};
