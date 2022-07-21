#include "common.hlsl"

float4 main(VertexOutput vertex_output) : SV_TARGET
{
    Vertex vert;
    vert.nrm = vertex_output.nrm;
    float diffuse = dot(normalize(float3(1, -3, 2)), vert.nrm) * 0.5 + 0.5;
    float3 col = float3(1, 1, 1);
    float3 sky_col = lerp(float3(0.1, 0.3, 1.0), float3(1.6, 1.4, 1) * 0.75, diffuse);
    return float4(col * diffuse * sky_col, 1);
}
