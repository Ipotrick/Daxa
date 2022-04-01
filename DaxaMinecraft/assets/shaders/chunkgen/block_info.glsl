
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
const uint BiomeID_Underworld = 4;

const uint BlockFace_Back   = 0;
const uint BlockFace_Front  = 1;
const uint BlockFace_Left   = 2;
const uint BlockFace_Right  = 3;
const uint BlockFace_Bottom = 4;
const uint BlockFace_Top    = 5;
// clang-format on

const uint BLOCK_ID_MASK = 0x0000ffff;
const uint BIOME_ID_MASK = 0x00ff0000;
const uint SDF_DIST_MASK = 0xff000000;

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
    case BlockID_Bedrock: return 0;
    case BlockID_Brick: return 1;
    case BlockID_Cactus: return 2;
    case BlockID_Cobblestone: return 3;
    case BlockID_CompressedStone: return 4;
    case BlockID_DiamondOre: return 5;
    case BlockID_Dirt: return 6;
    case BlockID_DriedShrub: return 7;
    case BlockID_Grass:
        switch (face) {
        case BlockFace_Back:
        case BlockFace_Front:
        case BlockFace_Left:
        case BlockFace_Right: return 8;
        case BlockFace_Bottom: return 6;
        case BlockFace_Top: return 9;
        default: return 0;
        }
    case BlockID_Gravel: return 10;
    case BlockID_Lava: return 11;
    case BlockID_Leaves: return 12;
    case BlockID_Log:
        switch (face) {
        case BlockFace_Back:
        case BlockFace_Front:
        case BlockFace_Left:
        case BlockFace_Right: return 13;
        case BlockFace_Bottom:
        case BlockFace_Top: return 14;
        default: return 0;
        }
    case BlockID_MoltenRock: return 15;
    case BlockID_Planks: return 16;
    case BlockID_Rose: return 17;
    case BlockID_Sand: return 18;
    case BlockID_Sandstone: return 19;
    case BlockID_Stone: return 20;
    case BlockID_TallGrass: return 21;
    case BlockID_Water: return 22;
    default: return 0;
    }
}
