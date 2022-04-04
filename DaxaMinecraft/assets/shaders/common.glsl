#extension GL_EXT_nonuniform_qualifier : enable

#include <block_info.glsl>

struct ChunkBlockPresence {
    uint x4[128];
    uint x16[2];
};

layout(set = 0, binding = 4) buffer Globals {
    mat4 viewproj_mat;
    vec4 pos;
    vec4 pick_pos;
    ivec2 frame_dim;
    float time, fov;
    uint texture_index;
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

bool load_block_presence_4x(vec3 pos) {
    ivec3 chunk_i = ivec3(pos / CHUNK_SIZE);
    if (chunk_i.x < 0 || chunk_i.x > CHUNK_N.x - 1 ||
        chunk_i.y < 0 || chunk_i.y > CHUNK_N.y - 1 ||
        chunk_i.z < 0 || chunk_i.z > CHUNK_N.z - 1) {
        return false;
    }

    const uint XY_LAYER_UINT_SIZE = 8; // 16 * 16 = 8 * 32 (sizeof uint)

    ivec3 in_chunk_p = ivec3(pos) - chunk_i * ivec3(CHUNK_SIZE);
    ivec3 x4_pos = in_chunk_p / 4;
    uint access_mask = 1 << x4_pos.x;
    access_mask = access_mask << (16 * (x4_pos.y & 0x1));

    return (chunk_block_presence(chunk_i).x4[XY_LAYER_UINT_SIZE * x4_pos.z + x4_pos.y / 2] & access_mask) != 0;
}

bool load_block_presence_16x(vec3 pos) {
    ivec3 chunk_i = ivec3(pos / CHUNK_SIZE);
    if (chunk_i.x < 0 || chunk_i.x > CHUNK_N.x - 1 ||
        chunk_i.y < 0 || chunk_i.y > CHUNK_N.y - 1 ||
        chunk_i.z < 0 || chunk_i.z > CHUNK_N.z - 1) {
        return false;
    }

    ivec3 in_chunk_p = ivec3(pos) - chunk_i * ivec3(CHUNK_SIZE);
    ivec3 x16_pos = in_chunk_p / 16;
    uint access_mask = 1 << (x16_pos.x + 4 * x16_pos.y + (x16_pos.z & 0x00000007) * 16);
    uint array_index = (x16_pos.z & 0x000000008) >> 3;

    return (chunk_block_presence(chunk_i).x16[array_index] & access_mask) != 0;
}
