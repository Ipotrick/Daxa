#include <daxa/daxa.inl>
#include <daxa/utils/task_list.inl>

struct Testu6Alignment
{
    daxa_u32 i0;
    daxa_u32 i1;
    daxa_u32 i2;
    daxa_u64 i3;
    daxa_u32 i4;
    daxa_u64 i5[3];
    daxa_u32 i6[3];
    daxa_u64 i7;
};
DAXA_ENABLE_BUFFER_PTR(Testu6Alignment)

DAXA_INL_TASK_USES_BEGIN(TestShaderUses, DAXA_CBUFFER_SLOT0)
DAXA_INL_TASK_USE_BUFFER(align_test_src, daxa_BufferPtr(Testu6Alignment), COMPUTE_SHADER_READ_ONLY)
DAXA_INL_TASK_USE_BUFFER(align_test_dst, daxa_RWBufferPtr(Testu6Alignment), COMPUTE_SHADER_READ_WRITE)
DAXA_INL_TASK_USES_END()

// Test compilation of shared functions with aligned types
daxa_u32 test_fn_u32(daxa_u32 by_value)
{
    return (by_value * by_value) * 4;
}