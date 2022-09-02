#include "common.hlsl"

struct Push
{
    f32mat4x4 view_mat;
    f32vec3 chunk_pos;
    BufferId face_buffer_id;
    ImageViewId texture_array_id;
    SamplerId sampler_id;
    u32 mode;
};
[[vk::push_constant]] const Push p;

VertexOutput main(u32 vert_i
                  : SV_VERTEXID)
{
    StructuredBuffer<FaceBuffer> face_buffer = daxa::get_StructuredBuffer<FaceBuffer>(p.face_buffer_id);
    Vertex vert = face_buffer[0].get_vertex(vert_i);

    VertexOutput result;
    result.frag_pos = mul(p.view_mat, f32vec4(vert.pos + p.chunk_pos, 1));
    result.nrm = vert.nrm;
    result.uv = vert.uv;
    result.tex_id = vert.tex_id;
    return result;
}
