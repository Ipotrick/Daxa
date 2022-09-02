#include "common.hlsl"
#include "chunk_blocks.hlsl"

static const f32vec2 instance_offsets[6] = {
    f32vec2(+0.0, +0.0),
    f32vec2(+1.0, +0.0),
    f32vec2(+0.0, +1.0),
    f32vec2(+1.0, +0.0),
    f32vec2(+1.0, +1.0),
    f32vec2(+0.0, +1.0),
};
static const f32vec3 cross_instance_positions[6] = {
    f32vec3(+0.0 + 0.2, +0.2, +1.0 - 0.2),
    f32vec3(+1.0 - 0.2, +0.2, +0.0 + 0.2),
    f32vec3(+0.0 + 0.2, +1.0, +1.0 - 0.2),
    f32vec3(+1.0 - 0.2, +0.2, +0.0 + 0.2),
    f32vec3(+1.0 - 0.2, +1.0, +0.0 + 0.2),
    f32vec3(+0.0 + 0.2, +1.0, +1.0 - 0.2),
};

struct FaceVertex
{
    f32vec3 block_pos;
    f32vec3 pos;
    f32vec3 nrm;
    f32vec2 uv;
    BlockID block_id;
    BlockFace block_face;
    u32 tex_id;
    u32 vert_id;

    void correct_pos(u32 face_id)
    {
        f32vec3 cross_uv = cross_instance_positions[vert_id];
        switch (block_face)
        {
            // clang-format off
        case BlockFace::Left:   pos += f32vec3(1.0,        uv.x,       uv.y), nrm = f32vec3(+1.0, +0.0, +0.0); break;
        case BlockFace::Right:  pos += f32vec3(0.0,        1.0 - uv.x, uv.y), nrm = f32vec3(-1.0, +0.0, +0.0); break;
        case BlockFace::Bottom: pos += f32vec3(1.0 - uv.x, 1.0,        uv.y), nrm = f32vec3(+0.0, +1.0, +0.0); break;
        case BlockFace::Top:    pos += f32vec3(uv.x,       0.0,        uv.y), nrm = f32vec3(+0.0, -1.0, +0.0); break;
        case BlockFace::Back:   pos += f32vec3(uv.x,       uv.y,        1.0), nrm = f32vec3(+0.0, +0.0, +1.0); break;
        case BlockFace::Front:  pos += f32vec3(1.0 - uv.x, uv.y,        0.0), nrm = f32vec3(+0.0, +0.0, -1.0); break;
            // clang-format on

        case BlockFace::Cross_A:
        {
            if (face_id % 2 == 0)
            {
                pos += cross_uv;
                nrm = f32vec3(0.707, 0, 0.707);
            }
            else
            {
                pos += f32vec3(1.0 - cross_uv.x, cross_uv.y, 1.0 - cross_uv.z);
                nrm = f32vec3(-0.707, 0, -0.707);
            }
            pos.x += rand(block_pos.x + 2.0 * block_pos.z - block_pos.y) * 0.5 - 0.25;
            pos.z += rand(block_pos.z + 2.0 * block_pos.x + block_pos.y) * 0.5 - 0.25;
        }
        break;
        case BlockFace::Cross_B:
        {
            if (face_id % 2 == 0)
            {
                pos += f32vec3(cross_uv.x, cross_uv.y, 1.0 - cross_uv.z);
                nrm = f32vec3(-0.707, 0, 0.707);
            }
            else
            {
                pos += f32vec3(1.0 - cross_uv.x, cross_uv.y, cross_uv.z);
                nrm = f32vec3(0.707, 0, -0.707);
            }
            pos.x += rand(block_pos.x + 2.0 * block_pos.z - block_pos.y) * 0.5 - 0.25;
            pos.z += rand(block_pos.z + 2.0 * block_pos.x + block_pos.y) * 0.5 - 0.25;
        }
        break;
        }
    }

