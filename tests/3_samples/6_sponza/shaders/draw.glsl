#version 450 core

#include <shared.inl>

DAXA_USE_PUSH_CONSTANT(DrawPush)

#if defined(DRAW_VERT)

layout(location = 0) out f32vec4 v_col;
void main()
{
    DrawVertex vert = daxa_buffer_address_to_ref(DrawVertex, WrappedBufferRef, push_constant.vbuffer + gl_VertexIndex * (4 * 3)).value;
    f32vec3 vert_pos = f32vec3(vert.x, vert.y, vert.z);
    gl_Position = push_constant.input_buffer.view_mat * f32vec4(vert_pos, 1);
    v_col = f32vec4(1, 0, 1, 1);
}

#elif defined(DRAW_FRAG)

layout(location = 0) in f32vec4 v_col;
layout(location = 0) out f32vec4 color;
void main()
{
    color = f32vec4(v_col.rgb, 1);
}

#endif
