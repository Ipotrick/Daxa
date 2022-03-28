
// clang-format off
const uint BlockID_Air         = 0;
const uint BlockID_Brick       = 1;
const uint BlockID_Cactus      = 2;
const uint BlockID_Cobblestone = 3;
const uint BlockID_DiamondOre  = 4;
const uint BlockID_Dirt        = 5;
const uint BlockID_DriedShrub  = 6;
const uint BlockID_Grass       = 7;
const uint BlockID_Gravel      = 8;
const uint BlockID_Leaves      = 9;
const uint BlockID_Log         = 10;
const uint BlockID_Planks      = 11;
const uint BlockID_Rose        = 12;
const uint BlockID_Sand        = 13;
const uint BlockID_Sandstone   = 14;
const uint BlockID_Stone       = 15;
const uint BlockID_TallGrass   = 16;
const uint BlockID_Water       = 17;

const uint BiomeID_Plains = 0;
const uint BiomeID_Forest = 1;
const uint BiomeID_Desert = 2;
const uint BiomeID_Beach  = 3;

const uint BlockFace_Back   = 0;
const uint BlockFace_Front  = 1;
const uint BlockFace_Left   = 2;
const uint BlockFace_Right  = 3;
const uint BlockFace_Bottom = 4;
const uint BlockFace_Top    = 5;
// clang-format on

const uint BLOCK_ID_MASK = 0x0000ffff;
const uint BIOME_ID_MASK = 0xffff0000;

const uint WATER_LEVEL = 64;

bool is_transparent(uint block_id) {
    switch (block_id & BLOCK_ID_MASK) {
    case BlockID_Air:
    case BlockID_DriedShrub:
    case BlockID_Leaves:
    case BlockID_Rose:
    case BlockID_TallGrass:
        return true;
    default: return false;
    }
}

uint tile_texture_index(uint block_id, uint face) {
    switch (block_id) {
    case BlockID_Brick: return 0;
    case BlockID_Cactus: return 1;
    case BlockID_Cobblestone: return 2;
    case BlockID_DiamondOre: return 3;
    case BlockID_Dirt: return 4;
    case BlockID_DriedShrub: return 5;
    case BlockID_Grass:
        switch (face) {
        case BlockFace_Back:
        case BlockFace_Front:
        case BlockFace_Left:
        case BlockFace_Right: return 6;
        case BlockFace_Bottom: return 4;
        case BlockFace_Top: return 7;
        default: return 0;
        }
    case BlockID_Gravel: return 8;
    case BlockID_Leaves: return 9;
    case BlockID_Log:
        switch (face) {
        case BlockFace_Back:
        case BlockFace_Front:
        case BlockFace_Left:
        case BlockFace_Right: return 10;
        case BlockFace_Bottom:
        case BlockFace_Top: return 11;
        default: return 0;
        }
    case BlockID_Planks: return 12;
    case BlockID_Rose: return 13;
    case BlockID_Sand: return 14;
    case BlockID_Sandstone: return 15;
    case BlockID_Stone: return 16;
    case BlockID_TallGrass: return 17;
    case BlockID_Water: return 18;
    default: return 0;
    }
}