    void correct_uv()
    {
        switch (block_face)
        {
            // clang-format off
        case BlockFace::Left:    uv = f32vec2(1.0, 1.0) + f32vec2(-uv.y, -uv.x); break;
        case BlockFace::Right:   uv = f32vec2(0.0, 0.0) + f32vec2(+uv.y, +uv.x); break;
        case BlockFace::Bottom:  uv = f32vec2(0.0, 0.0) + f32vec2(+uv.x, +uv.y); break;
        case BlockFace::Top:     uv = f32vec2(0.0, 0.0) + f32vec2(+uv.x, +uv.y); break;
        case BlockFace::Back:    uv = f32vec2(1.0, 1.0) + f32vec2(-uv.x, -uv.y); break;
        case BlockFace::Front:   uv = f32vec2(1.0, 1.0) + f32vec2(-uv.x, -uv.y); break;
            // clang-format on

        case BlockFace::Cross_A: uv = f32vec2(1.0, 1.0) + f32vec2(-uv.x, -uv.y); break;
        case BlockFace::Cross_B: uv = f32vec2(1.0, 1.0) + f32vec2(-uv.x, -uv.y); break;
        }

        if (tex_id == 11 || tex_id == 8)
        {
            u32 i = (u32)(rand(block_pos) * 8);
            switch (i)
            {
            case 0: uv = f32vec2(0 + uv.x, 0 + uv.y); break;
            case 1: uv = f32vec2(1 - uv.x, 0 + uv.y); break;
            case 2: uv = f32vec2(1 - uv.x, 1 - uv.y); break;
            case 3: uv = f32vec2(0 + uv.x, 1 - uv.y); break;

            case 4: uv = f32vec2(0 + uv.y, 0 + uv.x); break;
            case 5: uv = f32vec2(1 - uv.y, 0 + uv.x); break;
            case 6: uv = f32vec2(1 - uv.y, 1 - uv.x); break;
            case 7: uv = f32vec2(0 + uv.y, 1 - uv.x); break;
            }
        }
    }
};

struct PackedFace
{
    u32 data;

    FaceVertex unpack(u32 vert_i)
    {
        u32 data_index = vert_i / 6;
        u32 data_instance = vert_i - data_index * 6;
        FaceVertex result;
        result.block_pos = f32vec3(
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
    u32 free_meshlet_list[FACE_MESHLET_POOL_SIZE];
    u32 free_meshlets_list_size;
    u32 lock_int;

    // Needs one thread.
    u32 malloc_one()
    {
        u32 allocation = free_meshlet_list[free_meshlets_list_size - 1];
        free_meshlets_list_size -= 1;
        return allocation;
    }

    PackedFace read(u32 allocation, u32 index)
    {
        return meshlets[allocation].faces[index];
    }

    void write(u32 allocation, u32 index, PackedFace face)
    {
        meshlets[allocation].faces[index] = face;
    }

    void increment_free_meshlet_list_size() { ++free_meshlets_list_size; }
};
DAXA_DEFINE_GET_STRUCTURED_BUFFER(FaceMeshletPool);

void FaceMeshletPool_lock(in out StructuredBuffer<FaceMeshletPool> meshlet_pool)
{
    u32 original_value;
    do
    {
        InterlockedCompareExchange(meshlet_pool[0].lock_int, 0, 1, original_value);
    } while (original_value != 0);
}

void FaceMeshletPool_unlock(in out StructuredBuffer<FaceMeshletPool> meshlet_pool)
{
    u32 original_value;
    InterlockedExchange(meshlet_pool[0].lock_int, 0, original_value);
}

// Needs 256 threads.
void FaceMeshletPool_free_one(in out StructuredBuffer<FaceMeshletPool> meshlet_pool, u32 allocation, u32 thread_id)
{
    if (thread_id < 256)
    {
        meshlet_pool[0].meshlets[allocation].faces[thread_id].data = 0;
        if (thread_id == 0)
        {
            FaceMeshletPool_lock(meshlet_pool);
            meshlet_pool[0].free_meshlet_list[meshlet_pool[0].free_meshlets_list_size] = allocation;
            meshlet_pool[0].increment_free_meshlet_list_size();
            // meshlet_pool[0].free_meshlets_list_size++;
            FaceMeshletPool_unlock(meshlet_pool);
        }
    }
}

struct ChunkFaces
{
    u32 meshlet_allocations[1024 * 3];
    u32 meshlet_allocation_count;

    void increment_meshlet_allocation_count() { ++meshlet_allocation_count; }
};

struct FaceBuffer
{
    u32 data[32 * 32 * 32 * 6];
};

struct ChunkMeshlets
{
    u32 meshlet_allocations[1024 * 3];
    u32 meshlet_allocation_count;

    void increment_meshlet_allocation_count() { ++meshlet_allocation_count; }
};
DAXA_DEFINE_GET_STRUCTURED_BUFFER(ChunkMeshlets);