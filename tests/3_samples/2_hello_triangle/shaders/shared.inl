#pragma once

#include <daxa/daxa.inl>

struct DrawVertex
{
    f32vec4 pos, col;
};
struct DrawVertexBuffer
{
    DrawVertex verts[3];
};
DAXA_REGISTER_STRUCT_GET_BUFFER(DrawVertexBuffer)

struct DrawPush
{
    BufferId vertex_buffer_id;
    u32vec2 frame_dim;
};
