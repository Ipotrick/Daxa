#pragma once

#include "common.hlsl"

#include "utils/rand.hlsl"

static const float2 instance_offsets[6] = {
    float2(+0.0, +0.0),
    float2(+1.0, +0.0),
    float2(+0.0, +1.0),
    float2(+1.0, +0.0),
    float2(+1.0, +1.0),
    float2(+0.0, +1.0),
};
static const float3 cross_instance_positions[6] = {
    float3(+0.0 + 0.2, +0.2, +1.0 - 0.2),
    float3(+1.0 - 0.2, +0.2, +0.0 + 0.2),
    float3(+0.0 + 0.2, +1.0, +1.0 - 0.2),
    float3(+1.0 - 0.2, +0.2, +0.0 + 0.2),
    float3(+1.0 - 0.2, +1.0, +0.0 + 0.2),
    float3(+0.0 + 0.2, +1.0, +1.0 - 0.2),
};

struct Vertex
{
    float3 block_pos;
    float3 pos;
    float3 nrm;
    float2 uv;
    BlockID block_id;
    BlockFace block_face;
    uint tex_id;
    uint vert_id;

    void correct_pos(uint face_id)
    {
        float3 cross_uv = cross_instance_positions[vert_id];
        switch (block_face)
        {
            // clang-format off
        case BlockFace::Left:   pos += float3(1.0,        uv.x,       uv.y), nrm = float3(+1.0, +0.0, +0.0); break;
        case BlockFace::Right:  pos += float3(0.0,        1.0 - uv.x, uv.y), nrm = float3(-1.0, +0.0, +0.0); break;
        case BlockFace::Bottom: pos += float3(1.0 - uv.x, 1.0,        uv.y), nrm = float3(+0.0, +1.0, +0.0); break;
        case BlockFace::Top:    pos += float3(uv.x,       0.0,        uv.y), nrm = float3(+0.0, -1.0, +0.0); break;
        case BlockFace::Back:   pos += float3(uv.x,       uv.y,        1.0), nrm = float3(+0.0, +0.0, +1.0); break;
        case BlockFace::Front:  pos += float3(1.0 - uv.x, uv.y,        0.0), nrm = float3(+0.0, +0.0, -1.0); break;
            // clang-format on

        case BlockFace::Cross_A:
        {
            if (face_id % 2 == 0)
            {
                pos += cross_uv;
                nrm = float3(0.707, 0, 0.707);
            }
            else
            {
                pos += float3(1.0 - cross_uv.x, cross_uv.y, 1.0 - cross_uv.z);
                nrm = float3(-0.707, 0, -0.707);
            }
            pos.x += rand(block_pos.x + 2.0 * block_pos.z - block_pos.y) * 0.5 - 0.25;
            pos.z += rand(block_pos.z + 2.0 * block_pos.x + block_pos.y) * 0.5 - 0.25;
        }
        break;
        case BlockFace::Cross_B:
        {
            if (face_id % 2 == 0)
            {
                pos += float3(cross_uv.x, cross_uv.y, 1.0 - cross_uv.z);
                nrm = float3(-0.707, 0, 0.707);
            }
            else
            {
                pos += float3(1.0 - cross_uv.x, cross_uv.y, cross_uv.z);
                nrm = float3(0.707,  0, -0.707);
            }
            pos.x += rand(block_pos.x + 2.0 * block_pos.z - block_pos.y) * 0.5 - 0.25;
            pos.z += rand(block_pos.z + 2.0 * block_pos.x + block_pos.y) * 0.5 - 0.25;
        }
        break;
        }
    }

