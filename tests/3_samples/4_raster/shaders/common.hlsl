#pragma once

#include <shared.inl>

#include "utils/rand.hlsl"
#include "utils/math.hlsl"

static const f32vec2 instance_offsets[6] = {
    f32vec2(+0.0, +0.0),
    f32vec2(+1.0, +0.0),
    f32vec2(+0.0, +1.0),
    f32vec2(+1.0, +0.0),
    f32vec2(+1.0, +1.0),
    f32vec2(+0.0, +1.0),
};
static const f32vec3 cross_instance_positions[6] = {
    f32vec3(+0.0 + 0.2, +0.2, +1.0 - 0.2),
    f32vec3(+1.0 - 0.2, +0.2, +0.0 + 0.2),
    f32vec3(+0.0 + 0.2, +1.0, +1.0 - 0.2),
    f32vec3(+1.0 - 0.2, +0.2, +0.0 + 0.2),
    f32vec3(+1.0 - 0.2, +1.0, +0.0 + 0.2),
    f32vec3(+0.0 + 0.2, +1.0, +1.0 - 0.2),
};

enum class BlockID : u32
{
    Debug,
    Air,
    Bedrock,
    Brick,
    Cactus,
    Cobblestone,
    CompressedStone,
    DiamondOre,
    Dirt,
    DriedShrub,
    Grass,
    Gravel,
    Lava,
    Leaves,
    Log,
    MoltenRock,
    Planks,
    Rose,
    Sand,
    Sandstone,
    Stone,
    TallGrass,
    Water,
};

enum class BlockFace : u32
{
    Left,
    Right,
    Bottom,
    Top,
    Back,
    Front,

    Cross_A,
    Cross_B,
};

struct VertexOutput
{
    f32vec4 frag_pos : SV_POSITION;
    f32vec3 nrm : NORMAL0;
    f32vec2 uv : TEXCOORD0;
    u32 tex_id : DATA0;
};

struct UnpackedVertex
{
    f32vec3 block_pos;
    f32vec3 pos;
    f32vec3 nrm;
    f32vec2 uv;
    BlockID block_id;
    BlockFace block_face;
    u32 tex_id;
    u32 vert_id;

    void correct_pos(u32 face_id)
    {
        f32vec3 cross_uv = cross_instance_positions[vert_id];
        f32vec2 rnd = f32vec2(
            block_pos.x * 3.165 + block_pos.y * 5.967 + block_pos.z * 7.830,
            block_pos.x * 1.715 + block_pos.y * 6.683 + block_pos.z * 3.096);
        rnd = f32vec2(rand(rnd.x) - 0.5, rand(rnd.y) - 0.5);
        f32vec2 cross_rnd = rnd * 0.4;
        switch (block_face)
        {
            // clang-format off
        case BlockFace::Left:   pos += f32vec3(1.0,        uv.x,       uv.y), nrm = f32vec3(+1.0, +0.0, +0.0); break;
        case BlockFace::Right:  pos += f32vec3(0.0,        1.0 - uv.x, uv.y), nrm = f32vec3(-1.0, +0.0, +0.0); break;
        case BlockFace::Bottom: pos += f32vec3(1.0 - uv.x, 1.0,        uv.y), nrm = f32vec3(+0.0, +1.0, +0.0); break;
        case BlockFace::Top:    pos += f32vec3(uv.x,       0.0,        uv.y), nrm = f32vec3(+0.0, -1.0, +0.0); break;
        case BlockFace::Back:   pos += f32vec3(uv.x,       uv.y,        1.0), nrm = f32vec3(+0.0, +0.0, +1.0); break;
        case BlockFace::Front:  pos += f32vec3(1.0 - uv.x, uv.y,        0.0), nrm = f32vec3(+0.0, +0.0, -1.0); break;
            // clang-format on

        case BlockFace::Cross_A:
        {
            if (face_id % 2 == 0)
            {
                pos += cross_uv;
            }
            else
            {
                pos += f32vec3(1.0 - cross_uv.x, cross_uv.y, 1.0 - cross_uv.z);
            }
            pos.xz += cross_rnd;
            nrm = f32vec3(+0.0, -1.0, +0.0);
        }
        break;
        case BlockFace::Cross_B:
        {
            if (face_id % 2 == 0)
            {
                pos += f32vec3(cross_uv.x, cross_uv.y, 1.0 - cross_uv.z);
            }
            else
            {
                pos += f32vec3(1.0 - cross_uv.x, cross_uv.y, cross_uv.z);
            }
            pos.xz += cross_rnd;
            nrm = f32vec3(+0.0, -1.0, +0.0);
        }
        break;
        }
    }

