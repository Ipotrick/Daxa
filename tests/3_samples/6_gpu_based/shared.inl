#pragma once

#include <daxa/daxa.inl>

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
    BufferId globals_buffer_id;
};

struct ChunkgenComputePush
{
    f32vec3 chunk_pos;
    BufferId buffer_id;
};

struct MeshgenComputePush
{
    BufferId build_info_buffer_id;
    BufferId meshlet_pool_buffer_id;
    BufferId chunk_meshlets_buffer_id;
    BufferId chunk_blocks_buffer_id;
    i32vec3 chunk_i;
};

struct Input
{
    f32mat4x4 view_mat;
    f32 time;
};
DAXA_DEFINE_GET_STRUCTURED_BUFFER(Input);

struct IndirectDrawParam
{
    u32 vertex_count;
    u32 instance_count;
    u32 first_vertex;
    u32 first_instance;
};

struct IndirectDrawBuffer
{
    IndirectDrawParam param_entries[CHUNK_COUNT_X * CHUNK_COUNT_Y * CHUNK_COUNT_Z];
};

struct ChunkDrawInfo
{
    u32vec3 chunk_index;
};
DAXA_DEFINE_GET_STRUCTURED_BUFFER(ChunkDrawInfo);

struct SharedGlobals
{
    BufferId indirect_draw_buffer_id;
    BufferId chunk_draw_infos_buffer_id;
};
DAXA_DEFINE_GET_STRUCTURED_BUFFER(SharedGlobals);
