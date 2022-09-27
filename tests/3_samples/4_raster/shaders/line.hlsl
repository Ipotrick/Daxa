#include "common.hlsl"

[[vk::push_constant]] const DrawPush p;

DAXA_DEFINE_GET_TEXTURE2DARRAY(f32vec4)

VertexOutput vs_main(u32 vert_i
                     : SV_VERTEXID)
{
    StructuredBuffer<FaceBuffer> face_buffer = daxa::get_StructuredBuffer<FaceBuffer>(p.face_buffer_id);
    UnpackedVertex vert = get_vertex(face_buffer, vert_i);

    VertexOutput result;
    result.frag_pos = mul(p.view_mat, f32vec4(vert.pos + p.chunk_pos, 1));
    result.nrm = vert.nrm;
    result.uv = vert.uv;
    result.tex_id = vert.tex_id;
    return result;
}

f32vec4 fs_main(VertexOutput vertex_output) : SV_TARGET
{
    UnpackedVertex vert;
    vert.nrm = vertex_output.nrm;
    vert.uv = vertex_output.uv;
    vert.tex_id = vertex_output.tex_id;
    f32 diffuse = dot(normalize(f32vec3(1, -3, 2)), vert.nrm) * 0.5 + 0.5;
    f32vec3 sky_col = lerp(f32vec3(0.1, 0.3, 1.0), f32vec3(1.6, 1.4, 1) * 0.75, diffuse);
    f32vec4 result = 1;

    SamplerState atlas_sampler = daxa::get_sampler(p.sampler_id);
    Texture2DArray atlas_texture_array = daxa::get_Texture2DArray<f32vec4>(p.texture_array_id);

    f32vec4 albedo = atlas_texture_array.Sample(atlas_sampler, f32vec3(vert.uv, vert.tex_id));
    // if (albedo.a < 0.25)
    //     discard;

    f32vec3 col = albedo.rgb;
    col = rgb2hsv(col);
    col.x = frac(col.x + 0.5);
    col.z = 1.0 - col.z;
    col = hsv2rgb(col);

    switch (p.mode)
    {
    case 0: result = f32vec4(col, 1.0f); break;
    case 1: result = f32vec4(col, 0.2f); break;
    case 2: result = f32vec4(col * diffuse * sky_col, 0.1f); break;
    }

    return result;
}
