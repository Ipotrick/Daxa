#pragma once

#include DAXA_SHADER_INCLUDE

struct VertexOutput
{
    f32vec4 frag_pos : SV_POSITION;
    f32vec4 col : COLOR0;
};
