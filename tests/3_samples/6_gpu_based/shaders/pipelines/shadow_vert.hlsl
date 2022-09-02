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

// clang-format off
ShadowPassVertexOutput main(u32 vert_i : SV_VERTEXID)
// clang-format on
{
    StructuredBuffer<FaceBuffer> face_buffer = daxa::get_StructuredBuffer<FaceBuffer>(p.face_buffer_id);
    StructuredBuffer<Input> input = daxa::get_StructuredBuffer<Input>(p.input_buffer_id);
    Vertex vert = face_buffer[0].get_vertex(vert_i, input[0].time);

    ShadowPassVertexOutput result;
    result.frag_pos = mul(input[0].shadow_view_mat[p.data0], f32vec4(vert.pos + p.chunk_pos, 1));
    result.uv = vert.uv;
    result.tex_id = vert.tex_id;
    return result;
}
