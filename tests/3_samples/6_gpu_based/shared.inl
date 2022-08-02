#pragma once

#ifdef __cplusplus
#include <daxa/daxa.hpp>
#else
#include "daxa/daxa.hlsl"
#endif

#define SHADOW_RES 2048
#define VISUALIZE_OVERDRAW 0
#define CHUNK_SIZE 64
#define CHUNK_COUNT_X 6
#define CHUNK_COUNT_Y 6
#define CHUNK_COUNT_Z 6
#define FACE_MESHLET_POOL_SIZE (1u << 17u)
#define MESHLET_SIZE 256

struct DrawRasterPush
{
    daxa::f32vec3 chunk_pos;
    daxa::BufferId chunk_meshlets_buffer_id;
};

struct ChunkgenComputePush
{
    daxa::f32vec3 chunk_pos;
    daxa::BufferId buffer_id;
};

struct MeshgenComputePush
{
    daxa::f32vec3 chunk_pos;
    daxa::BufferId buffer_id;
};
