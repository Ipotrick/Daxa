#pragma once

#include <daxa/daxa.inl>

struct GpuInput
{
    daxa_f32 time;
    daxa_f32 delta_time;
};
DAXA_ENABLE_BUFFER_PTR(GpuInput)
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
