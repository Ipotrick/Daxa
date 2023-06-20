#include <daxa/daxa.inl>
#include <daxa/utils/task_list.inl>

struct Settings
{
    daxa_f32 set_value;
};
DAXA_DECL_BUFFER_PTR_ALIGN(Settings, 4)

// Must be in an .inl file for now.
DAXA_DECL_TASK_USES_BEGIN(ShaderIntegrationTask, DAXA_UNIFORM_BUFFER_SLOT1)
    DAXA_TASK_USE_BUFFER(settings, daxa_BufferPtr(Settings), COMPUTE_SHADER_READ)
    DAXA_TASK_USE_IMAGE(image, REGULAR_2D, COMPUTE_SHADER_READ_WRITE)
DAXA_DECL_TASK_USES_END()
