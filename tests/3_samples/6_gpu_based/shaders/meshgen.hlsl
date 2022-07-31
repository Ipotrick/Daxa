
struct ChunkMeshBuildInfo
{
    uint face_count;
    uint generation_barrier;    // must reach 8*8*8 = 256 to signal
    uint allocation_barrier;    // must reach meshlet_count to signal
};

struct Push
{
    BufferId build_info_buffer;
    BufferId meshlet_pool_buffer;
    BufferId world_buffer;
    uint3 chunk_i;
};

// clang-format off
[numthreads(8, 8, 8)] void main(uint3 tid : SV_DispatchThreadID)
// clang-format on
{
    StructuredBuffer<Chunk> chunk = daxa::get_StructuredBuffer<Chunk>(p.buffer_id);

    // TODO(pahrens): replace
    build_info;
    meshlet_pool;

    uint faces_to_write = 0;
    uint packed_faces[6];

    bool is_first_thread = false;   // TODO(pahrens): fill

    // check faces
    if (faces_to_write == 0 && !is_first_thread)
    {
        return;
    }

    uint faces_index = 0;
    InterlockedAdd(build_info.face_count, faces_to_write, faces_index);

    uint meshlet = faces_index / 256;
    uint in_meshlet_face_index = faces_index - meshlet * 256;

    GroupMemoryBarrierWithGroupSync();

    if (igid.x == 0 && igid.y == 0 &&igid.z == 0)
    {
        uint dummy0;
        InterlockedAdd(build_info.generation_barrier, 1, dummy0);
    }

    // wait on generation_barrier
    uint old_barrier_value = 0;
    do 
    {
        InterlockedOr(build_info.generation_barrier, 0, old_barrier_value);
    }
    while(old_barrier_value < 256);

    uint meshlet_count = (build_info.face_count + 256 - 1) / 256;

    if (is_first_thread)
    {
        // allocate all meshlets for chunk
        meshlet_pool.lock();
        for (uint meshlet_i = 0; meshlet_i < meshlet_count; ++meshlet_count)
        {
            uint allocation = meshlet_pool.malloc_one();

            // TODO(pahrens): write the allocation.
        }
        meshlet_pool.unlock();

        uint dummy0;
        InterlockedAdd(build_info.allocation_barrier, 1, dummy0);
    }

    // make sure the allocations are visible
    AllMemoryBarrierWithGroupSync();

    // wait on allocation barrier
    old_barrier_value = 0;
    do 
    {
        InterlockedOr(build_info.allocation_barrier, 0, old_barrier_value);
    }
    while(old_barrier_value < meshlet_count);

    // TODO(pahrens): write out faces
}
