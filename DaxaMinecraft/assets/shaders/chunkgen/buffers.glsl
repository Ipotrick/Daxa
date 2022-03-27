#include "chunkgen/block_info.glsl"

struct ChunkgenBuffer {
    uint blocks[CHUNK_SIZE.x * CHUNK_SIZE.y * CHUNK_SIZE.z];
};

layout(set = 0, binding = 4) buffer ChunkBufferView {
    ChunkgenBuffer chunks[CHUNK_MAX.x * CHUNK_MAX.y * CHUNK_MAX.z];
}
chunk_buffer_views[];

// layout(set = 0, binding = 0) buffer chunk_buffer_0 {
//     ChunkgenBuffer chunks_0[CHUNK_MAX.x * CHUNK_MAX.y * CHUNK_MAX.z];
// };
// layout(set = 0, binding = 1) buffer chunk_buffer_1 {
//     ChunkgenBuffer chunks_1[CHUNK_MAX.x * CHUNK_MAX.y * CHUNK_MAX.z];
// };
