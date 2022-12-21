#define DAXA_ENABLE_SHADER_NO_NAMESPACE 1
#include <shared.inl>

[[vk::push_constant]] const DrawPush p;

struct VertexOutput
{
    f32vec4 frag_pos : SV_POSITION;
    f32vec4 col : COLOR0;
};

VertexOutput vs_main(u32 vert_i
                     : SV_VERTEXID)
{
    StructuredBuffer<DrawVertexBuffer> vertex_buffer = daxa::get_StructuredBuffer<DrawVertexBuffer>(p.vertex_buffer_id);
    DrawVertex vert = vertex_buffer[0].verts[vert_i];

    VertexOutput result;
    result.frag_pos = f32vec4(vert.pos.xy, 0, 1);
    result.col = vert.col;
    return result;
}

f32vec4 fs_main(VertexOutput vertex_output) : SV_TARGET
{
    return f32vec4(vertex_output.col.rgb, 1);
}
