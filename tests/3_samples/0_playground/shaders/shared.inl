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
    BufferRef(DrawVertexBuffer) face_buffer;
};
