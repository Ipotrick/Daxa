#include <daxa/daxa.inl>
#include <daxa/utils/task_list.inl>

struct Settings
{
    daxa_f32 set_value;
};
DAXA_ENABLE_BUFFER_PTR(Settings)

// Must be in an .inl file for now.
DAXA_TASK_USES_BEGIN(ShaderIntegrationTaskListUses, DAXA_CBUFFER_SLOT1)
DAXA_TASK_USE_IMAGE(shader_integration_image, daxa_RWImage2DArrayf32, COMPUTE_SHADER_READ_WRITE, {})
DAXA_TASK_USE_BUFFER(settings, daxa_BufferPtr(Settings), COMPUTE_SHADER_READ_ONLY)
DAXA_TASK_USES_END()