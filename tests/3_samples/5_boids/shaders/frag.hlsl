#include "common.hlsl"

f32vec4 main(VertexOutput vertex_output) : SV_TARGET
{
    return f32vec4(vertex_output.col.rgb, 1);
}
