#version 450 core

#include <shared.inl>

DAXA_PUSH_CONSTANT(DrawPush)

#if defined(DRAW_VERT)

layout(location = 0) out f32vec4 v_col;
void main()
{
    DrawVertexBuffer vertex_buffer = daxa_GetBuffer(DrawVertexBuffer, daxa_push.vertex_buffer_id);
    DrawVertex vert = vertex_buffer.verts[0];
    gl_Position = f32vec4(vert.pos.xy, 0, 1);
    v_col = vert.col;
}

#elif defined(DRAW_FRAG)

layout(location = 0) in f32vec4 v_col;
void main()
{
    gl_FragColor = f32vec4(v_col.rgb, 1);
}

#endif
