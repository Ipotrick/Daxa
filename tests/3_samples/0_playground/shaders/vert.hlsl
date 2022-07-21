#include "daxa/daxa.hlsl"

static const float2 instance_offsets[6] = {
    float2(+0.0, +0.0),
    float2(+1.0, +0.0),
    float2(+0.0, +1.0),
    float2(+1.0, +0.0),
    float2(+1.0, +1.0),
    float2(+0.0, +1.0),
};

struct Vertex
{
    float3 pos;
};
struct VertexBuffer
{
    uint data[3];

    Vertex get(uint vert_i)
    {
        uint data_index = vert_i / 6;
        uint data_instance = vert_i - data_index * 6;

        uint vert_data = data[data_index];

        Vertex result;

        result.pos.x = (vert_data >> 0x00) & 0x1f;
        result.pos.y = (vert_data >> 0x05) & 0x1f;
        result.pos.z = (vert_data >> 0x0a) & 0x1f;

        result.pos += float3(instance_offsets[data_instance] * 0.9, 0);

        result.pos *= 1.0 / 32.0;

        return result;
    }
};
DAXA_DEFINE_GET_STRUCTURED_BUFFER(VertexBuffer);

struct Push
{
    float4x4 view_mat;
    daxa::BufferId vertex_buffer_id;
    uint2 frame_dim;
};
[[vk::push_constant]] const Push p;

struct VertexOutput
{
    float4 frag_pos : SV_POSITION;
    float4 col : COLOR0;
};

VertexOutput main(uint vert_i
                  : SV_VERTEXID)
{
    StructuredBuffer<VertexBuffer> vertex_buffer = daxa::get_StructuredBuffer<VertexBuffer>(p.vertex_buffer_id);

    Vertex vert = vertex_buffer[0].get(vert_i);

    VertexOutput result;
    result.frag_pos = float4(vert.pos, 1);
    result.col = float4(0, 0, 1, 1);
    return result;
}