    void correct_uv(u32 face_id)
    {
        switch (block_face)
        {
            // clang-format off
        case BlockFace::Left:    uv = f32vec2(1.0, 1.0) + f32vec2(-uv.y, -uv.x); break;
        case BlockFace::Right:   uv = f32vec2(0.0, 0.0) + f32vec2(+uv.y, +uv.x); break;
        case BlockFace::Bottom:  uv = f32vec2(0.0, 0.0) + f32vec2(+uv.x, +uv.y); break;
        case BlockFace::Top:     uv = f32vec2(0.0, 0.0) + f32vec2(+uv.x, +uv.y); break;
        case BlockFace::Back:    uv = f32vec2(1.0, 1.0) + f32vec2(-uv.x, -uv.y); break;
        case BlockFace::Front:   uv = f32vec2(1.0, 1.0) + f32vec2(-uv.x, -uv.y); break;
            // clang-format on

        case BlockFace::Cross_A:
        {
            if (face_id % 2 == 0)
            {
                uv = f32vec2(1.0, 1.0) + f32vec2(-uv.x, -uv.y);
            }
            else
            {
                uv = f32vec2(0.0, 1.0) + f32vec2(+uv.x, -uv.y);
            }
        }
        break;
        case BlockFace::Cross_B:
        {
            if (face_id % 2 == 0)
            {
                uv = f32vec2(1.0, 1.0) + f32vec2(-uv.x, -uv.y);
            }
            else
            {
                uv = f32vec2(0.0, 1.0) + f32vec2(+uv.x, -uv.y);
            }
        }
        break;
        }

        if (tex_id == 11 || tex_id == 8)
        {
            u32 i = (u32)(rand(block_pos) * 8);
            switch (i)
            {
            case 0: uv = f32vec2(0 + uv.x, 0 + uv.y); break;
            case 1: uv = f32vec2(1 - uv.x, 0 + uv.y); break;
            case 2: uv = f32vec2(1 - uv.x, 1 - uv.y); break;
            case 3: uv = f32vec2(0 + uv.x, 1 - uv.y); break;

            case 4: uv = f32vec2(0 + uv.y, 0 + uv.x); break;
            case 5: uv = f32vec2(1 - uv.y, 0 + uv.x); break;
            case 6: uv = f32vec2(1 - uv.y, 1 - uv.x); break;
            case 7: uv = f32vec2(0 + uv.y, 1 - uv.x); break;
            }
        }
    }
};

u32 tile_texture_index(BlockID block_id, BlockFace face)
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
    case BlockID::Lava:            return 13; // + i32(globals[0].time * 6) % 8;
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

UnpackedVertex get_vertex(StructuredBuffer<FaceBuffer> face_buffer, u32 vert_i)
{
    u32 data_index = vert_i / 6;
    u32 data_instance = vert_i - data_index * 6;

    u32 vert_data = face_buffer[0].data[data_index];

    UnpackedVertex result;

    result.block_pos = f32vec3(
        (vert_data >> 0) & 0x1f,
        (vert_data >> 5) & 0x1f,
        (vert_data >> 10) & 0x1f);
    result.pos = result.block_pos;
    result.uv = instance_offsets[data_instance];
    result.block_id = (BlockID)((vert_data >> 18) & 0x3fff);
    result.block_face = (BlockFace)((vert_data >> 15) & 0x7);

    result.vert_id = data_instance;
    result.correct_pos(data_index);
    result.tex_id = tile_texture_index(result.block_id, result.block_face);
    result.correct_uv(data_index);

    return result;
}
