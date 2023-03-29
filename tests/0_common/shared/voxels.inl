#pragma once

#include <daxa/daxa.inl>

// clang-format off
#define BlockID_Debug           0
#define BlockID_Air             1
#define BlockID_Bedrock         2
#define BlockID_Brick           3
#define BlockID_Cactus          4
#define BlockID_Cobblestone     5
#define BlockID_CompressedStone 6
#define BlockID_DiamondOre      7
#define BlockID_Dirt            8
#define BlockID_DriedShrub      9
#define BlockID_Grass           10
#define BlockID_Gravel          11
#define BlockID_Lava            12
#define BlockID_Leaves          13
#define BlockID_Log             14
#define BlockID_MoltenRock      15
#define BlockID_Planks          16
#define BlockID_Rose            17
#define BlockID_Sand            18
#define BlockID_Sandstone       19
#define BlockID_Stone           20
#define BlockID_TallGrass       21
#define BlockID_Water           22

#define BlockFace_Left    0
#define BlockFace_Right   1
#define BlockFace_Bottom  2
#define BlockFace_Top     3
#define BlockFace_Back    4
#define BlockFace_Front   5
#define BlockFace_Cross_A 6
#define BlockFace_Cross_B 7
// clang-format on

struct UnpackedFace
{
    daxa_f32vec3 block_pos;
    daxa_f32vec3 pos;
    daxa_f32vec3 nrm;
    daxa_f32vec2 uv;
    daxa_u32 block_id;
    daxa_u32 block_face;
    daxa_u32 tex_id;
    daxa_u32 vert_id;
};
