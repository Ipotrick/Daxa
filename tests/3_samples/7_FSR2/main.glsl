#include <shared.inl>

#include <utils/voxels.glsl>

DAXA_DECL_PUSH_CONSTANT(DrawPush)

#if DAXA_SHADER_STAGE == DAXA_SHADER_STAGE_VERTEX

layout(location = 0) out f32vec3 v_nrm;
layout(location = 1) out f32vec3 v_tex_uv;
layout(location = 2) out f32vec2 v_mv;

void main()
{
    UnpackedFace vert = get_vertex(daxa_push_constant.packed_faces_ptr, gl_VertexIndex);

    gl_Position = deref(daxa_push_constant.perframe_input_ptr).view_mat * f32vec4(vert.pos.xyz + daxa_push_constant.chunk_pos, 1);
    f32vec4 prev_pos = deref(daxa_push_constant.perframe_input_ptr).prev_view_mat * f32vec4(vert.pos.xyz + daxa_push_constant.chunk_pos, 1);
    v_tex_uv = f32vec3(vert.uv, vert.tex_id);
    v_nrm = vert.nrm;

    v_mv = f32vec2((gl_Position.xy / gl_Position.w - prev_pos.xy / prev_pos.w) - deref(daxa_push_constant.perframe_input_ptr).jitter) * -0.5;
}

#elif DAXA_SHADER_STAGE == DAXA_SHADER_STAGE_FRAGMENT

layout(location = 0) in f32vec3 v_nrm;
layout(location = 1) in f32vec3 v_tex_uv;
layout(location = 2) in f32vec2 v_mv;

layout(location = 0) out f32vec4 color;
layout(location = 1) out f32vec4 mv;

void main()
{
    f32vec4 tex_col = texture(daxa_push_constant.atlas_texture, daxa_push_constant.atlas_sampler, v_tex_uv);
    f32vec3 col = tex_col.rgb;
    col *= max(dot(normalize(v_nrm), normalize(f32vec3(1, -3, 2))) * 0.5 + 0.5, 0.0);

    mv = f32vec4(v_mv, 0, 0);

    color = f32vec4(col, 1);
}

#endif
