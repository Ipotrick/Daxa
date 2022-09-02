#include DAXA_SHADER_INCLUDE

#include "boids.hlsl"
#include "common.hlsl"

struct Push
{
    f32mat4x4 view_mat;
    BufferId boids_buffer_id;
};
[[vk::push_constant]] const Push p;

VertexOutput main(u32 vert_i : SV_VERTEXID)
{
    StructuredBuffer<BoidsBuffer> boids_buffer = daxa::get_StructuredBuffer<BoidsBuffer>(p.boids_buffer_id);
    BoidVertex vert = boids_buffer[0].get_vertex(vert_i);

    VertexOutput result;
    result.frag_pos = mul(p.view_mat, f32vec4(vert.pos.xy, 0, 1));
    result.col = f32vec4(vert.col, 1);
    return result;
}
