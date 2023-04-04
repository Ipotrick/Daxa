#include <daxa/daxa.inl>
#include <daxa/utils/task_list.inl>

struct Settings
{
    daxa_f32 set_value;
};
DAXA_ENABLE_BUFFER_PTR(Settings)

#if DAXA_SHADER
#define DAXA_PADD(INDEX) uint padd##INDEX;
#else
#define DAXA_PADD(INDEX) uses.size += sizeof(daxa_u32);
#endif

DAXA_TASK_USES_BEGIN(ShaderIntegrationTaskListUses, DAXA_SLOT1)
DAXA_TASK_USE_IMAGE(image, daxa_RWImage2DArrayf32, COMPUTE_SHADER_READ_WRITE, {})
DAXA_TASK_USE_BUFFER(settings, daxa_BufferPtr(Settings), COMPUTE_SHADER_READ_ONLY)
DAXA_TASK_USES_END()