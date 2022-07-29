#include "common.hlsl"

struct Push
{
    float4x4 view_mat;
    float3 chunk_pos;
    daxa::BufferId face_buffer_id;
    daxa::ImageId texture_array_id;
    daxa::SamplerId sampler_id;
    uint mode;
};
[[vk::push_constant]] const Push p;

DAXA_DEFINE_GET_TEXTURE2DARRAY(float4)

float4 main(VertexOutput vertex_output) : SV_TARGET
{
    Vertex vert;
    vert.nrm = vertex_output.nrm;
    vert.uv = vertex_output.uv;
    // vert.block_id = vertex_output.block_id;
    // vert.block_face = vertex_output.block_face;
    vert.tex_id = vertex_output.tex_id;
    float diffuse = dot(normalize(float3(1, -3, 2)), vert.nrm) * 0.5 + 0.5;
    float3 sky_col = lerp(float3(0.1, 0.3, 1.0), float3(1.6, 1.4, 1) * 0.75, diffuse);
    float4 result = 1;

    SamplerState atlas_sampler = daxa::get_sampler(p.sampler_id);
    Texture2DArray atlas_texture_array = daxa::get_Texture2DArray<float4>(p.texture_array_id);

    // float4 albedo = atlas_texture_array.Load(int4(vert.uv.x * 16, vert.uv.y * 16, vert.tex_id, 0));
    float4 albedo = atlas_texture_array.Sample(atlas_sampler, float3(vert.uv, vert.tex_id));

    float3 col = albedo.rgb;

    switch (p.mode)
    {
    case 0: result = float4(col * diffuse * sky_col, 1.0f); break;
    case 1: result = float4(col * diffuse * sky_col, 0.4f); break;
    }

    return result;
}
