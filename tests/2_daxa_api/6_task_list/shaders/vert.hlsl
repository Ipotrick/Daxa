#include "daxa/daxa.hlsl"

static const float2 positions[3] = {
    float2(-0.5, +0.5),
    float2(+0.0, -0.5),
    float2(+0.5, +0.5),
};
static const float3 colors[3] = {
    float3(1, 0, 0),
    float3(0, 1, 0),
    float3(0, 0, 1),
};

struct VertexOutput
{
    float4 frag_pos : SV_POSITION;
    float4 col : COLOR0;
};

VertexOutput main(uint vert_i : SV_VERTEXID)
{
    VertexOutput result;
    result.frag_pos = float4(positions[vert_i], 0, 1);
    result.col = float4(colors[vert_i], 1);
    return result;
}
