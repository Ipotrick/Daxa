#pragma once

#include <daxa/daxa.inl>

struct ComputePush
{
#if DAXA_SHADERLANG == DAXA_SHADERLANG_GLSL
    daxa_RWImage2Df32 image;
#elif DAXA_SHADERLANG == DAXA_SHADERLANG_HLSL
    daxa_ImageViewId image;
#endif
    daxa_u32vec2 frame_dim;
};
