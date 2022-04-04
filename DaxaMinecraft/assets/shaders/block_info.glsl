
// clang-format off
const uint BlockID_Air             = 0;
const uint BlockID_Bedrock         = 1;
const uint BlockID_Brick           = 2;
const uint BlockID_Cactus          = 3;
const uint BlockID_Cobblestone     = 4;
const uint BlockID_CompressedStone = 5;
const uint BlockID_DiamondOre      = 6;
const uint BlockID_Dirt            = 7;
const uint BlockID_DriedShrub      = 8;
const uint BlockID_Grass           = 9;
const uint BlockID_Gravel          = 10;
const uint BlockID_Lava            = 11;
const uint BlockID_Leaves          = 12;
const uint BlockID_Log             = 13;
const uint BlockID_MoltenRock      = 14;
const uint BlockID_Planks          = 15;
const uint BlockID_Rose            = 16;
const uint BlockID_Sand            = 17;
const uint BlockID_Sandstone       = 18;
const uint BlockID_Stone           = 19;
const uint BlockID_TallGrass       = 20;
const uint BlockID_Water           = 21;

const uint BiomeID_Plains     = 0;
const uint BiomeID_Forest     = 1;
const uint BiomeID_Desert     = 2;
const uint BiomeID_Beach      = 3;
const uint BiomeID_Caves      = 4;
const uint BiomeID_Underworld = 5;

const uint BlockFace_Back   = 0;
const uint BlockFace_Front  = 1;
const uint BlockFace_Left   = 2;
const uint BlockFace_Right  = 3;
const uint BlockFace_Bottom = 4;
const uint BlockFace_Top    = 5;
// clang-format on

bool is_block_occluding(uint block_id) {
    switch (block_id) {
    case BlockID_Air:
        return false;
    default: return true;
    }
}

bool is_transparent(uint block_id) {
    switch (block_id) {
    case BlockID_Air:
    case BlockID_DriedShrub:
    case BlockID_Lava:
    case BlockID_Leaves:
    case BlockID_Rose:
    case BlockID_TallGrass:
    case BlockID_Water:
        return true;
    default: return false;
    }
}

const uint BLOCK_ID_MASK = 0x0000ffff;
const uint BIOME_ID_MASK = 0x00ff0000;
const uint SDF_DIST_MASK = 0xff000000;

const uvec3 CHUNK_SIZE = uvec3(64);
const uvec3 BLOCK_N = uvec3(512, 256, 512);
const uvec3 CHUNK_N = BLOCK_N / CHUNK_SIZE;

const uint MAX_STEPS = CHUNK_N.x * CHUNK_SIZE.x + CHUNK_N.y * CHUNK_SIZE.y + CHUNK_N.z * CHUNK_SIZE.z;
const uint WATER_LEVEL = 160;

uint get_block_id(uint tile) { return tile & BLOCK_ID_MASK; }
uint get_biome_id(uint tile) { return tile & BIOME_ID_MASK; }
uint get_sdf_dist(uint tile) { return tile & SDF_DIST_MASK; }
