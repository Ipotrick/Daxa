#include "common.hlsl"
#include "chunk_blocks.hlsl"

[[vk::push_constant]] const UpdatePush p;

DAXA_DEFINE_GET_STRUCTURED_BUFFER(SharedGlobals);

[numthreads(1, 1, 1)] void main()
{
    StructuredBuffer<SharedGlobals> chunk = daxa::get_StructuredBuffer<SharedGlobals>(p.buffer_id);
    chunk[0].chunkgen(block_offset);
}
