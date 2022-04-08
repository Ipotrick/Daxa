
layout(push_constant) uniform Push {
    uint globals_sb;
    uint output_image_i;
}
p;

layout(set = 0, binding = 3, r32ui) uniform readonly uimage3D input_images[];
layout(set = 0, binding = 3, rgba8) uniform writeonly image2D output_images[];
layout(set = 0, binding = 1) uniform sampler2DArray get_texture[];

#include <common.glsl>

uint tile_texture_index(uint block_id, uint face) {
    // clang-format off
    switch (block_id) {
    case BlockID_Debug:           return 0;
    case BlockID_Air:             return 1;
    case BlockID_Bedrock:         return 2;
    case BlockID_Brick:           return 3;
    case BlockID_Cactus:          return 4;
    case BlockID_Cobblestone:     return 5;
    case BlockID_CompressedStone: return 6;
    case BlockID_DiamondOre:      return 7;
    case BlockID_Dirt:            return 8;
    case BlockID_DriedShrub:      return 9;
    case BlockID_Grass:
        switch (face) {
        case BlockFace_Back:
        case BlockFace_Front:
        case BlockFace_Left:
        case BlockFace_Right:     return 10;
        case BlockFace_Bottom:    return 8;
        case BlockFace_Top:       return 11;
        default:                  return 0;
        }
    case BlockID_Gravel:          return 12;
    case BlockID_Lava:            return 13 + int(globals.time * 6) % 8;
    case BlockID_Leaves:          return 21;
    case BlockID_Log:
        switch (face) {
        case BlockFace_Back:
        case BlockFace_Front:
        case BlockFace_Left:
        case BlockFace_Right:     return 22;
        case BlockFace_Bottom:
        case BlockFace_Top:       return 23;
        default:                  return 0;
        }
    case BlockID_MoltenRock:      return 24;
    case BlockID_Planks:          return 25;
    case BlockID_Rose:            return 26;
    case BlockID_Sand:            return 27;
    case BlockID_Sandstone:       return 28;
    case BlockID_Stone:           return 29;
    case BlockID_TallGrass:       return 30;
    case BlockID_Water:           return 31;
    default:                      return 0;
    }
    // clang-format on
}

// Helper defines
#define ALBEDO_TEXTURE 0
#define ALBEDO_DEBUG_POS 1
#define ALBEDO_DEBUG_NRM 2
#define ALBEDO_DEBUG_DIST 4
#define ALBEDO_DEBUG_RANDOM 5

// Which information to show as the albedo
#define ALBEDO ALBEDO_TEXTURE
// Whether to disable everything else and draw just the complexity
#define VISUALIZE_STEP_COMPLEXITY 0
#define ENABLE_X16 1
// Whether to cast shadow rays
#define ENABLE_SHADOWS 1
// Whether to visualize the position that the view ray intersects
#define SHOW_PICK_POS 0
#define SHOW_SINGLE_RAY 0
// Whether to variate the sample-space coordinates based on time
#define JITTER_VIEW 0
// Number of samples per axis (so a value of 4 means 16 samples)
#define SUBSAMPLE_N 6

// Visualize x_n grid (n can be 1, 4, or 16)
#define VISUALIZE_SUBGRID 64
