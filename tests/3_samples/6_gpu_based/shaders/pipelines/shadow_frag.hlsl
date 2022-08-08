#include "raster_common.hlsl"

struct Push
{
    float3 chunk_pos;
    daxa::BufferId input_buffer_id;
    daxa::BufferId face_buffer_id;
    daxa::ImageViewId texture_array_id;
    daxa::SamplerId sampler_id;
    uint mode;
    uint data0;
};
[[vk::push_constant]] const Push p;

DAXA_DEFINE_GET_TEXTURE2DARRAY(float4)

void main(ShadowPassVertexOutput vertex_output)
{
    Vertex vert;
    vert.uv = vertex_output.uv;
    vert.tex_id = vertex_output.tex_id;
    Texture2DArray atlas_texture_array = daxa::get_Texture2DArray<float4>(p.texture_array_id);
    float4 albedo = atlas_texture_array.Load(int4(vert.uv.x * 16, vert.uv.y * 16, vert.tex_id, 0));
    if (albedo.a < 0.25 || vert.tex_id == 31)
        discard;
}
