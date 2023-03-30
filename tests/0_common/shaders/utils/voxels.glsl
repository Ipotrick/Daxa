#pragma once

#include <utils/rand.glsl>

const f32vec2 instance_offsets[6] = {
    f32vec2(+0.0, +0.0),
    f32vec2(+1.0, +0.0),
    f32vec2(+0.0, +1.0),
    f32vec2(+1.0, +0.0),
    f32vec2(+1.0, +1.0),
    f32vec2(+0.0, +1.0),
};
const f32vec3 cross_instance_positions[6] = {
    f32vec3(+0.0 + 0.2, +0.2, +1.0 - 0.2),
    f32vec3(+1.0 - 0.2, +0.2, +0.0 + 0.2),
    f32vec3(+0.0 + 0.2, +1.0, +1.0 - 0.2),
    f32vec3(+1.0 - 0.2, +0.2, +0.0 + 0.2),
    f32vec3(+1.0 - 0.2, +1.0, +0.0 + 0.2),
    f32vec3(+0.0 + 0.2, +1.0, +1.0 - 0.2),
};

void correct_pos(inout UnpackedFace face, u32 face_id)
{
    f32vec3 cross_uv = cross_instance_positions[face.vert_id];
    f32vec2 rnd = f32vec2(
        face.block_pos.x * 3.165 + face.block_pos.y * 5.967 + face.block_pos.z * 7.830,
        face.block_pos.x * 1.715 + face.block_pos.y * 6.683 + face.block_pos.z * 3.096);
    rnd = f32vec2(rand(rnd.x) - 0.5, rand(rnd.y) - 0.5);
    f32vec2 cross_rnd = rnd * 0.4;
    switch (face.block_face)
    {
    case BlockFace_Left: face.pos += f32vec3(1.0, face.uv.x, face.uv.y), face.nrm = f32vec3(+1.0, +0.0, +0.0); break;
    case BlockFace_Right: face.pos += f32vec3(0.0, 1.0 - face.uv.x, face.uv.y), face.nrm = f32vec3(-1.0, +0.0, +0.0); break;
    case BlockFace_Bottom: face.pos += f32vec3(1.0 - face.uv.x, 1.0, face.uv.y), face.nrm = f32vec3(+0.0, +1.0, +0.0); break;
    case BlockFace_Top: face.pos += f32vec3(face.uv.x, 0.0, face.uv.y), face.nrm = f32vec3(+0.0, -1.0, +0.0); break;
    case BlockFace_Back: face.pos += f32vec3(face.uv.x, face.uv.y, 1.0), face.nrm = f32vec3(+0.0, +0.0, +1.0); break;
    case BlockFace_Front: face.pos += f32vec3(1.0 - face.uv.x, face.uv.y, 0.0), face.nrm = f32vec3(+0.0, +0.0, -1.0); break;
    case BlockFace_Cross_A:
    {
        if (face_id % 2 == 0)
        {
            face.pos += cross_uv;
        }
        else
        {
            face.pos += f32vec3(1.0 - cross_uv.x, cross_uv.y, 1.0 - cross_uv.z);
        }
        face.pos.xz += cross_rnd;
        face.nrm = f32vec3(+0.0, -1.0, +0.0);
    }
    break;
    case BlockFace_Cross_B:
    {
        if (face_id % 2 == 0)
        {
            face.pos += f32vec3(cross_uv.x, cross_uv.y, 1.0 - cross_uv.z);
        }
        else
        {
            face.pos += f32vec3(1.0 - cross_uv.x, cross_uv.y, cross_uv.z);
        }
        face.pos.xz += cross_rnd;
        face.nrm = f32vec3(+0.0, -1.0, +0.0);
    }
    break;
    }
}

