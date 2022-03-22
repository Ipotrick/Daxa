
// clang-format off
const uint Air         = 0;
const uint Brick       = 1;
const uint Cactus      = 2;
const uint Cobblestone = 3;
const uint DiamondOre  = 4;
const uint Dirt        = 5;
const uint DriedShrub  = 6;
const uint Grass       = 7;
const uint Gravel      = 8;
const uint Leaves      = 9;
const uint Log         = 10;
const uint Planks      = 11;
const uint Rose        = 12;
const uint Sand        = 13;
const uint Sandstone   = 14;
const uint Stone       = 15;
const uint TallGrass   = 16;
const uint Water       = 17;

const uint Plains = 0;
const uint Forest = 1;
const uint Desert = 2;
const uint Beach  = 3;
// clang-format on

const uvec3 CHUNK_SIZE = uvec3(16);
const uint RENDER_DIST_XZ = 4;
const uvec3 CHUNK_MAX = uvec3(16, 6, 16);

const uint BLOCK_ID_MASK = 0x0000ffff;
const uint BIOME_ID_MASK = 0xffff0000;

struct ChunkgenBuffer {
    uint blocks[CHUNK_SIZE.x * CHUNK_SIZE.y * CHUNK_SIZE.z];
};

bool is_transparent(uint id) {
    switch (id & BLOCK_ID_MASK) {
    case Air:
    case DriedShrub:
    case Leaves:
    case Rose:
    case TallGrass:
        return true;
    default: return false;
    }
}
