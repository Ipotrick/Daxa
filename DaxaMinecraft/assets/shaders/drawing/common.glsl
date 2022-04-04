
layout(push_constant) uniform Push {
    uint globals_sb;
    uint output_image_i;
    uint chunk_buffer_i;
}
p;

layout(set = 0, binding = 3, r32ui) uniform readonly uimage3D input_images[];
layout(set = 0, binding = 3, rgba8) uniform writeonly image2D output_images[];
layout(set = 0, binding = 1) uniform sampler2DArray get_texture[];

#include <common.glsl>

uint tile_texture_index(uint block_id, uint face) {
    // clang-format off
    switch (block_id) {
    case BlockID_Bedrock:         return 0;
    case BlockID_Brick:           return 1;
    case BlockID_Cactus:          return 2;
    case BlockID_Cobblestone:     return 3;
    case BlockID_CompressedStone: return 4;
    case BlockID_DiamondOre:      return 5;
    case BlockID_Dirt:            return 6;
    case BlockID_DriedShrub:      return 7;
    case BlockID_Grass:
        switch (face) {
        case BlockFace_Back:
        case BlockFace_Front:
        case BlockFace_Left:
        case BlockFace_Right:     return 8;
        case BlockFace_Bottom:    return 6;
        case BlockFace_Top:       return 9;
        default:                  return 0;
        }
    case BlockID_Gravel:          return 10;
    case BlockID_Lava:            return 11 + int(globals.time * 6) % 8;
    case BlockID_Leaves:          return 19;
    case BlockID_Log:
        switch (face) {
        case BlockFace_Back:
        case BlockFace_Front:
        case BlockFace_Left:
        case BlockFace_Right:     return 20;
        case BlockFace_Bottom:
        case BlockFace_Top:       return 21;
        default:                  return 0;
        }
    case BlockID_MoltenRock:      return 22;
    case BlockID_Planks:          return 23;
    case BlockID_Rose:            return 24;
    case BlockID_Sand:            return 25;
    case BlockID_Sandstone:       return 26;
    case BlockID_Stone:           return 27;
    case BlockID_TallGrass:       return 28;
    case BlockID_Water:           return 29;
    default:                      return 0;
    }
    // clang-format on
}

#define VISUALIZE_STEP_COMPLEXITY 0
#define ENABLE_SHADOWS 1
#define SHOW_PICK_POS 0
#define JITTER_VIEW 1

#define ALBEDO_TEXTURE 0
#define ALBEDO_DEBUG_POS 1
#define ALBEDO_DEBUG_NRM 2
#define ALBEDO_DEBUG_DIST 4
#define ALBEDO_DEBUG_RANDOM 5
#define ALBEDO ALBEDO_TEXTURE
