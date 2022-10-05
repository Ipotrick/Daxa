#pragma once

#include <daxa/daxa.inl>

#if defined(__cplusplus)
using namespace daxa::types;
#endif

DAXA_DECL_BUFFER_STRUCT(
    GpuInput,
    {
        f32 time;
        f32 delta_time;
    });

struct ComputePush
{
    ImageViewId image_id;
#if DAXA_SHADERLANG == DAXA_SHADERLANG_GLSL
    BufferRef(GpuInput) gpu_input;
#elif DAXA_SHADERLANG == DAXA_SHADERLANG_HLSL
    BufferId input_buffer_id;
#endif
    u32vec2 frame_dim;
};
