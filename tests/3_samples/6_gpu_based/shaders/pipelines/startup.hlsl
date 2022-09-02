#include "common.hlsl"
#include "chunk_blocks.hlsl"

[[vk::push_constant]] const ChunkgenComputePush p;

DAXA_DEFINE_GET_STRUCTURED_BUFFER(ChunkBlocks);

[numthreads(1, 1, 1)] void main()
{
    StructuredBuffer<ChunkBlocks> chunk = daxa::get_StructuredBuffer<ChunkBlocks>(p.buffer_id);
    chunk[0].chunkgen(block_offset);
}
