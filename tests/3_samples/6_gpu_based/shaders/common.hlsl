#pragma once

#include "daxa/daxa.hlsl"

enum class BlockID : uint
{
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

enum class BlockFace : uint
{
    Left,
    Right,
    Bottom,
    Top,
    Back,
    Front,

    Cross_A,
    Cross_B,
};

struct Input
{
    float4x4 view_mat;
    float4x4 shadow_view_mat[3];
    daxa::ImageId shadow_depth_image[3];
    float time;
};

DAXA_DEFINE_GET_STRUCTURED_BUFFER(Input);