    void correct_uv()
    {
        switch (block_face)
        {
            // clang-format off
        case BlockFace::Left:    uv = float2(1.0, 1.0) + float2(-uv.y, -uv.x); break;
        case BlockFace::Right:   uv = float2(0.0, 0.0) + float2(+uv.y, +uv.x); break;
        case BlockFace::Bottom:  uv = float2(0.0, 0.0) + float2(+uv.x, +uv.y); break;
        case BlockFace::Top:     uv = float2(0.0, 0.0) + float2(+uv.x, +uv.y); break;
        case BlockFace::Back:    uv = float2(1.0, 1.0) + float2(-uv.x, -uv.y); break;
        case BlockFace::Front:   uv = float2(1.0, 1.0) + float2(-uv.x, -uv.y); break;
            // clang-format on

        case BlockFace::Cross_A: uv = float2(1.0, 1.0) + float2(-uv.x, -uv.y); break;
        case BlockFace::Cross_B: uv = float2(1.0, 1.0) + float2(-uv.x, -uv.y); break;
        }

        if (tex_id == 11 || tex_id == 8)
        {
            uint i = (uint)(rand(block_pos) * 8);
            switch (i)
            {
            case 0: uv = float2(0 + uv.x, 0 + uv.y); break;
            case 1: uv = float2(1 - uv.x, 0 + uv.y); break;
            case 2: uv = float2(1 - uv.x, 1 - uv.y); break;
            case 3: uv = float2(0 + uv.x, 1 - uv.y); break;

            case 4: uv = float2(0 + uv.y, 0 + uv.x); break;
            case 5: uv = float2(1 - uv.y, 0 + uv.x); break;
            case 6: uv = float2(1 - uv.y, 1 - uv.x); break;
            case 7: uv = float2(0 + uv.y, 1 - uv.x); break;
            }
        }
    }
};

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

uint tile_texture_index(BlockID block_id, BlockFace face, float time)
{
    // clang-format off
    switch (block_id) {
    case BlockID::Debug:           return 0;
    case BlockID::Air:             return 1;
    case BlockID::Bedrock:         return 2;
    case BlockID::Brick:           return 3;
    case BlockID::Cactus:          return 4;
    case BlockID::Cobblestone:     return 5;
    case BlockID::CompressedStone: return 6;
    case BlockID::DiamondOre:      return 7;
    case BlockID::Dirt:            return 8;
    case BlockID::DriedShrub:      return 9;
    case BlockID::Grass:
        switch (face) {
        case BlockFace::Back:
        case BlockFace::Front:
        case BlockFace::Left:
        case BlockFace::Right:     return 10;
        case BlockFace::Bottom:    return 8;
        case BlockFace::Top:       return 11;
        default:                   return 0;
        }
    case BlockID::Gravel:          return 12;
    case BlockID::Lava:            return 13 + int(time * 6) % 8;
    case BlockID::Leaves:          return 21;
    case BlockID::Log:
        switch (face) {
        case BlockFace::Back:
        case BlockFace::Front:
        case BlockFace::Left:
        case BlockFace::Right:     return 22;
        case BlockFace::Bottom:
        case BlockFace::Top:       return 22;
        default:                   return 0;
        }
    case BlockID::MoltenRock:      return 24;
    case BlockID::Planks:          return 25;
    case BlockID::Rose:            return 26;
    case BlockID::Sand:            return 27;
    case BlockID::Sandstone:       return 28;
    case BlockID::Stone:           return 29;
    case BlockID::TallGrass:       return 30;
    case BlockID::Water:           return 31;
    default:                       return 0;
    }
    // clang-format on
}

struct FaceBuffer
{
    uint data[32 * 32 * 32 * 6];

    Vertex get_vertex(uint vert_i, float time)
    {
        uint data_index = vert_i / 6;
        uint data_instance = vert_i - data_index * 6;

        uint vert_data = data[data_index];

        Vertex result;

        result.block_pos = float3(
            (vert_data >> 0) & 0x3f,
            (vert_data >> 6) & 0x3f,
            (vert_data >> 12) & 0x3f);
        result.pos = result.block_pos;
        result.uv = instance_offsets[data_instance];
        result.block_face = (BlockFace)((vert_data >> 18) & 0x7);
        result.block_id = (BlockID)((vert_data >> 21) & 0x07ff);

        result.vert_id = data_instance;
        result.correct_pos(data_index);
        result.tex_id = tile_texture_index(result.block_id, result.block_face, time);
        result.correct_uv();

        return result;
    }
};

DAXA_DEFINE_GET_STRUCTURED_BUFFER(FaceBuffer);

// struct Globals
// {
// };
// DAXA_DEFINE_GET_STRUCTURED_BUFFER(Globals);

#include "../config.inl"
