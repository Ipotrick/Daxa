#include "common.hlsl"

struct PackedFace
{
    uint data;

    Vertex unpack(uint vert_i)
    {
        Vertex result;
        result.block_pos = float3(
            (data >> 0) & 0x3f,
            (data >> 6) & 0x3f,
            (data >> 12) & 0x3f);
        result.pos = result.block_pos;
        result.uv = instance_offsets[data_instance];
        result.block_face = (BlockFace)((data >> 18) & 0x7);
        result.block_id = (BlockID)((data >> 21) & 0x07ff);
        result.vert_id = data_instance;
        result.correct_pos(data_index);
        result.tex_id = tile_texture_index(result.block_id, result.block_face, 0);
        result.correct_uv();
        return result;
    }
};

struct FaceMeshlet
{
    PackedFace faces[256];
};

struct FaceMeshletPool
{
    FaceMeshlet meshlets[FACE_MESHLET_POOL_SIZE];
    uint free_meshlet_list[FACE_MESHLET_POOL_SIZE];
    uint free_meshlets_list_size;
    uint lock_int;

    void lock()
    {
        uint original_value;
        do
        {
            InterlockedCompareExchange(lock_int, 0, 1, original_value);
        }
        while (original_value != 0);
    }

    void unlock()
    {
        uint original_value;
        InterlockedExchange(lock_int, 0, original_value);
    }

    // Needs one thread.
    uint malloc_one()
    {
        uint allocation = free_meshlet_list[free_meshlets_list_size - 1];
        free_meshlets_list_size -= 1;
        return allocation;
    }

    // Needs 256 threads.
    void free_one(uint allocation, uint thread_id)
    {
        if (thread_id < 256)
        {
            meshlets[allocation].faces[thread_id] = 0;
            if (thread_id == 0)
            {
                lock();
                free_meshlet_list[free_meshlets_list_size] = allocation;
                free_meshlets_list_size += 1;
                unlock();
            }
        }
    }

    PackedFace read(uint allocation, uint index)
    {
        return meshlets[allocation].faces[index];
    }

    void write(uint allocation, uint index, PackedFace face)
    {
        meshlets[allocation].faces[index] = face;
    }
};
DAXA_DEFINE_GET_STRUCTURED_BUFFER(FaceMeshletPool);

struct ChunkFaces
{
    uint meshlet_allocations[1024*3];
    uint meshlet_allocation_count;
};

struct FaceBuffer
{
    uint data[32 * 32 * 32 * 6];
};

struct MeshletIndirectDraw
{
    u32 vertex_count;
    u32 instance_count;
    u32 first_vertex;
    u32 first_instance;
    
};

struct ChunkMeshlets
{
    uint meshlet_allocations[1024 * 3];
    uint meshlet_allocation_count;
}
DAXA_DEFINE_GET_STRUCTURED_BUFFER(ChunkMeshlets);