#pragma once

#include <daxa/daxa.inl>

DAXA_DECL_BUFFER_STRUCT(
    MipmappingGpuInput,
    {
        f32 mouse_x;
        f32 mouse_y;
        f32 p_mouse_x;
        f32 p_mouse_y;
        f32 paint_radius;
        f32vec3 paint_col;
    });

struct MipmappingComputePushConstant
{
    ImageViewId image_id;
    BufferId gpu_input;
    // BufferRef(MipmappingGpuInput) gpu_input;
    u32vec2 frame_dim;
};
