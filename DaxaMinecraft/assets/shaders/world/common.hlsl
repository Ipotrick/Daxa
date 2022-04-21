#pragma once

struct Push {
    float4 pos;
    uint globals_sb;
    uint output_image_i;
};

[[vk::push_constant]] const Push p;
