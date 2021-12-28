#version 450
#extension GL_KHR_vulkan_glsl : enable

layout(location = 10) in vec3 v_pos;
layout(location = 11) in vec3 v_nrm;
layout(location = 12) in vec2 v_tex;

layout (location = 0) out vec4 o_col;
layout(set = 0, binding = 1) uniform sampler2D tex;

layout(set = 0, binding = 0) uniform SomeBuffer {
    mat4 viewproj_mat;
    vec3 cam_pos;
} u;

const uint Chunk_NX = 16;
const uint Chunk_NY = 16;
const uint Chunk_NZ = 16;
layout (set = 0, binding = 2) buffer ChunkBuffer {
    uint blocks[16 * 16 * 16];
} chunk_buffer;

uint get_tile(ivec3 coord) {
    uint index = 0; // coord.x + coord.y * Chunk_NX + coord.z * Chunk_NX * Chunk_NY
    // return 1;
    return chunk_buffer.blocks[index];
    // return uint(texture(tex, vec2(v_tex)).r * 50);
}

void main() {
    vec3 cam_diff = v_pos - u.cam_pos;
    vec3 cam_dir = normalize(cam_diff);

    uint tile_id = get_tile(ivec3(v_pos) + 8);

    o_col = vec4(vec3(tile_id), 1);
}
