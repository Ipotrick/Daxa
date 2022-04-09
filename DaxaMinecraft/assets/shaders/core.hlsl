#if !defined(CORE_HLSL)
#define CORE_HLSL

#include "block_info.hlsl"

template <typename T>
StructuredBuffer<T> getBuffer(uint id);
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

DAXA_DEFINE_BA_RWTEXTURE3D(uint)

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

BlockID load_block_id(float3 pos) {
    int3 chunk_i = int3(pos / CHUNK_SIZE);
    if (chunk_i.x < 0 || chunk_i.x > CHUNK_NX - 1 || chunk_i.y < 0 ||
        chunk_i.y > CHUNK_NY - 1 || chunk_i.z < 0 || chunk_i.z > CHUNK_NZ - 1) {
        return BlockID::Air;
    }
    return (BlockID)getRWTexture3D<uint>(
        getBuffer<Globals>(p.globals_sb)
            .Load(0)
            .chunk_images[chunk_i.x][chunk_i.y][chunk_i.z])
        [int3(pos) - chunk_i * int3(CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE)];
}

// #define X4_444_PACKING

uint x2_uint_bit_mask(uint3 x2_i) { return 1u << x2_i.x; }

uint x2_uint_array_index(uint3 x2_i) { return x2_i.y + x2_i.z * 32; }

uint3 linear_index_to_x2_packed_index(uint lin) {
    uint3 x2_i = uint3(0, 0, 0);
    x2_i.x = (lin & 0x1F);
    x2_i.y = ((lin >> 5) & 0x1F);
    x2_i.z = ((lin >> 10) & 0x1F);
    return x2_i;
}

uint x4_uint_bit_mask(uint3 x4_i) {
#ifdef X4_444_PACKING
    return 1u << ((x4_i.x & 0x3) + 4 * (x4_i.y & 0x3) + 16 * (x4_i.z & 0x1));
#else
    return 1u << ((x4_i.x) + 16 * (x4_i.y & 0x1));
#endif
}

///
/// first 2 x bits are for masking, 4x4x2 area (one uint)
/// last 2 x bits are for array indexing
/// frist 2 y bits are for masking, 4x4x2 area (one uint)
/// last 2 y bits are for array indexing
/// first 1 z bit is for masking in the 4x4x2 area (one uint)
/// second z bit is the first indexer into the array
/// last 2 z bits are for array indexing
///

uint x4_uint_array_index(uint3 x4_i) {
#ifdef X4_444_PACKING
    return ((x4_i.z) >> 2) * 2 * 16 +
           ((x4_i.z >> 1) &
            0x1) +             // the first z bit is for the mask, the second for the uint
           (x4_i.x >> 2) * 2 + // mul x by two as two uints make one block
           (x4_i.y >> 2) * 2 * 4;
#else
    return (x4_i.y >> 1) + x4_i.z * 8;
#endif
}

uint3 linear_index_to_x4_packed_index(uint lin) {
    uint3 x4_i = uint3(0, 0, 0);
#ifdef X4_444_PACKING
    x4_i.x = (lin & 0x3) | (((lin >> 6) & 0x3) << 2);
    x4_i.y = ((lin >> 2) & 0x3) | (((lin >> 8) & 0x3) << 2);
    x4_i.z = ((lin >> 4) & 0x3) | (((lin >> 10) & 0x3) << 2);
#else
    x4_i.x = (lin & 0xF);
    x4_i.y = ((lin >> 4) & 0xF);
    x4_i.z = ((lin >> 8) & 0xF);
#endif
    return x4_i;
}

uint x8_uint_bit_mask(uint3 x8_i) {
    return 1u << (x8_i.x + (x8_i.y & 0x3) * 8);
}

uint x8_uint_array_index(uint3 x8_i) {
    return ((x8_i.y & 0x4) >> 2) + x8_i.z * 2;
}

uint3 linear_index_to_x8_packed_index(uint lin) {
    return uint3(lin & 0x7, (lin >> 3) & 0x7, (lin >> 6) & 0x7);
}

uint x16_uint_bit_mask(uint3 x16_i) {
    return 1u << (x16_i.x + 4 * x16_i.y + 16 * (x16_i.z & 0x1));
}

uint x16_uint_array_index(uint3 x16_i) { return x16_i.z >> 1; }

uint x32_uint_array_index(uint3 x32_i) {
    return x32_i.x + x32_i.y * 2 + x32_i.z * 4;
}

// bool load_block_presence_2x(float3 pos) {
//     int3 chunk_i = int3(pos / CHUNK_SIZE);
//     int3 in_chunk_p = int3(pos) - chunk_i * int3(CHUNK_SIZE);
//     int3 x2_pos = in_chunk_p / 2;
//     uint access_mask = x2_uint_bit_mask(x2_pos);
//     uint uint_array_index = x2_uint_array_index(x2_pos);
//     return (chunk_block_presence(chunk_i).x2[uint_array_index] & access_mask) != 0;
// }

// bool load_block_presence_4x(float3 pos) {
//     int3 chunk_i = int3(pos / CHUNK_SIZE);
//     int3 in_chunk_p = int3(pos) - chunk_i * int3(CHUNK_SIZE);
//     int3 x4_pos = in_chunk_p / 4;
//     uint access_mask = x4_uint_bit_mask(x4_pos);
//     uint uint_array_index = x4_uint_array_index(x4_pos);
//     return (chunk_block_presence(chunk_i).x4[uint_array_index] & access_mask) != 0;
// }

// bool load_block_presence_8x(float3 pos) {
//     int3 chunk_i = int3(pos / CHUNK_SIZE);
//     int3 in_chunk_p = int3(pos) - chunk_i * int3(CHUNK_SIZE);
//     int3 x8_pos = in_chunk_p / 8;
//     uint access_mask = x8_uint_bit_mask(x8_pos);
//     uint uint_array_index = x8_uint_array_index(x8_pos);
//     return (chunk_block_presence(chunk_i).x8[uint_array_index] & access_mask) != 0;
// }

// bool load_block_presence_16x(float3 pos) {
//     int3 chunk_i = int3(pos / CHUNK_SIZE);
//     int3 in_chunk_p = int3(pos) - chunk_i * int3(CHUNK_SIZE);
//     int3 x16_pos = in_chunk_p / 16;
//     uint access_mask = x16_uint_bit_mask(x16_pos);
//     uint array_index = x16_uint_array_index(x16_pos);
//     return (chunk_block_presence(chunk_i).x16[array_index] & access_mask) != 0;
// }

// bool load_block_presence_32x(float3 pos) {
//     int3 chunk_i = int3(pos / CHUNK_SIZE);
//     int3 in_chunk_p = int3(pos) - chunk_i * int3(CHUNK_SIZE);
//     int3 x32_pos = in_chunk_p / 32;
//     uint array_index = x32_uint_array_index(x32_pos);
//     return chunk_block_presence(chunk_i).x32[array_index];
// }

// bool load_block_presence_64x(float3 pos) {
//     int3 chunk_i = int3(pos / CHUNK_SIZE);
//     return (chunk_block_presence(chunk_i).x16[0] | chunk_block_presence(chunk_i).x16[1]) != 0;
// }

#endif