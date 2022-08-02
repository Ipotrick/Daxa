
struct VertexOutput
{
    float4 col : LOCATION_0;
};

float4 main(VertexOutput vertex_output) : SV_TARGET0
{
    return float4(vertex_output.col.rgb, 1);
}
