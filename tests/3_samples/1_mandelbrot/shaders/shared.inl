#pragma once

#include <daxa/daxa.inl>

DAXA_DECL_BUFFER_STRUCT(
    GpuInput,
    {
        DAXA_NAMESPACE(f32) time;
        DAXA_NAMESPACE(f32) delta_time;
    });

struct ComputePush
{
    DAXA_NAMESPACE(ImageViewId) image_id;
#if DAXA_SHADERLANG == DAXA_SHADERLANG_GLSL
    DAXA_NAMESPACE(BufferRef)(GpuInput) gpu_input;
#elif DAXA_SHADERLANG == DAXA_SHADERLANG_HLSL
    DAXA_NAMESPACE(BufferId) input_buffer_id;
#endif
    DAXA_NAMESPACE(u32vec2) frame_dim;
};
