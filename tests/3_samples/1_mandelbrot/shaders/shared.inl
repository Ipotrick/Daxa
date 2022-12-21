#pragma once

#include <daxa/daxa.inl>

#if DAXA_SHADERLANG == DAXA_SHADERLANG_GLSL
struct GpuInput
{
    daxa_f32 time;
    daxa_f32 delta_time;
};
DAXA_ENABLE_BUFFER_PTR(GpuInput)
#elif DAXA_SHADERLANG == DAXA_SHADERLANG_HLSL
DAXA_DECL_BUFFER_STRUCT(
    GpuInput,
    {
        daxa_f32 time;
        daxa_f32 delta_time;
    })
#endif
struct ComputePush
{
    daxa_ImageViewId image_id;
#if DAXA_SHADERLANG == DAXA_SHADERLANG_GLSL
    daxa_RWBufferPtr(GpuInput) gpu_input;
#elif DAXA_SHADERLANG == DAXA_SHADERLANG_HLSL
    daxa_BufferId input_buffer_id;
#endif
    daxa_u32vec2 frame_dim;
};
