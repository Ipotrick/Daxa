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

#define CHUNK_SIZE 64
#define BLOCK_NX 1024
#define BLOCK_NY 256
#define BLOCK_NZ 1024
#define CHUNK_NX (BLOCK_NX / CHUNK_SIZE)
#define CHUNK_NY (BLOCK_NY / CHUNK_SIZE)
#define CHUNK_NZ (BLOCK_NZ / CHUNK_SIZE)
#define WATER_LEVEL 160
