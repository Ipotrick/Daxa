#include <daxa/daxa.inl>

#include <shared.inl>

DAXA_DECL_PUSH_CONSTANT(MyPushConstant, push)

#if DAXA_SHADER_STAGE == DAXA_SHADER_STAGE_VERTEX

layout(location = 0) out daxa_f32vec3 v_col;
void main()
{
    daxa_BufferPtr(MyVertex) vertices_ptr = push.vertices;
    // Daxa provides convenience functions to deref the i'th element for each buffer ptr:
    MyVertex vert = deref_i(push.vertices, gl_VertexIndex);
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
