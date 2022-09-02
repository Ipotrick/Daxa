#include "../common.hlsl"

struct VertexOutput
{
    f32vec4 frag_pos : SV_POSITION;
    f32vec4 col : LOCATION_0;
};

static const VertexOutput vertices[3] = {
    {f32vec4(-0.5, +0.5, +0.0, +1.0), f32vec4(1, 0, 0, 1)},
    {f32vec4(+0.0, -0.5, +0.0, +1.0), f32vec4(0, 1, 0, 1)},
    {f32vec4(+0.5, +0.5, +0.0, +1.0), f32vec4(0, 0, 1, 1)},
};

[[vk::push_constant]] DrawRasterPush p;

VertexOutput main(u32 vert_i : SV_VERTEXID, [[vk::builtin("DrawIndex")]] u32 draw_index : DI)
{
    StructuredBuffer<SharedGlobals> globals_buffer = daxa::get_StructuredBuffer<SharedGlobals>(p.globals_buffer_id);
    StructuredBuffer<ChunkDrawInfo> draw_info_buffer = daxa::get_StructuredBuffer<ChunkDrawInfo>(globals_buffer[0].chunk_draw_infos_buffer_id);

    u32vec3 chunk_index = draw_info_buffer[draw_index].chunk_index;
    f32vec3 chunk_position_offset = f32vec3(chunk_index) * f32vec3(1,1,1);
    VertexOutput result = vertices[vert_i];
    result.frag_pos += f32vec4(chunk_position_offset.xyz, 0);
    return result;
}
