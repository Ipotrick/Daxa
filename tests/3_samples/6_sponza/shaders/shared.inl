#pragma once

#include <daxa/daxa.inl>

DAXA_DECL_BUFFER_STRUCT(
    DrawVertex,
    {
        f32 x;
        f32 y;
        f32 z;
    });

DAXA_DECL_BUFFER_STRUCT(
    GpuInput,
    {
        f32mat4x4 view_mat;
    });

struct DrawPush
{
    BufferRef(GpuInput) input_buffer;
    u64 vbuffer;
};