void correct_uv(inout UnpackedFace face, u32 face_id)
{
    switch (face.block_face)
    {
    case BlockFace_Left: face.uv = f32vec2(1.0, 1.0) + f32vec2(-face.uv.y, -face.uv.x); break;
    case BlockFace_Right: face.uv = f32vec2(0.0, 0.0) + f32vec2(+face.uv.y, +face.uv.x); break;
    case BlockFace_Bottom: face.uv = f32vec2(0.0, 0.0) + f32vec2(+face.uv.x, +face.uv.y); break;
    case BlockFace_Top: face.uv = f32vec2(0.0, 0.0) + f32vec2(+face.uv.x, +face.uv.y); break;
    case BlockFace_Back: face.uv = f32vec2(1.0, 1.0) + f32vec2(-face.uv.x, -face.uv.y); break;
    case BlockFace_Front: face.uv = f32vec2(1.0, 1.0) + f32vec2(-face.uv.x, -face.uv.y); break;
    case BlockFace_Cross_A:
    {
        if (face_id % 2 == 0)
        {
            face.uv = f32vec2(1.0, 1.0) + f32vec2(-face.uv.x, -face.uv.y);
        }
        else
        {
            face.uv = f32vec2(0.0, 1.0) + f32vec2(+face.uv.x, -face.uv.y);
        }
    }
    break;
    case BlockFace_Cross_B:
    {
        if (face_id % 2 == 0)
        {
            face.uv = f32vec2(1.0, 1.0) + f32vec2(-face.uv.x, -face.uv.y);
        }
        else
        {
            face.uv = f32vec2(0.0, 1.0) + f32vec2(+face.uv.x, -face.uv.y);
        }
    }
    break;
    }

    if (face.tex_id == 11 || face.tex_id == 8 || face.tex_id == 8)
    {
        u32 i = u32(rand(face.block_pos) * 8);
        switch (i)
        {
        case 0: face.uv = f32vec2(0 + face.uv.x, 0 + face.uv.y); break;
        case 1: face.uv = f32vec2(1 - face.uv.x, 0 + face.uv.y); break;
        case 2: face.uv = f32vec2(1 - face.uv.x, 1 - face.uv.y); break;
        case 3: face.uv = f32vec2(0 + face.uv.x, 1 - face.uv.y); break;
        case 4: face.uv = f32vec2(0 + face.uv.y, 0 + face.uv.x); break;
        case 5: face.uv = f32vec2(1 - face.uv.y, 0 + face.uv.x); break;
        case 6: face.uv = f32vec2(1 - face.uv.y, 1 - face.uv.x); break;
        case 7: face.uv = f32vec2(0 + face.uv.y, 1 - face.uv.x); break;
        }
    }
}

u32 tile_texture_index(u32 block_id, u32 face)
{
    // clang-format off
    switch (block_id) {
    case BlockID_Debug:           return 0;
    case BlockID_Air:             return 1;
    case BlockID_Bedrock:         return 2;
    case BlockID_Brick:           return 3;
    case BlockID_Cactus:          return 4;
    case BlockID_Cobblestone:     return 5;
    case BlockID_CompressedStone: return 6;
    case BlockID_DiamondOre:      return 7;
    case BlockID_Dirt:            return 8;
    case BlockID_DriedShrub:      return 9;
    case BlockID_Grass:
        switch (face) {
        case BlockFace_Back:
        case BlockFace_Front:
        case BlockFace_Left:
        case BlockFace_Right:     return 10;
        case BlockFace_Bottom:    return 8;
        case BlockFace_Top:       return 11;
        default:                  return 0;
        }
    case BlockID_Gravel:          return 12;
    case BlockID_Lava:            return 13;
    case BlockID_Leaves:          return 21;
    case BlockID_Log:
        switch (face) {
        case BlockFace_Back:
        case BlockFace_Front:
        case BlockFace_Left:
        case BlockFace_Right:     return 22;
        case BlockFace_Bottom:
        case BlockFace_Top:       return 22;
        default:                  return 0;
        }
    case BlockID_MoltenRock:      return 24;
    case BlockID_Planks:          return 25;
    case BlockID_Rose:            return 26;
    case BlockID_Sand:            return 27;
    case BlockID_Sandstone:       return 28;
    case BlockID_Stone:           return 29;
    case BlockID_TallGrass:       return 30;
    case BlockID_Water:           return 31;
    default:                      return 0;
    }
    // clang-format on
}

UnpackedFace get_vertex(daxa_BufferPtr(daxa_u32) packed_faces_ptr, u32 vert_i)
{
    u32 data_index = vert_i / 6;
    u32 data_instance = vert_i - data_index * 6;
    u32 vert_data = deref(packed_faces_ptr[data_index]);
    UnpackedFace result;
    result.block_pos = f32vec3(
        (vert_data >> 0) & 0x1f,
        (vert_data >> 5) & 0x1f,
        (vert_data >> 10) & 0x1f);
    result.pos = result.block_pos;
    result.uv = instance_offsets[data_instance];
    result.block_id = (vert_data >> 18) & 0x3fff;
    result.block_face = (vert_data >> 15) & 0x7;
    result.vert_id = data_instance;
    correct_pos(result, data_index);
    result.tex_id = tile_texture_index(result.block_id, result.block_face);
    correct_uv(result, data_index);
    return result;
}
