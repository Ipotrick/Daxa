#pragma once

#include <daxa/daxa.inl>

struct DrawVertex
{
    f32vec4 pos, col;
};

DAXA_DECL_BUFFER_STRUCT(
    DrawVertexBuffer,
    {
        DrawVertex verts[3];
    });

struct DrawPush
{
    BufferId vertex_buffer_id;
    // u32vec2 frame_dim;
};
