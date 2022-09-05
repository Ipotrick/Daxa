#pragma once

#include <daxa/daxa.inl>

DAXA_DECL_BUFFER_STRUCT(
    ComputeInput,
    {
        f32 time;
    }
);

struct ComputePush
{
    ImageViewId image_id;
    BufferId input_buffer_id;
    u32vec2 frame_dim;
};
