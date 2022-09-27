#version 450 core

#include <shared.inl>

DAXA_USE_PUSH_CONSTANT(DrawPush)

#if defined(DRAW_VERT)

layout(location = 0) out f32vec4 v_col;
void main()
{
    DrawVertex vert = push_constant.face_buffer.verts[gl_VertexIndex];
    gl_Position = f32vec4(vert.pos.xy, 0, 1);
    v_col = vert.col;
}

#elif defined(DRAW_FRAG)

layout(location = 0) in f32vec4 v_col;
layout(location = 0) out f32vec4 color;
void main()
{
    color = v_col;
}

#endif
