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

void main(ShadowPassVertexOutput vertex_output)
{
    Vertex vert;
    vert.uv = vertex_output.uv;
    vert.tex_id = vertex_output.tex_id;
    Texture2DArray atlas_texture_array = daxa::get_Texture2DArray<f32vec4>(p.texture_array_id);
    f32vec4 albedo = atlas_texture_array.Load(i32vec4(vert.uv.x * 16, vert.uv.y * 16, vert.tex_id, 0));
    if (albedo.a < 0.25 || vert.tex_id == 31)
        discard;
}
