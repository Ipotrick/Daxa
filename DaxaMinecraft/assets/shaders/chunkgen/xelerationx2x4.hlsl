template<typename T>
StructuredBuffer<T> getBuffer(uint id);

#define DAXA_DEFINE_BA_BUFFER(Type)\
[[vk::binding(4,0)]] StructuredBuffer<Type> BufferView##Type[];\
template<>\
StructuredBuffer<Type> getBuffer(uint id) {\
    return BufferView##Type[id];\
}\

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
    uint chunk_images[CHUNK_NZ][CHUNK_NY][CHUNK_NX];
    ChunkBlockPresence chunk_block_presence[CHUNK_NZ][CHUNK_NY][CHUNK_NX];
};
DAXA_DEFINE_BA_BUFFER(Globals)

struct Push {
    uint4 chunk_i;
    uint globalsID;
};
[[vk::push_constant]] const Push p;

[numthreads(512,1,1)]
void main(
    uint3 invocationID : SV_DispatchThreadID
) {
    float4 pos = getBuffer<Globals>(p.globalsID).Load(0).pos;

    uint3 chunk_i = p.chunk_i.xyz;
    uint chunk_texture_id = getBuffer<Globals>(p.globalsID).Load(0).chunk_images[chunk_i.x][chunk_i.y][chunk_i.z];

    uint3 x2_i = uint3(
        invocationID.x & 0x7,
        (invocationID.x & 0x7) >> 3,
        (invocationID.x & 0x7) >> 6
    );

    x2_i += 8 * uint3(
        invocationID.x & 0x3,
        (invocationID.x & 0x3) >> 2,
        (invocationID.x & 0x3) >> 4
    );

    uint3 in_chunk_i = x2_i * 2;

    bool at_least_one_occluding = false;
    for (int x = 0; x < 2; ++x) 
    for (int y = 0; y < 2; ++y) 
    for (int z = 0; z < 2; ++z) {
        float3 world_i = float3(p.chunk_i.xyz) * 64.0 +  float3(in_chunk_i.xyz) + float3(x,y,z);

    }
}