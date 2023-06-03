#pragma once

#include <daxa/daxa.inl>

struct DrawVertex
{
    daxa_f32vec4 pos, col;
};

struct DrawVertexBuffer
{
    DrawVertex verts[3];
};
DAXA_DECL_BUFFER_PTR(DrawVertexBuffer)

struct DrawPush
{
    daxa_BufferPtr(DrawVertexBuffer) face_buffer;
};
