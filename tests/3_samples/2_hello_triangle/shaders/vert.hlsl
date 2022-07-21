#include "daxa/daxa.hlsl"

struct Vertex {
    float4 pos;
    float4 col;
};
struct VertexBuffer
{
    Vertex verts[3];
};
DAXA_DEFINE_GET_STRUCTURED_BUFFER(VertexBuffer);

struct Push
{
    daxa::BufferId vertex_buffer_id;
    uint2 frame_dim;
};
[[vk::push_constant]] const Push p;

struct VertexOutput
{
    float4 frag_pos : SV_POSITION;
    float4 col : COLOR0;
};

VertexOutput main(uint vert_i : SV_VERTEXID)
{
    StructuredBuffer<VertexBuffer> vertex_buffer = daxa::get_StructuredBuffer<VertexBuffer>(p.vertex_buffer_id);
    Vertex vert = vertex_buffer[0].verts[vert_i];

    VertexOutput result;
    result.frag_pos = float4(vert.pos.xy, 0, 1);
    result.col = vert.col;
    return result;
}
