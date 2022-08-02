#include "common.hlsl"
#include "chunk_blocks.hlsl"

[[vk::push_constant]] const ChunkgenComputePush p;

DAXA_DEFINE_GET_STRUCTURED_BUFFER(ChunkBlocks)

// clang-format off
[numthreads(8, 8, 8)] void main(uint3 block_offset : SV_DispatchThreadID)
// clang-format on
{
    StructuredBuffer<ChunkBlocks> chunk = daxa::get_StructuredBuffer<ChunkBlocks>(p.buffer_id);
    chunk[0].chunkgen(block_offset);
}
