#include "chunkgen/block_info.glsl"

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

layout(set = 0, binding = 0) buffer chunk_buffer_0 {
    ChunkgenBuffer chunks_0[CHUNK_MAX.x * CHUNK_MAX.y * CHUNK_MAX.z];
};
layout(set = 0, binding = 1) buffer chunk_buffer_1 {
    ChunkgenBuffer chunks_1[CHUNK_MAX.x * CHUNK_MAX.y * CHUNK_MAX.z];
};

layout(push_constant) uniform Push {
    vec3 pos;
}
p;
