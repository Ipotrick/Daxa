#include "../common.hlsl"

struct VertexOutput
{
    float4 frag_pos : SV_POSITION;
    float4 col : LOCATION_0;
};

static const VertexOutput vertices[3] = {
    {float4(-0.5, +0.5, +0.0, +1.0), float4(1, 0, 0, 1)},
    {float4(+0.0, -0.5, +0.0, +1.0), float4(0, 1, 0, 1)},
    {float4(+0.5, +0.5, +0.0, +1.0), float4(0, 0, 1, 1)},
};

[[vk::push_constant]] DrawRasterPush p;

VertexOutput main(uint vert_i : SV_VERTEXID, [[vk::builtin("DrawIndex")]] uint draw_index : DI)
{
    StructuredBuffer<SharedGlobals> globals_buffer = daxa::get_StructuredBuffer<SharedGlobals>(p.globals_buffer_id);
    StructuredBuffer<ChunkDrawInfo> draw_info_buffer = daxa::get_StructuredBuffer<ChunkDrawInfo>(globals_buffer[0].chunk_draw_infos_buffer_id);

    uint3 chunk_index = draw_info_buffer[draw_index].chunk_index;
    float3 chunk_position_offset = float3(chunk_index) * float3(1,1,1);
    VertexOutput result = vertices[vert_i];
    result.frag_pos += float4(chunk_position_offset.xyz, 0);
    return result;
}
