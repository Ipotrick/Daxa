#define DAXA_BA_SAMPLER_BINDING 0
#define DAXA_BA_COMBINED_IMAGE_SAMPLER_BINDING 1
#define DAXA_BA_SAMPLED_IMAGE_BINDING 2
#define DAXA_BA_STORAGE_IMAGE_BINDING 3
#define DAXA_BA_STORAGE_BUFFER_BINDING 4

#define DaxaStorageBuffer StructuredBuffer
#define DaxaSampledImage3D Texture3D

template <typename T>
StructuredBuffer<T> getBuffer(uint id);
template <typename T>
SamplerState getSampler(uint id);
template <typename T>
Texture2D<T> getTexture2D(uint id);
template <typename T>
RWTexture2D<T> getRWTexture2D(uint id);
template <typename T>
Texture2DArray<T> getTexture2DArray(uint id);
template <typename T>
Texture3D<T> getTexture3D(uint id);
template <typename T>
RWTexture3D<T> getRWTexture3D(uint id);

#define DAXA_DEFINE_BA_BUFFER(Type)                                  \
    [[vk::binding(4, 0)]] StructuredBuffer<Type> BufferView##Type[]; \
    template <>                                                      \
    StructuredBuffer<Type> getBuffer(uint id) {                      \
        return BufferView##Type[id];                                 \
    }

#define DAXA_DEFINE_BA_SAMPLER(Type)                             \
    [[vk::binding(1, 0)]] SamplerState SamplerStateView##Type[]; \
    template <>                                                  \
    SamplerState getSampler<Type>(uint id) {                     \
        return SamplerStateView##Type[id];                       \
    }

#define DAXA_DEFINE_BA_RWTEXTURE2D(Type)                             \
    [[vk::binding(3, 0)]] RWTexture2D<Type> RWTexture2DView##Type[]; \
    template <>                                                      \
    RWTexture2D<Type> getRWTexture2D<Type>(uint id) {                \
        return RWTexture2DView##Type[id];                            \
    }

#define DAXA_DEFINE_BA_TEXTURE2DARRAY(Type)                                \
    [[vk::binding(1, 0)]] Texture2DArray<Type> Texture2DArrayView##Type[]; \
    template <>                                                            \
    Texture2DArray<Type> getTexture2DArray<Type>(uint id) {                \
        return Texture2DArrayView##Type[id];                               \
    }

#define DAXA_DEFINE_BA_TEXTURE3D(Type)                           \
    [[vk::binding(1, 0)]] Texture3D<Type> Texture3DView##Type[]; \
    template <>                                                  \
    Texture3D<Type> getTexture3D<Type>(uint id) {                \
        return Texture3DView##Type[id];                          \
    }

#define DAXA_DEFINE_BA_RWTEXTURE3D(Type)                             \
    [[vk::binding(3, 0)]] RWTexture3D<Type> RWTexture3DView##Type[]; \
    template <>                                                      \
    RWTexture3D<Type> getRWTexture3D<Type>(uint id) {                \
        return RWTexture3DView##Type[id];                            \
    }

DAXA_DEFINE_BA_RWTEXTURE2D(float4)
DAXA_DEFINE_BA_RWTEXTURE3D(uint)

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

#define CHUNK_SIZE 64
#define BLOCK_NX 1024
#define BLOCK_NY 256
#define BLOCK_NZ 1024
#define CHUNK_NX BLOCK_NX / CHUNK_SIZE
#define CHUNK_NY BLOCK_NY / CHUNK_SIZE
#define CHUNK_NZ BLOCK_NZ / CHUNK_SIZE
#define MAX_STEPS CHUNK_NX * CHUNK_SIZE + CHUNK_NY * CHUNK_SIZE + CHUNK_NZ * CHUNK_SIZE
#define WATER_LEVEL 160

struct ChunkBlockPresence {
    uint x2[1024];
    uint x4[256];
    uint x8[64];
    uint x16[16];
    uint x32[4]; 
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
    uint sampler_index;
    uint single_ray_steps;
    uint chunk_images[CHUNK_NZ][CHUNK_NY][CHUNK_NX];
    ChunkBlockPresence chunk_block_presence[CHUNK_NZ][CHUNK_NY][CHUNK_NX];
};
DAXA_DEFINE_BA_BUFFER(Globals)

// Log2_N = 1 : x2, 2 : x4, 3: x8 ...
template<uint N>
uint x_index(uint3 x_i) {
    enum CONSTANTS : uint {
        STRIDE = 64 / N,
    };
    return x_i.x + x_i.y * STRIDE;
}

uint x_mask(uint3 x_i) {
    return 1u << x_i.z;
}

template<uint N>
bool x_presence_in_chunk(inout ChunkBlockPresence chunk_presence, uint index, uint mask);

template<>
bool x_presence_in_chunk<2>(inout ChunkBlockPresence chunk_presence, uint index, uint mask) {
    return (chunk_presence.x2[index] & mask) != 0;
}

template<>
bool x_presence_in_chunk<4>(inout ChunkBlockPresence chunk_presence, uint index, uint mask) {
    return (chunk_presence.x4[index] & mask) != 0;
}

template<>
bool x_presence_in_chunk<8>(inout ChunkBlockPresence chunk_presence, uint index, uint mask) {
    return (chunk_presence.x8[index] & mask) != 0;
}

template<>
bool x_presence_in_chunk<16>(inout ChunkBlockPresence chunk_presence, uint index, uint mask) {
    return (chunk_presence.x16[index] & mask) != 0;
}

template<>
bool x_presence_in_chunk<32>(inout ChunkBlockPresence chunk_presence, uint index, uint mask) {
    return (chunk_presence.x16[index] & mask) != 0;
}

template<uint N>
bool x_load_presence(inout StructuredBuffer<Globals> globals, float3 world_pos) {
    uint3 chunk_i = int3(world_pos / CHUNK_SIZE);
    uint3 in_chunk_p = int3(world_pos) - chunk_i * CHUNK_SIZE;
    uint3 x_i = in_chunk_p / N;
    uint index = x_index<N>(x_i);
    uint mask = x_mask(x_i);
    return x_presence_in_chunk<N>(
        globals[0].chunk_block_presence[chunk_i.z][chunk_i.y][chunk_i.x],
        index,
        mask
    );
}