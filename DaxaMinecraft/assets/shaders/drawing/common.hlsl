#pragma once

struct Push {
    uint globals_sb;
    uint output_image_i;
};

[[vk::push_constant]] const Push p;

#include "drawing/defines.hlsl"
