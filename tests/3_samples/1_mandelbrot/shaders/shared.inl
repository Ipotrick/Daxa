#pragma once

#include <daxa/daxa.inl>

DAXA_DECL_BUFFER_STRUCT(
    ComputeInput,
    {
        f32 time;
    });

struct ComputePush
{
    ImageViewId image_id;
#if DAXA_GLSL
    BufferRef(ComputeInput) compute_input;
#elif DAXA_HLSL
    BufferId input_buffer_id;
#endif
    u32vec2 frame_dim;
};
