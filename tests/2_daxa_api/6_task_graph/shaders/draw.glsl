#define DAXA_ENABLE_SHADER_NO_NAMESPACE 1
#include <shared.inl>

DAXA_DECL_PUSH_CONSTANT(DrawPush)

#if DAXA_SHADER_STAGE == DAXA_SHADER_STAGE_VERTEX



layout(location = 0) out f32vec4 v_col;
void main()
{
    DrawVertex vert = daxa_push_constant.vertex_buffer[gl_VertexIndex].value;
    gl_Position = f32vec4(vert.pos.xy, 0, 1);
    vec4 value = imageLoad(daxa_image2D(tex));
    vec4 value = textureFetch(daxa_texture2D(tex));

    v_col = vert.col;
}

#elif DAXA_SHADER_STAGE == DAXA_SHADER_STAGE_FRAGMENT

layout(location = 0) in f32vec4 v_col;
layout(location = 0) out f32vec4 color;
void main()
{
    color = v_col;
}

#endif
