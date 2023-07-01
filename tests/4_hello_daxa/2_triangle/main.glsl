#include <daxa/daxa.inl>

#include <shared.inl>

DAXA_DECL_PUSH_CONSTANT(MyPushConstant, push)

#if DAXA_SHADER_STAGE == DAXA_SHADER_STAGE_VERTEX

layout(location = 0) out daxa_f32vec3 v_col;
void main()
{
    MyVertex vert = deref(push.my_vertex_ptr[gl_VertexIndex]);
    gl_Position = daxa_f32vec4(vert.position, 1);
    v_col = vert.color;
}

#elif DAXA_SHADER_STAGE == DAXA_SHADER_STAGE_FRAGMENT

layout(location = 0) in daxa_f32vec3 v_col;
layout(location = 0) out daxa_f32vec4 color;
void main()
{
    color = daxa_f32vec4(v_col, 1);
}

#endif
