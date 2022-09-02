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
    daxa::BufferId globals_buffer_id;
};

struct ChunkgenComputePush
{
    daxa::f32vec3 chunk_pos;
    daxa::BufferId buffer_id;
};

struct MeshgenComputePush
{
    daxa::BufferId build_info_buffer_id;
    daxa::BufferId meshlet_pool_buffer_id;
    daxa::BufferId chunk_meshlets_buffer_id;
    daxa::BufferId chunk_blocks_buffer_id;
    daxa::i32vec3 chunk_i;
};

struct Input
{
    daxa::f32mat4x4 view_mat;
    daxa::f32 time;
};
DAXA_REGISTER_STRUCT_GET_BUFFER(Input);

struct IndirectDrawParam
{
    daxa::u32 vertex_count;
    daxa::u32 instance_count;
    daxa::u32 first_vertex;
    daxa::u32 first_instance;
};

struct IndirectDrawBuffer
{
    IndirectDrawParam param_entries[CHUNK_COUNT_X * CHUNK_COUNT_Y * CHUNK_COUNT_Z];
};

struct ChunkDrawInfo
{
    daxa::u32vec3 chunk_index;
};
DAXA_REGISTER_STRUCT_GET_BUFFER(ChunkDrawInfo);

struct SharedGlobals
{
    daxa::BufferId indirect_draw_buffer_id;
    daxa::BufferId chunk_draw_infos_buffer_id;
};
DAXA_REGISTER_STRUCT_GET_BUFFER(SharedGlobals);
