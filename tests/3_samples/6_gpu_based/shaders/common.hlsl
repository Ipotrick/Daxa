#pragma once

#include "daxa/daxa.hlsl"

struct Input
{
    float4x4 view_mat;
    float4x4 shadow_view_mat[3];
    daxa::ImageId shadow_depth_image[3];
    float time;
};

DAXA_DEFINE_GET_STRUCTURED_BUFFER(Input);
