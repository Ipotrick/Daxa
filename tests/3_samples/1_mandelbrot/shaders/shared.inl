#pragma once

#include <daxa/daxa.inl>

DAXA_DECL_BUFFER_STRUCT(
    GpuInput,
    {
        daxa_f32 time;
        daxa_f32 delta_time;
    });

struct ComputePush
{
    daxa_ImageViewId image_id;
#if DAXA_SHADERLANG == DAXA_SHADERLANG_GLSL
    daxa_BufferRef(GpuInput) gpu_input;
#elif DAXA_SHADERLANG == DAXA_SHADERLANG_HLSL
    daxa_BufferId input_buffer_id;
#endif
    daxa_u32vec2 frame_dim;
};
