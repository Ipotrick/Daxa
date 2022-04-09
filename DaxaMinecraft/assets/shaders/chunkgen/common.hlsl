#if !defined(CHUNKGEN_COMMON_HLSL)
#define CHUNKGEN_COMMON_HLSL

struct Push {
    float4 pos;
    uint globals_sb;
    uint output_image_i;
};

[[vk::push_constant]] const Push p;

#include "core.hlsl"

#endif