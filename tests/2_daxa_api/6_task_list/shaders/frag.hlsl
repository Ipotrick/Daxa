
struct VertexOutput
{
    float4 frag_pos : SV_POSITION;
    float4 col : COLOR0;
};

float4 main(VertexOutput vertex_output) : SV_TARGET
{
    return float4(vertex_output.col.rgb, 1);
}
