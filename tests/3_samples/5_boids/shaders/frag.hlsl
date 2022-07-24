#include "common.hlsl"

float4 main(VertexOutput vertex_output) : SV_TARGET
{
    return float4(vertex_output.col.rgb, 1);
}
