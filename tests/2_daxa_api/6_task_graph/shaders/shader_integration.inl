#include <daxa/daxa.inl>
#include <daxa/utils/task_graph.inl>

struct Settings
{
    daxa_f32 set_value;
};
DAXA_DECL_BUFFER_PTR_ALIGN(Settings, 4)

// Must be in an .inl file for now.
DAXA_DECL_TASK_HEAD_BEGIN(ShaderIntegrationTaskHead)
    DAXA_TH_BUFFER_PTR(COMPUTE_SHADER_READ, daxa_BufferPtr(Settings), settings)
    DAXA_TH_IMAGE_ID(COMPUTE_SHADER_STORAGE_READ_WRITE, REGULAR_2D, image)
DAXA_DECL_TASK_HEAD_END
