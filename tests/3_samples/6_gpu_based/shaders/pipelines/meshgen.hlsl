#pragma once

#include "../chunk_blocks.hlsl"
#include "../meshlet.hlsl"

struct ChunkMeshBuildInfo
{
    uint face_count;
    uint generation_barrier;    // must reach 8*8*8 = 256 to signal
    uint allocation_barrier;    // must reach meshlet_count to signal
};

struct Push
{
    daxa::BufferId build_info_buffer_id;
    daxa::BufferId meshlet_pool_buffer_id;
    daxa::BufferId chunk_meshlets_buffer_id;
    daxa::BufferId chunk_blocks_buffer_id;
    uint3 chunk_i;
};

groupshared uint group_face_count;

// clang-format off
[numthreads(8, 8, 8)] void main(uint3 tid : SV_DispatchThreadID)
// clang-format on
{
    StructuredBuffer<ChunkMeshBuildInfo> chunk_build_info_buffer = daxa::get_StructuredBuffer<ChunkMeshBuildInfo>(p.build_info_buffer_id);
    StructuredBuffer<FaceMeshletPool> meshlet_pool_buffer = daxa::get_StructuredBuffer<FaceMeshletPool>(p.meshlet_pool_buffer_id);
    StructuredBuffer<ChunkMeshlets> chunk_meslets_buffer = daxa::get_StructuredBuffer<ChunkMeshlets>(p.chunk_meshlets_buffer_id);
    StructuredBuffer<ChunkBlocks> chunk_blocks_buffer = daxa::get_StructuredBuffer<ChunkBlocks>(p.chunk_blocks_buffer_id);

    uint flat_chunk_index = CHUNK_COUNT_X * CHUNK_COUNT_Y * p.chunk_i.z + CHUNK_COUNT_X * p.chunk_i.y + p.chunk_i.x;

    uint faces_to_write = 0;
    PackedFace packed_faces[6];

    // TODO: check faces

    bool first_thread_in_group = tid.x == 0 && tid.y == 0 && tid.z == 0;

    // check faces
    if (faces_to_write == 0 && !first_thread_in_group)
    {
        return;
    }

    uint in_group_faces_index = 0;
    InterlockedAdd(group_face_count, faces_to_write, faces_index);

    GroupMemoryBarrierWithGroupSync();

    uint group_offset = 0;
    if (first_thread_in_group)
    {
        InterlockedAdd(group_face_count, group_face_count, group_offset);
    }

    GroupMemoryBarrierWithGroupSync();

    uint in_chunk_faces_index = group_offset + in_group_faces_index;

    uint group_finish_index = 0;
    if (first_thread_in_group)
    {
        InterlockedAdd(chunk_build_info_buffer[0].generation_barrier, 1, group_finish_index);
    }
    bool lead_thread_in_chunk = first_thread_in_group && group_finish_index == 0;

    // wait on generation_barrier
    uint old_barrier_value = 0;
    do 
    {
        InterlockedOr(chunk_build_info_buffer[0].generation_barrier, 0, old_barrier_value);
    }
    while(old_barrier_value < 256);

    uint meshlet_count = (chunk_build_info_buffer[0].face_count + 256 - 1) / 256;

    if (lead_thread_in_chunk)
    {
        // allocate all meshlets for chunk
        meshlet_pool_buffer[0].lock();
        for (uint meshlet_i = 0; meshlet_i < meshlet_count; ++meshlet_count)
        {
            uint allocation = meshlet_pool_buffer[0].malloc_one();
            
            chunk_meslets_buffer[flat_chunk_index].meshlet_allocation[meshlet_i] = allocation;
            chunk_meslets_buffer[flat_chunk_index].meshlet_allocation_count = chunk_meslets_buffer[flat_chunk_index].meshlet_allocation_count + 1;
        }
        meshlet_pool_buffer[0].unlock();

        uint dummy0;
        InterlockedAdd(build_info.allocation_barrier, 1, dummy0);
    }

    // wait on allocation barrier
    old_barrier_value = 0;
    do 
    {
        InterlockedOr(chunk_build_info_buffer[0].allocation_barrier, 0, old_barrier_value);
    }
    while(old_barrier_value < meshlet_count);

    // make sure the allocations are visible
    AllMemoryBarrierWithGroupSync();

    [loop]
    for ( int face_i = 0; face_i < faces_to_write; ++face_i )
    {
        uint meshlet_i = in_chunk_faces_index / 256;
        uint meshlet_allocation = chunk_meslets_buffer[flat_chunk_index].meshlet_allocation[meshlet_i];
        uint in_meshlet_face_i = in_chunk_faces_index - meshlet_i * 256;
        meshlet_pool_buffer[0].write(meshlet_allocation, in_meshlet_face_i, packed_faces[face_i]);
        ++in_chunk_faces_index;
    }
}
