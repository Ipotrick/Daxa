#pragma once

#include <daxa/daxa.inl>

DAXA_DECL_BUFFER_STRUCT(
    MipmappingComputeInput,
    {
        f32 mouse_x;
        f32 mouse_y;
        f32 p_mouse_x;
        f32 p_mouse_y;
        f32vec3 paint_col;
    });

struct MipmappingComputePushConstant
{
    ImageViewId image_id;
    BufferRef(MipmappingComputeInput) compute_input;
    u32vec2 frame_dim;
};
