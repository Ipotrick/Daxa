#include <daxa/daxa.inl>

#include <shared.inl>

layout(push_constant, scalar) uniform PushBlock
{
    MyPushConstant push;
};

#if defined(DRAW_VERT)

layout(location = 0) out daxa_f32vec3 v_col;
void main()
{
    MyVertex vert = deref(push.my_vertex_ptr[gl_VertexIndex]);
    gl_Position = daxa_f32vec4(vert.position, 1);
    v_col = vert.color;
}

#elif defined(DRAW_FRAG)

layout(location = 0) in daxa_f32vec3 v_col;
layout(location = 0) out daxa_f32vec4 color;
void main()
{
    color = daxa_f32vec4(v_col, 1);
}

#endif
