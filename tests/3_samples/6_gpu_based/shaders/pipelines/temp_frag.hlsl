
struct VertexOutput
{
    f32vec4 col : LOCATION_0;
};

f32vec4 main(VertexOutput vertex_output) : SV_TARGET0
{
    return f32vec4(vertex_output.col.rgb, 1);
}
