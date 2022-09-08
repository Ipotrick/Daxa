#pragma once

#include <daxa/daxa.inl>

DAXA_DECL_BUFFER_STRUCT(
    FaceBuffer,
    {
        u32 data[32 * 32 * 32 * 6];
    });

struct DrawPush
{
    f32mat4x4 view_mat;
    f32vec3 chunk_pos;
    BufferId face_buffer_id;
    ImageViewId texture_array_id;
    SamplerId sampler_id;
    u32 mode;
};
