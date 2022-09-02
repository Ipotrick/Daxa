#pragma once

#include "common.hlsl"
#include "meshlet.hlsl"

#include "utils/rand.hlsl"

struct VertexOutput
{
    f32vec4 frag_pos : SV_POSITION;
    f32vec3 nrm : NORMAL0;
    f32vec2 uv : TEXCOORD0;
    f32vec3 pos : DATA1;
    u32 tex_id : DATA0;
};
struct ShadowPassVertexOutput
{
    f32vec4 frag_pos : SV_POSITION;
    f32vec2 uv : TEXCOORD0;
    u32 tex_id : DATA0;
};


// DAXA_DEFINE_GET_STRUCTURED_BUFFER(FaceBuffer);

// struct Globals
// {
// };
// DAXA_DEFINE_GET_STRUCTURED_BUFFER(Globals);

#include "../config.inl"
