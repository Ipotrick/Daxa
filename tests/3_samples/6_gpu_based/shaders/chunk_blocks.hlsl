#pragma once

#include "worldgen.hlsl"
#include "common.hlsl"

enum class BlockID : uint
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

enum class BlockFace : uint
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

struct ChunkBlocks
{
    float3 chunk_pos;
    BlockID voxels[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];

    BlockID generate_block_id(in WorldgenState worldgen_state)
    {
        float val = worldgen_state.t_noise;
        val = max(val, 0.0);
        if (val > 0.0)
            return BlockID::Stone;
        if (worldgen_state.pos.y > 33.0)
            return BlockID::Water;
        return BlockID::Air;
    }

    void chunkgen(uint3 block_offset)
    {
        float3 block_pos = float3(block_offset) + chunk_pos;
        WorldgenState worldgen_state = get_worldgen_state(block_pos);
        BlockID id = generate_block_id(worldgen_state);

        if (id == BlockID::Stone)
        {
            uint above_i;
            for (above_i = 0; above_i < 6; ++above_i)
            {
                WorldgenState temp_worldgen_state = get_worldgen_state(block_pos - float3(0, above_i + 1, 0));
                if (generate_block_id(temp_worldgen_state) == BlockID::Air)
                    break;
            }
            uint r = (uint)(worldgen_state.r * 10);
            switch (r)
            {
            case 0: id = BlockID::Gravel; break;
            case 1: id = BlockID::Cobblestone; break;
            default: break;
            }
            if (above_i == 0)
                id = BlockID::Grass;
            else if (above_i < 4)
                id = BlockID::Dirt;
        }
        else if (id == BlockID::Air)
        {
            uint below_i;
            for (below_i = 0; below_i < 6; ++below_i)
            {
                WorldgenState temp_worldgen_state = get_worldgen_state(block_pos + float3(0, below_i + 1, 0));
                if (generate_block_id(temp_worldgen_state) == BlockID::Stone)
                    break;
            }
            if (below_i == 0)
            {
                uint r = (uint)(worldgen_state.r * 100);
                if (r < 50)
                {
                    switch (r)
                    {
                    case 0: id = BlockID::Rose; break;
                    default:
                        id = BlockID::TallGrass;
                        break;
                    }
                }
            }
        }

        uint voxel_index = block_offset.x + block_offset.y * CHUNK_SIZE + block_offset.z * CHUNK_SIZE * CHUNK_SIZE;
        voxels[voxel_index] = id;
    }
};

struct VoxelWorld
{
    ChunkBlocks chunks_blocks[CHUNK_COUNT_X * CHUNK_COUNT_Y * CHUNK_COUNT_Z];
};
DAXA_DEFINE_GET_STRUCTURED_BUFFER(VoxelWorld);

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
