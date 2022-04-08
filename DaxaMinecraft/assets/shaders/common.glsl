#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_shader_16bit_storage : enable

#include <block_info.glsl>

struct ChunkBlockPresence {
    uint x2[1024];
    uint x4[128];
    uint x8[16];
    uint x16[2];
    bool x32[8]; 
};

layout(set = 0, binding = 4) buffer Globals {
    mat4 viewproj_mat;
    vec4 pos;
    vec4 single_ray_pos;
    vec4 single_ray_nrm;
    vec4 pick_pos;
    ivec2 frame_dim;
    float time, fov;
    uint texture_index;
    uint single_ray_steps;
    uint chunk_images[CHUNK_N.z][CHUNK_N.y][CHUNK_N.x];
    ChunkBlockPresence chunk_block_presence[CHUNK_N.z][CHUNK_N.y][CHUNK_N.x];
}
globals_sb_view[];

#define globals globals_sb_view[p.globals_sb]
#define chunk_images(_ci) input_images[globals.chunk_images[_ci.z][_ci.y][_ci.x]]
#define chunk_block_presence(_ci) globals.chunk_block_presence[_ci.z][_ci.y][_ci.x]
#define output_image output_images[p.output_image_i]

uint load_tile(vec3 pos) {
    ivec3 chunk_i = ivec3(pos / CHUNK_SIZE);
    if (chunk_i.x < 0 || chunk_i.x > CHUNK_N.x - 1 ||
        chunk_i.y < 0 || chunk_i.y > CHUNK_N.y - 1 ||
        chunk_i.z < 0 || chunk_i.z > CHUNK_N.z - 1) {
        return BlockID_Air;
    }
    return imageLoad(chunk_images(chunk_i), ivec3(pos) - chunk_i * ivec3(CHUNK_SIZE)).r;
}

uint load_block_id(vec3 p) { return get_block_id(load_tile(p)); }
uint load_biome_id(vec3 p) { return get_biome_id(load_tile(p)); }
uint load_sdf_dist(vec3 p) { return get_sdf_dist(load_tile(p)); }

// #define X4_444_PACKING

uint x2_uint_bit_mask(uvec3 x2_i) {
    return 1 << x2_i.x;
}

uint x2_uint_array_index(uvec3 x2_i) {
    return x2_i.y + x2_i.z * 32;
}

uvec3 linear_index_to_x2_packed_index(uint linear) {
    uvec3 x2_i = uvec3(0);
    x2_i.x = (linear & 0x1F);
    x2_i.y = ((linear >> 5) & 0x1F);
    x2_i.z = ((linear >> 10) & 0x1F);
    return x2_i;
}

uint x4_uint_bit_mask(uvec3 x4_i) {
#ifdef X4_444_PACKING
    return 1 << ((x4_i.x & 0x3) + 4 * (x4_i.y & 0x3) + 16 * (x4_i.z & 0x1));
#else 
    return 1 << ((x4_i.x) + 16 * (x4_i.y & 0x1));
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

uint x4_uint_array_index(uvec3 x4_i) {
#ifdef X4_444_PACKING
    return ((x4_i.z) >> 2) * 2 * 16 +
           ((x4_i.z >> 1) & 0x1) + // the first z bit is for the mask, the second for the uint
           (x4_i.x >> 2) * 2 +     // mul x by two as two uints make one block
           (x4_i.y >> 2) * 2 * 4;
#else
    return (x4_i.y >> 1) + x4_i.z * 8;
#endif
}

uvec3 linear_index_to_x4_packed_index(uint linear) {
    uvec3 x4_i = uvec3(0);
#ifdef X4_444_PACKING
    x4_i.x = (linear & 0x3) | (((linear >> 6) & 0x3) << 2);
    x4_i.y = ((linear >> 2) & 0x3) | (((linear >> 8) & 0x3) << 2);
    x4_i.z = ((linear >> 4) & 0x3) | (((linear >> 10) & 0x3) << 2);
#else
    x4_i.x = (linear & 0xF);
    x4_i.y = ((linear >> 4) & 0xF);
    x4_i.z = ((linear >> 8) & 0xF);
#endif
    return x4_i;
}

uint x16_uint_bit_mask(uvec3 x16_i) {
    return 1 << (x16_i.x + 4 * x16_i.y + 16 * (x16_i.z & 0x1));
}

uint x16_uint_array_index(uvec3 x16_i) {
    return x16_i.z >> 1;
}

uint x32_uint_array_index(uvec3 x32_i) {
    return x32_i.x + x32_i.y * 2 + x32_i.z * 4;
}

bool load_block_presence_4x(vec3 pos) {
    ivec3 chunk_i = ivec3(pos / CHUNK_SIZE);

    ivec3 in_chunk_p = ivec3(pos) - chunk_i * ivec3(CHUNK_SIZE);
    ivec3 x4_pos = in_chunk_p / 4;
    uint access_mask = x4_uint_bit_mask(x4_pos);
    uint uint_array_index = x4_uint_array_index(x4_pos);
    return (chunk_block_presence(chunk_i).x4[uint_array_index] & access_mask) != 0;
}

bool load_block_presence_16x(vec3 pos) {
    ivec3 chunk_i = ivec3(pos / CHUNK_SIZE);

    ivec3 in_chunk_p = ivec3(pos) - chunk_i * ivec3(CHUNK_SIZE);
    ivec3 x16_pos = in_chunk_p / 16;
    uint access_mask = x16_uint_bit_mask(x16_pos);
    uint array_index = x16_uint_array_index(x16_pos);

    return (chunk_block_presence(chunk_i).x16[array_index] & access_mask) != 0;
}

bool load_block_presence_32x(vec3 pos) {
    ivec3 chunk_i = ivec3(pos / CHUNK_SIZE);

    ivec3 in_chunk_p = ivec3(pos) - chunk_i * ivec3(CHUNK_SIZE);
    ivec3 x32_pos = in_chunk_p / 32;
    uint array_index = x32_uint_array_index(x32_pos);

    return chunk_block_presence(chunk_i).x32[array_index];
}

bool load_block_presence_64x(vec3 pos) {
    ivec3 chunk_i = ivec3(pos / CHUNK_SIZE);

    return (chunk_block_presence(chunk_i).x16[0] | chunk_block_presence(chunk_i).x16[1]) != 0;
}