#include "raster_common.hlsl"

struct Push
{
    f32vec3 chunk_pos;
    BufferId input_buffer_id;
    BufferId face_buffer_id;
    ImageViewId texture_array_id;
    SamplerId sampler_id;
    u32 mode;
    u32 data0;
};
[[vk::push_constant]] const Push p;

DAXA_DEFINE_GET_TEXTURE2DARRAY(f32vec4)

#define SHOW_SHADOW_CASCADES 0

f32vec4 main(VertexOutput vertex_output) : SV_TARGET
{
    StructuredBuffer<Input> input = daxa::get_StructuredBuffer<Input>(p.input_buffer_id);

    Vertex vert;
    vert.nrm = vertex_output.nrm;
    vert.uv = vertex_output.uv;
    vert.pos = vertex_output.pos;
    vert.tex_id = vertex_output.tex_id;
    f32 diffuse = max(dot(normalize(f32vec3(-1, -3, -2)), vert.nrm), 0.0);
    f32vec3 sun_col = f32vec3(1.0, 0.7, 0.3) * 2;
    // f32vec3 sky_col = lerp(f32vec3(0.1, 0.3, 1.0), f32vec3(1.6, 1.4, 1) * 0.75, diffuse);
    f32vec3 sky_col = f32vec3(0.2f, 0.4f, 1.0f);
    f32vec4 result = 1;

    SamplerState atlas_sampler = daxa::get_sampler(p.sampler_id);
    Texture2DArray atlas_texture_array = daxa::get_Texture2DArray<f32vec4>(p.texture_array_id);

    f32vec4 albedo = atlas_texture_array.Sample(atlas_sampler, f32vec3(vert.uv, vert.tex_id));
    if (albedo.a < 0.25)
        discard;

    f32vec3 col = 0.1;
#if VISUALIZE_OVERDRAW
    col = 0.1;
#else
    col = albedo.rgb;

    Texture2D<f32> shadow_depth_image[3] = {
        daxa::get_Texture2D<f32>(input[0].shadow_depth_image[0]),
        daxa::get_Texture2D<f32>(input[0].shadow_depth_image[1]),
        daxa::get_Texture2D<f32>(input[0].shadow_depth_image[2]),
    };
    f32vec4 shadow_pos[3] = {
        mul(input[0].shadow_view_mat[0], f32vec4(vertex_output.pos + vert.nrm * (80.0 / SHADOW_RES), 1)),
        mul(input[0].shadow_view_mat[1], f32vec4(vertex_output.pos + vert.nrm * (60.0 / SHADOW_RES), 1)),
        mul(input[0].shadow_view_mat[2], f32vec4(vertex_output.pos + vert.nrm * (20.0 / SHADOW_RES), 1)),
    };
    shadow_pos[0].xyz *= 1.0 / shadow_pos[0].w;
    shadow_pos[1].xyz *= 1.0 / shadow_pos[1].w;
    shadow_pos[2].xyz *= 1.0 / shadow_pos[2].w;
    f32vec2 depth_uv[3] = {
        shadow_pos[0].xy * 0.5 + 0.5,
        shadow_pos[1].xy * 0.5 + 0.5,
        shadow_pos[2].xy * 0.5 + 0.5,
    };
    f32 shadow_depth[3] = {
        shadow_depth_image[0].Load(i32vec3(depth_uv[0] * SHADOW_RES, 0)),
        shadow_depth_image[1].Load(i32vec3(depth_uv[1] * SHADOW_RES, 0)),
        shadow_depth_image[2].Load(i32vec3(depth_uv[2] * SHADOW_RES, 0)),
    };

    b32 shadow_nearclipped[3] = {
        (shadow_depth[1] * 4 - 0.7) < 0.01,
        (shadow_depth[2] * 4 - 0.7) < 0.01,
        false,
    };
    shadow_nearclipped[0] = shadow_nearclipped[0] || shadow_nearclipped[1];

    for (u32 cascade_i = 0; cascade_i < 3; ++cascade_i)
    {
        if (!shadow_nearclipped[cascade_i] &&
            shadow_pos[cascade_i].x > -1 && shadow_pos[cascade_i].x < 1 &&
            shadow_pos[cascade_i].y > -1 && shadow_pos[cascade_i].y < 1 &&
            shadow_pos[cascade_i].z > -1 && shadow_pos[cascade_i].z < 1)
        {
            f32 shade = shadow_depth[cascade_i] + (0.4 / SHADOW_RES) > shadow_pos[cascade_i].z;
            diffuse *= shade;
            // col = shadow_depth[cascade_i];

#if SHOW_SHADOW_CASCADES
            if (frac(vert.pos.x + vert.pos.y + vert.pos.z + 0.0001) < 0.5)
                col = f32vec3(0.0, 0.5, 0.0);
#endif
            break;
        }
    }

    diffuse = max(diffuse, 0.0);
    col = col * (sun_col * diffuse + sky_col * 0.5);
    // col = vert.nrm;
#endif

    switch (p.mode)
    {
    case 0: result = f32vec4(col, 1.0f); break;
    case 1: result = f32vec4(col, 0.4f); break;
    }

    return result;
}
