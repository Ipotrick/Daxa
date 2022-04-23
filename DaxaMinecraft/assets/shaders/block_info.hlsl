#pragma once

enum class BlockID : uint {
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

enum class BiomeID : uint {
    Plains,
    Forest,
    Desert,
    Beach,
    Caves,
    Underworld,
};

enum class BlockFace : uint {
    Back,
    Front,
    Left,
    Right,
    Bottom,
    Top,
};

bool is_block_occluding(BlockID block_id) {
    switch (block_id) {
    case BlockID::Air:
        return false;
    default:
        return true;
    }
}

bool is_transparent(BlockID block_id) {
    switch (block_id) {
    case BlockID::Air:
    case BlockID::Water:
        return true;
    default:
        return false;
    }
}

BlockID inventory_palette(uint index) {
    switch (index) {
    case 0: return BlockID::Brick;
    case 1: return BlockID::Cobblestone;
    case 2: return BlockID::Grass;
    case 3: return BlockID::Dirt;
    case 4: return BlockID::Sand;
    case 5: return BlockID::Planks;
    case 6: return BlockID::Log;
    case 7: return BlockID::Leaves;
    }

    return BlockID::Debug;
}

#include "../../src/game/defines.inl"
#define WATER_LEVEL 160
