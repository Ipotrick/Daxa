#include <daxa/daxa.inl>

static const f32vec2 positions[3] = {
    f32vec2(-0.5, +0.5),
    f32vec2(+0.0, -0.5),
    f32vec2(+0.5, +0.5),
};
static const f32vec3 colors[3] = {
    f32vec3(1, 0, 0),
    f32vec3(0, 1, 0),
    f32vec3(0, 0, 1),
};

struct VertexOutput
{
    f32vec4 frag_pos : SV_POSITION;
    f32vec4 col : COLOR0;
};

VertexOutput main(u32 vert_i
                  : SV_VERTEXID)
{
    VertexOutput result;
    result.frag_pos = f32vec4(positions[vert_i], 0, 1);
    result.col = f32vec4(colors[vert_i], 1);
    return result;
}
