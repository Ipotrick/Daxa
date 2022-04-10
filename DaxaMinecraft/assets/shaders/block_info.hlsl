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

#define CHUNK_SIZE 64
#define BLOCK_NX 1024
#define BLOCK_NY 256
#define BLOCK_NZ 1024
#define CHUNK_NX 16
#define CHUNK_NY 4
#define CHUNK_NZ 16
#define MAX_STEPS 10000
#define WATER_LEVEL 160

#endif