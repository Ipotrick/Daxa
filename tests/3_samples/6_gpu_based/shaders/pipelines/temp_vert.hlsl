
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

VertexOutput main(uint vert_i : SV_VERTEXID, [[vk::builtin("DrawIndex")]] uint draw_index : DI)
{
    return vertices[vert_i];
}
