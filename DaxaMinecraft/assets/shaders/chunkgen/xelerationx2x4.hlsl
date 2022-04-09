#define DAXA_BA_SAMPLER_BINDING 0
#define DAXA_BA_COMBINED_IMAGE_SAMPLER_BINDING 1
#define DAXA_BA_SAMPLED_IMAGE_BINDING 2
#define DAXA_BA_STORAGE_IMAGE_BINDING 3
#define DAXA_BA_STORAGE_BUFFER_BINDING 4

#define DaxaStorageBuffer StructuredBuffer
#define DaxaSampledImage3D Texture3D

template<typename T>
StructuredBuffer<T> getBuffer(uint id);
template<typename T>
Texture3D<T> getTexture3D(uint id);

#define DAXA_DEFINE_BA_BUFFER(Type)\
[[vk::binding(DAXA_BA_STORAGE_BUFFER_BINDING,0)]] StructuredBuffer<Type> BufferView##Type[];\
template<>\
StructuredBuffer<Type> getBuffer(uint id) {\
    return BufferView##Type[id];\
}\

#define DAXA_DEFINE_BA_TEXTURE3D(Type)\
[[vk::binding(DAXA_BA_SAMPLED_IMAGE_BINDING,0)]] Texture3D<Type> Texture3DView##Type[];\
template<>\
Texture3D<Type> getTexture3D<Type>(uint id) {\
    return Texture3DView##Type[id];\
}\

DAXA_DEFINE_BA_TEXTURE3D(uint)

#include "chunkgen/include.hlsl"

// clang-format off
enum class BlockID : uint {
    Debug           = 0,
    Air             = 1,
    Bedrock         = 2,
    Brick           = 3,
    Cactus          = 4,
    Cobblestone     = 5,
    CompressedStone = 6,
    DiamondOre      = 7,
    Dirt            = 8,
    DriedShrub      = 9,
    Grass           = 10,
    Gravel          = 11,
    Lava            = 12,
    Leaves          = 13,
    Log             = 14,
    MoltenRock      = 15,
    Planks          = 16,
    Rose            = 17,
    Sand            = 18,
    Sandstone       = 19,
    Stone           = 20,
    TallGrass       = 21,
    Water           = 22,
};

enum class BiomeID : uint {
    Plains     = 0,
    Forest     = 1,
    Desert     = 2,
    Beach      = 3,
    Caves      = 4,
    Underworld = 5,
};

enum class BlockFace : uint {
    Back   = 0,
    Front  = 1,
    Left   = 2,
    Right  = 3,
    Bottom = 4,
    Top    = 5,
};

// clang-format on

bool is_block_occluding(BlockID block_id) {
    switch (block_id) {
    case BlockID::Air:
        return false;
    default: return true;
    }
}

bool is_transparent(BlockID block_id) {
    switch (block_id) {
    case BlockID::Air:
    case BlockID::DriedShrub:
    case BlockID::Lava:
    case BlockID::Leaves:
    case BlockID::Rose:
    case BlockID::TallGrass:
    case BlockID::Water:
        return true;
    default: return false;
    }
}

uint x2_uint_bit_mask(uint3 x2_i) {
    return 1u << x2_i.z;
}

uint x2_uint_array_index(uint3 x2_i) {
    return x2_i.x + x2_i.y * 32;
}

uint x4_uint_bit_mask(uint3 x4_i) {
    return 1u << ((x4_i.z & 0xF) + 16 * (x4_i.x & 0x1));
}

uint x4_uint_array_index(uint3 x4_i) {
    return (x4_i.x >> 1 /* / 2 */) + x4_i.y * 8 /* 16 / 2 */;
}

enum INTEGER_CONSTANTS {
    CHUNK_SIZE = 64,
    BLOCK_NX = 1024,
    BLOCK_NY = 256,
    BLOCK_NZ = 1024,
    CHUNK_NX = BLOCK_NX / CHUNK_SIZE,
    CHUNK_NY = BLOCK_NY / CHUNK_SIZE,
    CHUNK_NZ = BLOCK_NZ / CHUNK_SIZE,
    MAX_STEPS = CHUNK_NX * CHUNK_SIZE + CHUNK_NY * CHUNK_SIZE + CHUNK_NZ * CHUNK_SIZE,
    WATER_LEVEL = 160,
};

struct ChunkBlockPresence {
    uint x2[1024];
    uint x4[128];
    uint x8[16];
    uint x16[2];
    bool x32[8]; 
};

struct Globals {
    float4x4 viewproj_mat;
    float4 pos;
    float4 single_ray_pos;
    float4 single_ray_nrm;
    float4 pick_pos;
    int2 frame_dim;
    float time, fov;
    uint texture_index;
    uint single_ray_steps;
    uint chunk_images[CHUNK_NX][CHUNK_NY][CHUNK_NZ];
    ChunkBlockPresence chunk_block_presence[CHUNK_NX][CHUNK_NY][CHUNK_NZ];
};
DAXA_DEFINE_BA_BUFFER(Globals)

struct Push {
    uint4 chunk_i;
    uint globalsID;
};
[[vk::push_constant]] const Push p;

groupshared uint local_x2_copy[4][4];

/*
    x: in workgroup index
    y: in chunk index
    z: chunk index
*/
[numthreads(512,1,1)]
void main(
    uint3 group_local_ID : SV_GroupThreadID,
    uint3 group_ID : SV_GroupID
) {
    uint3 x2_i = uint3(
        (group_local_ID.x >> 5) & 0x3, 
        (group_local_ID.x >> 7) & 0x3, 
        (group_local_ID.x & 0x1F)
    ) + 4 * group_ID;
    uint3 chunk_i = p.chunk_i.xyz;
    StructuredBuffer<Globals> globals = getBuffer<Globals>(p.globalsID);
    Texture3D<uint> chunk = getTexture3D<uint>(globals[0].chunk_images[chunk_i.x][chunk_i.y][chunk_i.z]);
    uint3 in_chunk_i = x2_i * 2;

    bool at_least_one_occluding = false;
    for (int x = 0; x < 2; ++x) 
    for (int y = 0; y < 2; ++y) 
    for (int z = 0; z < 2; ++z) {
        int3 local_i = in_chunk_i + int3(x,y,z);
        at_least_one_occluding = at_least_one_occluding || is_block_occluding((BlockID)chunk[local_i]);
    }

    uint result = 0;
    if (at_least_one_occluding) {
        result = x2_uint_bit_mask(x2_i);
    }
    result = WaveActiveBitOr(result);
    if (WaveIsFirstLane()) {
        uint index = x2_uint_array_index(x2_i);
        globals[0].chunk_block_presence[chunk_i.x][chunk_i.y][chunk_i.z].x2[index] = result;
        local_x2_copy[x2_i.x][x2_i.y] = result;
    }

    if (group_local_ID.x >= 64) {
        return;
    }

    uint3 x4_i = uint3(
        (group_local_ID.x >> 4) & 0x1, 
        (group_local_ID.x >> 5) & 0x1, 
        group_local_ID.x & 0xF
    );
    x2_i = x4_i * 2;

    at_least_one_occluding = false;
    for (int x = 0; x < 2; ++x) 
    for (int y = 0; y < 2; ++y) 
    for (int z = 0; z < 2; ++z) {
        int3 local_i = x2_i + int3(x,y,z);
        uint mask = x2_uint_bit_mask(local_i);
        //int2 local_x2_copy_index = int3(
        //    //local_i & 0x
        //);


        at_least_one_occluding = at_least_one_occluding || is_block_occluding((BlockID)chunk[local_i]);
    }
}