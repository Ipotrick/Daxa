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

// clang-format off
VertexOutput main(uint vert_i : SV_VERTEXID)
// clang-format on
{
    StructuredBuffer<FaceBuffer> face_buffer = daxa::get_StructuredBuffer<FaceBuffer>(p.face_buffer_id);
    StructuredBuffer<Input> input = daxa::get_StructuredBuffer<Input>(p.input_buffer_id);
    Vertex vert = face_buffer[0].get_vertex(vert_i, input[0].time);

    VertexOutput result;
    result.frag_pos = mul(input[0].view_mat, float4(vert.pos + p.chunk_pos, 1));
    result.nrm = vert.nrm;
    result.uv = vert.uv;
    result.pos = vert.pos + p.chunk_pos;
    result.tex_id = vert.tex_id;
    return result;
}
