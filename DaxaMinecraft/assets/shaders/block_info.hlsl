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

#include "../../src/game/defines.inl"
#define WATER_LEVEL 160
