#include "common.hlsl"

struct Push
{
    float4x4 view_mat;
    float3 chunk_pos;
    daxa::BufferId face_buffer_id;
    uint mode;
};
[[vk::push_constant]] const Push p;

VertexOutput main(uint vert_i
                  : SV_VERTEXID)
{
    StructuredBuffer<FaceBuffer> face_buffer = daxa::get_StructuredBuffer<FaceBuffer>(p.face_buffer_id);
    Vertex vert = face_buffer[0].get_vertex(vert_i);

    VertexOutput result;
    result.frag_pos = mul(p.view_mat, float4(vert.pos + p.chunk_pos, 1));
    result.nrm = vert.nrm;
    // result.uv = vert.uv;
    return result;
}
