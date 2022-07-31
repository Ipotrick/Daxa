#include "raster_common.hlsl"

struct Push
{
    float3 chunk_pos;
    daxa::BufferId input_buffer_id;
    daxa::BufferId face_buffer_id;
    daxa::ImageId texture_array_id;
    daxa::SamplerId sampler_id;
    uint mode;
    uint data0;
};
[[vk::push_constant]] const Push p;

DAXA_DEFINE_GET_TEXTURE2DARRAY(float4)

#define SHOW_SHADOW_CASCADES 0

float4 main(VertexOutput vertex_output) : SV_TARGET
{
    StructuredBuffer<Input> input = daxa::get_StructuredBuffer<Input>(p.input_buffer_id);

    Vertex vert;
    vert.nrm = vertex_output.nrm;
    vert.uv = vertex_output.uv;
    vert.pos = vertex_output.pos;
    vert.tex_id = vertex_output.tex_id;
    float diffuse = max(dot(normalize(float3(-1, -3, -2)), vert.nrm), 0.0);
    float3 sun_col = float3(1.0, 0.7, 0.3) * 2;
    // float3 sky_col = lerp(float3(0.1, 0.3, 1.0), float3(1.6, 1.4, 1) * 0.75, diffuse);
    float3 sky_col = float3(0.2f, 0.4f, 1.0f);
    float4 result = 1;

    SamplerState atlas_sampler = daxa::get_sampler(p.sampler_id);
    Texture2DArray atlas_texture_array = daxa::get_Texture2DArray<float4>(p.texture_array_id);

    float4 albedo = atlas_texture_array.Sample(atlas_sampler, float3(vert.uv, vert.tex_id));
    if (albedo.a < 0.25)
        discard;

    float3 col = 0.1;
    // col = albedo.rgb;

    Texture2D<float> shadow_depth_image0 = daxa::get_Texture2D<float>(input[0].shadow_depth_image[0]);
    float4 shadow_pos0 = mul(input[0].shadow_view_mat[0], float4(vertex_output.pos + vert.nrm * (80.0 / SHADOW_RES), 1));
    shadow_pos0.xyz *= 1.0 / shadow_pos0.w;
    float2 depth_uv0 = shadow_pos0.xy * 0.5 + 0.5;
    float shadow_depth0 = shadow_depth_image0.Load(int3(depth_uv0 * SHADOW_RES, 0));

    Texture2D<float> shadow_depth_image1 = daxa::get_Texture2D<float>(input[0].shadow_depth_image[1]);
    float4 shadow_pos1 = mul(input[0].shadow_view_mat[1], float4(vertex_output.pos + vert.nrm * (60.0 / SHADOW_RES), 1));
    shadow_pos1.xyz *= 1.0 / shadow_pos1.w;
    float2 depth_uv1 = shadow_pos1.xy * 0.5 + 0.5;
    float shadow_depth1 = shadow_depth_image1.Load(int3(depth_uv1 * SHADOW_RES, 0));

    Texture2D<float> shadow_depth_image2 = daxa::get_Texture2D<float>(input[0].shadow_depth_image[2]);
    float4 shadow_pos2 = mul(input[0].shadow_view_mat[2], float4(vertex_output.pos + vert.nrm * (20.0 / SHADOW_RES), 1));
    shadow_pos2.xyz *= 1.0 / shadow_pos2.w;
    float2 depth_uv2 = (shadow_pos2.xy * 0.5 + 0.5);
    float shadow_depth2 = shadow_depth_image2.Load(int3(depth_uv2 * SHADOW_RES, 0));

    bool shadow0_nearclipped = (shadow_depth1 * 4 - 0.7) < 0.01;
    bool shadow1_nearclipped = (shadow_depth2 * 4 - 0.7) < 0.01;

    if (!shadow1_nearclipped && !shadow0_nearclipped && shadow_pos0.x > -1 && shadow_pos0.x < 1 && shadow_pos0.y > -1 && shadow_pos0.y < 1 && shadow_pos0.z > -1 && shadow_pos0.z < 1)
    {
        float shade = shadow_depth0 + (0.4 / SHADOW_RES) > shadow_pos0.z;
        diffuse *= shade;
        // col = shadow_depth0;

#if SHOW_SHADOW_CASCADES
        if (frac(vert.pos.x + vert.pos.y + vert.pos.z + 0.0001) < 0.5)
            col = float3(0.0, 0.5, 0.0);
#endif
    }
    else if (!shadow1_nearclipped && shadow_pos1.x > -1 && shadow_pos1.x < 1 && shadow_pos1.y > -1 && shadow_pos1.y < 1 && shadow_pos1.z > -1 && shadow_pos1.z < 1)
    {
        float shade = shadow_depth1 + (0.6 / SHADOW_RES) > shadow_pos1.z;
        diffuse *= shade;
        // col = shadow_depth1 * 4 - 0.7;

#if SHOW_SHADOW_CASCADES
        if (frac(vert.pos.x + vert.pos.y + vert.pos.z + 0.0001) < 0.5)
            col = float3(0.5, 0.5, 0.0);
#endif
    }
    else if (shadow_pos2.x > -1 && shadow_pos2.x < 1 && shadow_pos2.y > -1 && shadow_pos2.y < 1 && shadow_pos2.z > -1 && shadow_pos2.z < 1)
    {
        float shade = shadow_depth2 + (0.8 / SHADOW_RES) > shadow_pos2.z;
        diffuse *= shade;
        // col = (shadow_depth2 * 4 - 0.7) * 4 - 0.7;

#if SHOW_SHADOW_CASCADES
        if (frac(vert.pos.x + vert.pos.y + vert.pos.z + 0.0001) < 0.5)
            col = float3(0.5, 0.0, 0.0);
#endif
    }
    else
    {
#if SHOW_SHADOW_CASCADES
        if (frac(vert.pos.x + vert.pos.y + vert.pos.z + 0.0001) < 0.5)
            col = float3(0.0, 0.0, 0.0);
#endif
    }

    // diffuse = max(diffuse, 0.0);
    // col = col * (sun_col * diffuse + sky_col * 0.5);
    // col = vert.nrm;

    switch (p.mode)
    {
    case 0: result = float4(col, 1.0f); break;
    case 1: result = float4(col, 0.4f); break;
    }

    return result;
}
