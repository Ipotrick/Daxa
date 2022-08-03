#pragma once

#include "common.hlsl"
#include "meshlet.hlsl"

#include "utils/rand.hlsl"

struct VertexOutput
{
    float4 frag_pos : SV_POSITION;
    float3 nrm : NORMAL0;
    float2 uv : TEXCOORD0;
    float3 pos : DATA1;
    uint tex_id : DATA0;
};
struct ShadowPassVertexOutput
{
    float4 frag_pos : SV_POSITION;
    float2 uv : TEXCOORD0;
    uint tex_id : DATA0;
};


// DAXA_DEFINE_GET_STRUCTURED_BUFFER(FaceBuffer);

// struct Globals
// {
// };
// DAXA_DEFINE_GET_STRUCTURED_BUFFER(Globals);

#include "../config.inl"
