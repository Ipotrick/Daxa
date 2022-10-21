#pragma once

#include <daxa/daxa.inl>

DAXA_DECL_BUFFER_STRUCT(
    MipmappingGpuInput,
    {
        daxa_f32 mouse_x;
        daxa_f32 mouse_y;
        daxa_f32 p_mouse_x;
        daxa_f32 p_mouse_y;
        daxa_f32 paint_radius;
        daxa_f32vec3 paint_col;
    });

struct MipmappingComputePushConstant
{
    daxa_ImageViewId image_id;
    daxa_BufferId gpu_input;
    // daxa_BufferRef(MipmappingGpuInput) gpu_input;
    daxa_u32vec2 frame_dim;
};
