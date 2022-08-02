#include "../meshlet.hlsl"

struct Push
{
    daxa::BufferId meshlet_pool_buffer_id;
    daxa::BufferId chunk_mesh_buffer_id;
    uint flat_chunk_i;
};

// clang-format off
[numthreads(256,1,1)] void main(uint3 gid : SV_GroupID)
// clang-format on
{
    StructuredBuffer<FaceMeshletPool> meshlet_pool = daxa::get_StructuredBuffer(p.meshlet_pool_buffer_id);
    StructuredBuffer<ChunkMeshlets> meshlets_buffer = daxa::get_StructuredBuffer(p.chunk_mesh_buffer_id);

    uint allocation = meshlets_buffer[p.flat_chunk_i].meshlet_allocations[gid.x];

    meshlet_pool[0].free_one( allocation, gid.x );
}
