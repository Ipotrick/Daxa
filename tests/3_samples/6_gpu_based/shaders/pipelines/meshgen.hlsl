#pragma once

#include "chunk_blocks.hlsl"
#include "meshlet.hlsl"

struct ChunkMeshBuildInfo
{
    u32 face_count;
    u32 generation_barrier; // must reach 8*8*8 = 256 to signal
    u32 allocation_barrier; // must reach meshlet_count to signal
};
DAXA_DEFINE_GET_STRUCTURED_BUFFER(ChunkMeshBuildInfo);

[[vk::push_constant]] const MeshgenComputePush p;

groupshared u32 group_face_count;

// clang-format off
[numthreads(8, 8, 8)] void main(u32vec3 tid : SV_DispatchThreadID)
// clang-format on
{
    StructuredBuffer<ChunkMeshBuildInfo> chunk_build_info_buffer = daxa::get_StructuredBuffer<ChunkMeshBuildInfo>(p.build_info_buffer_id);
    StructuredBuffer<FaceMeshletPool> meshlet_pool_buffer = daxa::get_StructuredBuffer<FaceMeshletPool>(p.meshlet_pool_buffer_id);
    StructuredBuffer<ChunkMeshlets> chunk_meshlets_buffer = daxa::get_StructuredBuffer<ChunkMeshlets>(p.chunk_meshlets_buffer_id);

    u32 flat_chunk_index = CHUNK_COUNT_X * CHUNK_COUNT_Y * p.chunk_i.z + CHUNK_COUNT_X * p.chunk_i.y + p.chunk_i.x;

    u32 faces_to_write = 0;
    PackedFace packed_faces[6];

    // TODO: check faces

    b32 first_thread_in_group = tid.x == 0 && tid.y == 0 && tid.z == 0;

    // check faces
    if (faces_to_write == 0 && !first_thread_in_group)
    {
        return;
    }

    u32 in_group_faces_index = 0;
    InterlockedAdd(group_face_count, faces_to_write, in_group_faces_index);

    GroupMemoryBarrierWithGroupSync();

    u32 group_offset = 0;
    if (first_thread_in_group)
    {
        InterlockedAdd(group_face_count, group_face_count, group_offset);
    }

    GroupMemoryBarrierWithGroupSync();

    u32 in_chunk_faces_index = group_offset + in_group_faces_index;

    u32 group_finish_index = 0;
    if (first_thread_in_group)
    {
        InterlockedAdd(chunk_build_info_buffer[0].generation_barrier, 1, group_finish_index);
    }
    b32 lead_thread_in_chunk = first_thread_in_group && group_finish_index == 0;

    // wait on generation_barrier
    u32 old_barrier_value = 0;
    do
    {
        InterlockedOr(chunk_build_info_buffer[0].generation_barrier, 0, old_barrier_value);
    } while (old_barrier_value < 256);

    u32 meshlet_count = (chunk_build_info_buffer[0].face_count + 256 - 1) / 256;

    if (lead_thread_in_chunk)
    {
        // allocate all meshlets for chunk
        FaceMeshletPool_lock(meshlet_pool_buffer);
        for (u32 meshlet_i = 0; meshlet_i < meshlet_count; ++meshlet_count)
        {
            u32 allocation = meshlet_pool_buffer[0].malloc_one();

            chunk_meshlets_buffer[flat_chunk_index].meshlet_allocations[meshlet_i] = allocation;
            chunk_meshlets_buffer[flat_chunk_index].increment_meshlet_allocation_count();
        }
        FaceMeshletPool_unlock(meshlet_pool_buffer);

        u32 dummy0;
        InterlockedAdd(chunk_build_info_buffer[0].allocation_barrier, 1, dummy0);
    }

    // wait on allocation barrier
    old_barrier_value = 0;
    do
    {
        InterlockedOr(chunk_build_info_buffer[0].allocation_barrier, 0, old_barrier_value);
    } while (old_barrier_value < meshlet_count);

    // make sure the allocations are visible
    AllMemoryBarrierWithGroupSync();

    [loop] for (i32 face_i = 0; face_i < faces_to_write; ++face_i)
    {
        u32 meshlet_i = in_chunk_faces_index / 256;
        u32 meshlet_allocation = chunk_meshlets_buffer[flat_chunk_index].meshlet_allocations[meshlet_i];
        u32 in_meshlet_face_i = in_chunk_faces_index - meshlet_i * 256;
        meshlet_pool_buffer[0].write(meshlet_allocation, in_meshlet_face_i, packed_faces[face_i]);
        ++in_chunk_faces_index;
    }
}
