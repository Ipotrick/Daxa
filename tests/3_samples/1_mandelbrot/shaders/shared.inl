#pragma once

#include "daxa/daxa.inl"

struct GpuInput
{
    daxa_f32 time;
    daxa_f32 delta_time;
};
DAXA_DECL_BUFFER_PTR(GpuInput)

struct ComputePush
{
    daxa_ImageViewId image_id;
    daxa_BufferId input_buffer_id;
    daxa_BufferPtr(GpuInput) ptr;
    daxa_u32vec2 frame_dim;
};
