#extension GL_EXT_nonuniform_qualifier : enable

const uvec3 CHUNK_N = uvec3(8, 2, 8);
const uvec3 CHUNK_SIZE = uvec3(64);

const uint MAX_STEPS = CHUNK_N.x * CHUNK_SIZE.x + CHUNK_N.y * CHUNK_SIZE.y + CHUNK_N.z * CHUNK_SIZE.z;

layout(set = 0, binding = 4) buffer Globals {
    mat4 viewproj_mat;
    vec4 pos;
    ivec2 frame_dim;
    float time;

    uint texture_index;
    uint chunk_images[2][CHUNK_N.z][CHUNK_N.y][CHUNK_N.x];
}
globals_sb_view[];

#define globals globals_sb_view[p.globals_sb]
#define chunk_images(_ci) input_images[globals.chunk_images[p.chunk_buffer_i][_ci.z][_ci.y][_ci.x]]
#define output_image output_images[p.output_image_i]

uint get_tile(vec3 pos) {
    ivec3 chunk_i = ivec3(pos / CHUNK_SIZE);
    if (chunk_i.x < 0 || chunk_i.x > CHUNK_N.x - 1 ||
        chunk_i.y < 0 || chunk_i.y > CHUNK_N.y - 1 ||
        chunk_i.z < 0 || chunk_i.z > CHUNK_N.z - 1) {
        return 0;
    }
    return imageLoad(chunk_images(chunk_i), ivec3(pos.x, pos.y, pos.z) - chunk_i * ivec3(CHUNK_SIZE)).r;
}

uint get_block_id(vec3 p) {
    return get_tile(p) & 0xffff;
}

bool is_voxel_occluding(vec3 p) {
    return get_block_id(p) != 0;
}
