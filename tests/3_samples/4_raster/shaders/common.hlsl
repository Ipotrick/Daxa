#pragma once

#include "daxa/daxa.hlsl"

static const float2 instance_offsets[6] = {
    float2(+0.0, +0.0),
    float2(+1.0, +0.0),
    float2(+0.0, +1.0),
    float2(+1.0, +0.0),
    float2(+1.0, +1.0),
    float2(+0.0, +1.0),
};

struct Vertex
{
    float3 pos;
    float3 nrm;
    float2 uv;
};

struct VertexOutput
{
    float4 frag_pos : SV_POSITION;
    float3 nrm : NORMAL0;
    // float2 uv : TEXCOORD0;
};

struct FaceBuffer
{
    uint data[32 * 32 * 32 * 6];

    Vertex get_vertex(uint vert_i)
    {
        uint data_index = vert_i / 6;
        uint data_instance = vert_i - data_index * 6;

        uint vert_data = data[data_index];

        Vertex result;

        result.pos.x = (vert_data >> 0x00) & 0x1f;
        result.pos.y = (vert_data >> 0x05) & 0x1f;
        result.pos.z = (vert_data >> 0x0a) & 0x1f;

        float2 uv = instance_offsets[data_instance];
        result.uv = uv;

        uint side = (vert_data >> 0x0f) & 0xf;

        const float scl = 1.0;
        switch (side)
        {
        case 0: result.pos += float3(1.0, uv.x * scl, uv.y * scl), result.nrm = float3(+1.0, +0.0, +0.0); break;
        case 1: result.pos += float3(0.0, 1.0 - uv.x * scl, uv.y * scl), result.nrm = float3(-1.0, +0.0, +0.0); break;
        case 2: result.pos += float3(1.0 - uv.x * scl, 1.0, uv.y * scl), result.nrm = float3(+0.0, +1.0, +0.0); break;
        case 3: result.pos += float3(uv.x * scl, 0.0, uv.y * scl), result.nrm = float3(+0.0, -1.0, +0.0); break;
        case 4: result.pos += float3(uv.x * scl, uv.y * scl, 1.0), result.nrm = float3(+0.0, +0.0, +1.0); break;
        case 5: result.pos += float3(1.0 - uv.x * scl, uv.y * scl, 0.0), result.nrm = float3(+0.0, +0.0, -1.0); break;
        }

        return result;
    }
};

DAXA_DEFINE_GET_STRUCTURED_BUFFER(FaceBuffer);

// struct Input
// {
//     float4x4 view_mat;
// };
// DAXA_DEFINE_GET_STRUCTURED_BUFFER(Input);

// struct Globals
// {
// };
// DAXA_DEFINE_GET_STRUCTURED_BUFFER(Globals);
