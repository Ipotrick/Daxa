#if !defined(BLOCK_INFO_HLSL)
#define BLOCK_INFO_HLSL

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
    case BlockID::DriedShrub:
    case BlockID::Lava:
    case BlockID::Leaves:
    case BlockID::Rose:
    case BlockID::TallGrass:
    case BlockID::Water:
        return true;
    default:
        return false;
    }
}

enum INTEGER_CONSTANTS {
    CHUNK_SIZE = 64,
    BLOCK_NX = 1024,
    BLOCK_NY = 256,
    BLOCK_NZ = 1024,
    CHUNK_NX = BLOCK_NX / CHUNK_SIZE,
    CHUNK_NY = BLOCK_NY / CHUNK_SIZE,
    CHUNK_NZ = BLOCK_NZ / CHUNK_SIZE,
    MAX_STEPS =
        CHUNK_NX * CHUNK_SIZE + CHUNK_NY * CHUNK_SIZE + CHUNK_NZ * CHUNK_SIZE,
    WATER_LEVEL = 160,
};

#endif