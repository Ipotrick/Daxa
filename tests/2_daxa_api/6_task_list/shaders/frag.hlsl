
struct VertexOutput
{
    f32vec4 frag_pos : SV_POSITION;
    f32vec4 col : COLOR0;
};

f32vec4 main(VertexOutput vertex_output) : SV_TARGET
{
    return f32vec4(vertex_output.col.rgb, 1);
}
