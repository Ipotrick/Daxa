#pragma once

#define DAXA_RAY_TRACING 1
#include <daxa/daxa.inl>

#define MAX_PRIMITIVES 2

struct CameraView
{
    daxa_f32mat4x4 inv_view;
    daxa_f32mat4x4 inv_proj;
};
DAXA_DECL_BUFFER_PTR(CameraView)

struct Aabb
{
    daxa_f32vec3 minimum;
    daxa_f32vec3 maximum;
};

struct Aabbs
{
    Aabb aabbs[MAX_PRIMITIVES];
};
DAXA_DECL_BUFFER_PTR(Aabbs)

struct PushConstant
{
    daxa_u32 frame;
    daxa_u32vec2 size;
    daxa_TlasId tlas;
    daxa_ImageViewId swapchain;
    daxa_BufferPtr(CameraView) camera_buffer;
    daxa_BufferPtr(Aabbs) aabb_buffer;
};

struct rayLight
{
    daxa_f32vec3 inHitPosition;
    daxa_f32 outLightDistance;
    daxa_f32vec3 outLightDir;
    daxa_f32 outIntensity;
};
struct hitPayload
{
    daxa_f32vec3 hitValue;
    daxa_u32 seed;
};

struct Ray
{
    daxa_f32vec3 origin;
    daxa_f32vec3 direction;
};