#pragma once

#include <daxa/daxa.inl>

struct ComputeInput
{
    f32 time;
};
DAXA_REGISTER_STRUCT_GET_BUFFER(ComputeInput)

struct ComputePush
{
    ImageViewId image_id;
    BufferId input_buffer_id;
    u32vec2 frame_dim;
};
