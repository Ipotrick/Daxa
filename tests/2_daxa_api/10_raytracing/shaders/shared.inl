#pragma once

#define DAXA_RAY_TRACING 1
#include <daxa/daxa.inl>

struct PushConstant
{
    daxa_u32vec2 size;
    daxa_TlasId tlas;
    daxa_ImageViewId swapchain;
};