#include "common.hlsl"

struct Push
{
    f32mat4x4 view_mat;
    f32vec3 chunk_pos;
    BufferId face_buffer_id;
    ImageViewId texture_array_id;
    SamplerId sampler_id;
    u32 mode;
};
[[vk::push_constant]] const Push p;

DAXA_DEFINE_GET_TEXTURE2DARRAY(f32vec4)

f32vec4 main(VertexOutput vertex_output) : SV_TARGET
{
    Vertex vert;
    vert.nrm = vertex_output.nrm;
    vert.uv = vertex_output.uv;
    // vert.block_id = vertex_output.block_id;
    // vert.block_face = vertex_output.block_face;
    vert.tex_id = vertex_output.tex_id;
    f32 diffuse = dot(normalize(f32vec3(1, -3, 2)), vert.nrm) * 0.5 + 0.5;
    f32vec3 sky_col = lerp(f32vec3(0.1, 0.3, 1.0), f32vec3(1.6, 1.4, 1) * 0.75, diffuse);
    f32vec4 result = 1;

    SamplerState atlas_sampler = daxa::get_sampler(p.sampler_id);
    Texture2DArray atlas_texture_array = daxa::get_Texture2DArray<f32vec4>(p.texture_array_id);

    // f32vec4 albedo = atlas_texture_array.Load(i32vec4(vert.uv.x * 16, vert.uv.y * 16, vert.tex_id, 0));
    f32vec4 albedo = atlas_texture_array.Sample(atlas_sampler, f32vec3(vert.uv, vert.tex_id));
    if (albedo.a < 0.25)
        discard;

    f32vec3 col = 0;
    col += albedo.rgb;
    // col += vertex_output.pos * 1.0 / 32;
    // col += rand_vec3(vertex_output.pos);

    switch (p.mode)
    {
    case 0: result = f32vec4(col * diffuse * sky_col, 1.0f); break;
    case 1: result = f32vec4(col * diffuse * sky_col, 0.4f); break;
    }

    return result;
}
