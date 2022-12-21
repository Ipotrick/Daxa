#pragma once

#include <daxa/daxa.inl>

struct DrawVertex
{
    daxa_f32vec4 pos, col;
};

#if DAXA_SHADERLANG == DAXA_SHADERLANG_GLSL
struct DrawVertexBuffer
{
    DrawVertex verts[3];
};
DAXA_ENABLE_BUFFER_PTR(DrawVertexBuffer)
#elif DAXA_SHADERLANG == DAXA_SHADERLANG_HLSL
DAXA_DECL_BUFFER_STRUCT(
    DrawVertexBuffer,
    {
        DrawVertex verts[3];
    })
#endif

struct DrawPush
{
#if DAXA_SHADERLANG == DAXA_SHADERLANG_GLSL
    daxa_RWBufferPtr(DrawVertexBuffer) face_buffer;
#elif DAXA_SHADERLANG == DAXA_SHADERLANG_HLSL
    daxa_BufferId vertex_buffer_id;
#endif
    // u32vec2 frame_dim;
};
