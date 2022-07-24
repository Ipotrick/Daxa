#include "common.hlsl"

struct Push
{
    float4x4 view_mat;
    float3 chunk_pos;
    daxa::BufferId face_buffer_id;
    uint mode;
};
[[vk::push_constant]] const Push p;

float4 main(VertexOutput vertex_output) : SV_TARGET
{
    Vertex vert;
    vert.nrm = vertex_output.nrm;
    float diffuse = dot(normalize(float3(1, -3, 2)), vert.nrm) * 0.5 + 0.5;
    float3 sky_col = lerp(float3(0.1, 0.3, 1.0), float3(1.6, 1.4, 1) * 0.75, diffuse);
    float4 result = 1;

    switch (p.mode)
    {
    case 0: result = float4(float3(1.0, 1.0, 1.0) * diffuse * sky_col, 1.0f); break;
    case 1: result = float4(float3(0.2, 0.4, 1.0) * diffuse * sky_col, 0.6f); break;
    }
    return result;
}
