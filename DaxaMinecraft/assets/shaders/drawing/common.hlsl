#if !defined(DRAWING_COMMON_HLSL)
#define DRAWING_COMMON_HLSL

struct Push {
    uint globals_sb;
    uint output_image_i;
};

[[vk::push_constant]] const Push p;

// layout(set = 0, binding = 3, r32ui) uniform readonly uimage3D input_images[];
// layout(set = 0, binding = 3, rgba8) uniform writeonly image2D output_images[];
// layout(set = 0, binding = 1) uniform sampler2DArray get_texture[];

#include "core.hlsl"

uint tile_texture_index(BlockID block_id, BlockFace face) {
    // clang-format off
    switch (block_id) {
    case BlockID::Debug:           return 0;
    case BlockID::Air:             return 1;
    case BlockID::Bedrock:         return 2;
    case BlockID::Brick:           return 3;
    case BlockID::Cactus:          return 4;
    case BlockID::Cobblestone:     return 5;
    case BlockID::CompressedStone: return 6;
    case BlockID::DiamondOre:      return 7;
    case BlockID::Dirt:            return 8;
    case BlockID::DriedShrub:      return 9;
    case BlockID::Grass:
        switch (face) {
        case BlockFace::Back:
        case BlockFace::Front:
        case BlockFace::Left:
        case BlockFace::Right:     return 10;
        case BlockFace::Bottom:    return 8;
        case BlockFace::Top:       return 11;
        default:                   return 0;
        }
    case BlockID::Gravel:          return 12;
    case BlockID::Lava:            return 13 + int(globals.time * 6) % 8;
    case BlockID::Leaves:          return 21;
    case BlockID::Log:
        switch (face) {
        case BlockFace::Back:
        case BlockFace::Front:
        case BlockFace::Left:
        case BlockFace::Right:     return 22;
        case BlockFace::Bottom:
        case BlockFace::Top:       return 23;
        default:                   return 0;
        }
    case BlockID::MoltenRock:      return 24;
    case BlockID::Planks:          return 25;
    case BlockID::Rose:            return 26;
    case BlockID::Sand:            return 27;
    case BlockID::Sandstone:       return 28;
    case BlockID::Stone:           return 29;
    case BlockID::TallGrass:       return 30;
    case BlockID::Water:           return 31;
    default:                       return 0;
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
#define ENABLE_X16 0
#define ENABLE_X64 0
// Whether to cast shadow rays
#define ENABLE_SHADOWS 1
// Whether to visualize the position that the view ray intersects
#define SHOW_PICK_POS 0
#define SHOW_SINGLE_RAY 0
// Whether to variate the sample-space coordinates based on time
#define JITTER_VIEW 0
// Number of samples per axis (so a value of 4 means 16 samples)
#define SUBSAMPLE_N 1

// Visualize x_n grid (n can be 1, 4, or 16)
#define VISUALIZE_SUBGRID 1

#endif