#if DAXA_BUILT_WITH_UTILS_TASK_GRAPH || true

#include "../impl_core.hpp"

#include <algorithm>
#include <iostream>
#include <set>

#include <utility>

#include "impl_task_graph.hpp"
#include "impl_task_graph_debug.hpp"

namespace daxa
{
    void test_ArenaDynamicArray8k()
    {
        std::array<u8, 256> mem = {};
        MemoryArena arena = { "Test Arena", mem };

        ArenaDynamicArray8k<u32> a{&arena};

        for (u32 i = 0; i < 5; ++i)
            a.push_back(i);
        
        printf("a init 0:\n");
        for (auto e : a)
        {
            printf("  %i\n", e);
        }

        a.insert(a.begin() + 2, 128);
        
        
        printf("a insert 128 at 2:\n");
        for (auto e : a)
        {
            printf("  %i\n", e);
        }
        
        for (u32 i = 8; i < 10; ++i)
            a.push_back(i);
        
        printf("a init 2:\n");
        for (auto e : a)
        {
            printf("  %i\n", e);
        }

        ArenaDynamicArray8k<u32> b{&arena};

        for (u32 i = 5; i < 8; ++i)
            b.push_back(i);
        
        printf("b init:\n");
        for (auto e : a)
        {
            printf("  %i\n", e);
        }

        a.insert(a.begin() + 6, b.begin(), b.end());
        printf("a after b insert into a at 6:\n");
        for (auto e : a)
        {
            printf("  %i\n", e);
        }

        a.erase(a.begin() + 2);
        printf("a after erase 2:\n");
        for (auto e : a)
        {
            printf("  %i\n", e);
        }
        printf("hurray!\n");
    }
    
    auto error_message_unassigned_buffer_view(std::string_view task_name, std::string_view attachment_name) -> std::string
    {
        return std::format("Detected empty TaskBufferView in attachment \"{}\" view assignment in task \"{}\"!", attachment_name, task_name);
    }

    auto error_message_unassigned_image_view(std::string_view task_name, std::string_view attachment_name) -> std::string
    {
        return std::format("Detected empty TaskImageView in attachment \"{}\" view assignment in task \"{}\"!", attachment_name, task_name);
    }

    auto error_message_unassigned_tlas_view(std::string_view task_name, std::string_view attachment_name) -> std::string
    {
        return std::format("Detected empty TaskTlasView in attachment \"{}\" view assignment in task \"{}\"!", attachment_name, task_name);
    }

    auto error_message_unassigned_blas_view(std::string_view task_name, std::string_view attachment_name) -> std::string
    {
        return std::format("Detected empty TaskBlasView in attachment \"{}\" view assignment in task \"{}\"!", attachment_name, task_name);
    }

    auto flat_queue_index(daxa::Queue queue) -> u32
    {
        u32 offsets[3] = {
            0,
            1,
            1 + DAXA_MAX_COMPUTE_QUEUE_COUNT,
        };
        return offsets[static_cast<u32>(queue.family)] + queue.index;
    }

    auto flat_index_to_queue(u32 flat_index) -> daxa::Queue
    {
        daxa::Queue queues[] = {
            daxa::QUEUE_MAIN,
            daxa::QUEUE_COMPUTE_0,
            daxa::QUEUE_COMPUTE_1,
            daxa::QUEUE_COMPUTE_2,
            daxa::QUEUE_COMPUTE_3,
            daxa::QUEUE_TRANSFER_0,
            daxa::QUEUE_TRANSFER_1,
        };
        return queues[flat_index];
    }

    auto to_string(TaskStage stage) -> std::string_view
    {
        switch (stage)
        {
        case TaskStage::NONE: return "NONE";
        case TaskStage::VERTEX_SHADER: return "VERTEX_SHADER";
        case TaskStage::TESSELLATION_CONTROL_SHADER: return "TESSELLATION_CONTROL_SHADER";
        case TaskStage::TESSELLATION_EVALUATION_SHADER: return "TESSELLATION_EVALUATION_SHADER";
        case TaskStage::GEOMETRY_SHADER: return "GEOMETRY_SHADER";
        case TaskStage::FRAGMENT_SHADER: return "FRAGMENT_SHADER";
        case TaskStage::TASK_SHADER: return "TASK_SHADER";
        case TaskStage::MESH_SHADER: return "MESH_SHADER";
        case TaskStage::PRE_RASTERIZATION_SHADERS: return "PRE_RASTERIZATION_SHADERS";
        case TaskStage::RASTER_SHADER: return "RASTER_SHADER";
        case TaskStage::COMPUTE_SHADER: return "COMPUTE_SHADER";
        case TaskStage::RAY_TRACING_SHADER: return "RAY_TRACING_SHADER";
        case TaskStage::SHADER: return "SHADER";
        case TaskStage::COLOR_ATTACHMENT: return "COLOR_ATTACHMENT";
        case TaskStage::DEPTH_STENCIL_ATTACHMENT: return "DEPTH_STENCIL_ATTACHMENT";
        case TaskStage::RESOLVE: return "RESOLVE";
        case TaskStage::PRESENT: return "PRESENT";
        case TaskStage::INDIRECT_COMMAND: return "INDIRECT_COMMAND";
        case TaskStage::INDEX_INPUT: return "INDEX_INPUT";
        case TaskStage::TRANSFER: return "TRANSFER";
        case TaskStage::HOST: return "HOST";
        case TaskStage::AS_BUILD: return "AS_BUILD";
        case TaskStage::ANY_COMMAND: return "ANY_COMMAND";
        }
        return "UNKNOWN";
    }

    auto to_string(TaskType task_type) -> std::string_view
    {
        switch (task_type)
        {
        case TaskType::GENERAL: return "GENERAL";
        case TaskType::RASTER: return "RASTER";
        case TaskType::COMPUTE: return "COMPUTE";
        case TaskType::RAY_TRACING: return "RAY_TRACING";
        case TaskType::TRANSFER: return "TRANSFER";
        default: return "UNKNOWN";
        }
    }

    auto task_type_allowed_stages(TaskType task_type, TaskStage stage) -> bool
    {
        switch (task_type)
        {
        case TaskType::GENERAL:
            return true;
        case TaskType::RASTER:
            return stage == TaskStage::VERTEX_SHADER ||
                   stage == TaskStage::TESSELLATION_CONTROL_SHADER ||
                   stage == TaskStage::TESSELLATION_EVALUATION_SHADER ||
                   stage == TaskStage::GEOMETRY_SHADER ||
                   stage == TaskStage::FRAGMENT_SHADER ||
                   stage == TaskStage::TASK_SHADER ||
                   stage == TaskStage::MESH_SHADER ||
                   stage == TaskStage::PRE_RASTERIZATION_SHADERS ||
                   stage == TaskStage::RASTER_SHADER ||
                   stage == TaskStage::COLOR_ATTACHMENT ||
                   stage == TaskStage::DEPTH_STENCIL_ATTACHMENT ||
                   stage == TaskStage::RESOLVE ||
                   stage == TaskStage::PRESENT ||
                   stage == TaskStage::INDIRECT_COMMAND ||
                   stage == TaskStage::INDEX_INPUT;
        case TaskType::COMPUTE:
            return stage == TaskStage::COMPUTE_SHADER ||
                   stage == TaskStage::INDIRECT_COMMAND;
        case TaskType::RAY_TRACING:
            return stage == TaskStage::RAY_TRACING_SHADER ||
                   stage == TaskStage::INDIRECT_COMMAND ||
                   stage == TaskStage::AS_BUILD;
        case TaskType::TRANSFER:
            return stage == TaskStage::TRANSFER ||
                   stage == TaskStage::HOST ||
                   stage == TaskStage::AS_BUILD;
        default:
            return false;
        }
    }

    auto task_type_default_stage(TaskType task_type) -> TaskStage
    {
        switch (task_type)
        {
        case TaskType::GENERAL: return TaskStage::ANY_COMMAND;
        case TaskType::RASTER: return TaskStage::RASTER_SHADER;
        case TaskType::COMPUTE: return TaskStage::COMPUTE_SHADER;
        case TaskType::RAY_TRACING: return TaskStage::RAY_TRACING_SHADER;
        case TaskType::TRANSFER: return TaskStage::TRANSFER;
        default: return TaskStage::NONE;
        }
    }

    auto TaskInterface::get(TaskBufferAttachmentIndex index) const -> TaskBufferAttachmentInfo const &
    {
        return attachment_infos[index.value].value.buffer;
    }

    auto TaskInterface::get(TaskBufferView view) const -> TaskBufferAttachmentInfo const &
    {
        auto iter = std::find_if(attachment_infos.begin(), attachment_infos.end(), [&](auto const & other)
                                 { 
            if (other.type == TaskAttachmentType::BUFFER)
            {
                return other.value.buffer.view == view || other.value.buffer.translated_view == view;
            }
            return false; });
        DAXA_DBG_ASSERT_TRUE_M(iter != attachment_infos.end(), "Detected invalid task buffer view as index for attachment!");

        return iter->value.buffer;
    }

    auto TaskInterface::get(TaskBlasAttachmentIndex index) const -> TaskBlasAttachmentInfo const &
    {
        return attachment_infos[index.value].value.blas;
    }

    auto TaskInterface::get(TaskBlasView view) const -> TaskBlasAttachmentInfo const &
    {
        auto iter = std::find_if(attachment_infos.begin(), attachment_infos.end(), [&](auto const & other)
                                 { 
            if (other.type == TaskAttachmentType::BLAS)
            {
                return other.value.blas.view == view || other.value.blas.translated_view == view;
            }
            return false; });
        DAXA_DBG_ASSERT_TRUE_M(iter != attachment_infos.end(), "Detected invalid task blas view as index for attachment!");

        return iter->value.blas;
    }

    auto TaskInterface::get(TaskTlasAttachmentIndex index) const -> TaskTlasAttachmentInfo const &
    {
        return attachment_infos[index.value].value.tlas;
    }

    auto TaskInterface::get(TaskTlasView view) const -> TaskTlasAttachmentInfo const &
    {
        auto iter = std::find_if(attachment_infos.begin(), attachment_infos.end(), [&](auto const & other)
                                 { 
            if (other.type == TaskAttachmentType::TLAS)
            {
                return other.value.tlas.view == view || other.value.tlas.translated_view == view;
            }
            return false; });
        DAXA_DBG_ASSERT_TRUE_M(iter != attachment_infos.end(), "Detected invalid task tlas view as index for attachment!");

        return iter->value.tlas;
    }

    auto TaskInterface::get(TaskImageAttachmentIndex index) const -> TaskImageAttachmentInfo const &
    {
        return attachment_infos[index.value].value.image;
    }

    auto TaskInterface::get(TaskImageView view) const -> TaskImageAttachmentInfo const &
    {
        auto iter = std::find_if(attachment_infos.begin(), attachment_infos.end(), [&](auto const & other)
                                 { 
            if (other.type == TaskAttachmentType::IMAGE)
            {
                return other.value.image.view == view || other.value.image.translated_view == view;
            }
            return false; });
        DAXA_DBG_ASSERT_TRUE_M(iter != attachment_infos.end(), "Detected invalid task image view as index for attachment!");
        return iter->value.image;
    }

    auto TaskInterface::get(usize index) const -> TaskAttachmentInfo const &
    {
        return attachment_infos[index];
    }

    auto to_string(TaskGPUResourceView const & id) -> std::string
    {
        return std::format("tg idx: {}, index: {}", id.task_graph_index, id.index);
    }

    auto to_access_type(TaskAccessType taccess) -> AccessTypeFlags
    {
        AccessTypeFlags ret = {};
        switch (taccess)
        {
        case TaskAccessType::NONE: ret = AccessTypeFlagBits::NONE; break;
        case TaskAccessType::READ: ret = AccessTypeFlagBits::READ; break;
        case TaskAccessType::SAMPLED: ret = AccessTypeFlagBits::READ; break;
        case TaskAccessType::WRITE: ret = AccessTypeFlagBits::WRITE; break;
        case TaskAccessType::READ_WRITE: ret = AccessTypeFlagBits::READ_WRITE; break;
        case TaskAccessType::WRITE_CONCURRENT: ret = AccessTypeFlagBits::WRITE; break;
        case TaskAccessType::READ_WRITE_CONCURRENT: ret = AccessTypeFlagBits::READ_WRITE; break;
        default: DAXA_DBG_ASSERT_TRUE_M(false, "INVALID ACCESS TYPE"); break;
        }
        return ret;
    }

    auto to_pipeline_stage_flags(TaskStage stage) -> PipelineStageFlags
    {
        PipelineStageFlags ret = {};
        switch (stage)
        {
        case TaskStage::NONE: ret = (daxa::PipelineStageFlagBits::NONE); break;
        case TaskStage::VERTEX_SHADER: ret = (daxa::PipelineStageFlagBits::VERTEX_SHADER); break;
        case TaskStage::TESSELLATION_CONTROL_SHADER: ret = (daxa::PipelineStageFlagBits::TESSELLATION_CONTROL_SHADER); break;
        case TaskStage::TESSELLATION_EVALUATION_SHADER: ret = (daxa::PipelineStageFlagBits::TESSELLATION_EVALUATION_SHADER); break;
        case TaskStage::GEOMETRY_SHADER: ret = (daxa::PipelineStageFlagBits::GEOMETRY_SHADER); break;
        case TaskStage::FRAGMENT_SHADER: ret = (daxa::PipelineStageFlagBits::FRAGMENT_SHADER); break;
        case TaskStage::COMPUTE_SHADER: ret = (daxa::PipelineStageFlagBits::COMPUTE_SHADER); break;
        case TaskStage::RAY_TRACING_SHADER: ret = (daxa::PipelineStageFlagBits::RAY_TRACING_SHADER); break;
        case TaskStage::TASK_SHADER: ret = (daxa::PipelineStageFlagBits::TASK_SHADER); break;
        case TaskStage::MESH_SHADER: ret = (daxa::PipelineStageFlagBits::MESH_SHADER); break;
        case TaskStage::PRE_RASTERIZATION_SHADERS: ret = (daxa::PipelineStageFlagBits::PRE_RASTERIZATION_SHADERS); break;
        case TaskStage::RASTER_SHADER: ret = (daxa::PipelineStageFlagBits::ALL_GRAPHICS); break;
        case TaskStage::SHADER: ret = (daxa::PipelineStageFlagBits::ALL_GRAPHICS | daxa::PipelineStageFlagBits::RAY_TRACING_SHADER | daxa::PipelineStageFlagBits::COMPUTE_SHADER); break;
        case TaskStage::COLOR_ATTACHMENT: ret = (daxa::PipelineStageFlagBits::COLOR_ATTACHMENT_OUTPUT); break;
        case TaskStage::DEPTH_STENCIL_ATTACHMENT: ret = (daxa::PipelineStageFlagBits::LATE_FRAGMENT_TESTS) | (daxa::PipelineStageFlagBits::EARLY_FRAGMENT_TESTS) | (daxa::PipelineStageFlagBits::COLOR_ATTACHMENT_OUTPUT); break;
        case TaskStage::RESOLVE: ret = (daxa::PipelineStageFlagBits::RESOLVE); break;
        case TaskStage::PRESENT: ret = (daxa::PipelineStageFlagBits::ALL_COMMANDS); break;
        case TaskStage::INDIRECT_COMMAND: ret = (daxa::PipelineStageFlagBits::DRAW_INDIRECT); break;
        case TaskStage::INDEX_INPUT: ret = (daxa::PipelineStageFlagBits::INDEX_INPUT); break;
        case TaskStage::TRANSFER: ret = (daxa::PipelineStageFlagBits::TRANSFER); break;
        case TaskStage::HOST: ret = (daxa::PipelineStageFlagBits::HOST); break;
        case TaskStage::AS_BUILD: ret = (daxa::PipelineStageFlagBits::ACCELERATION_STRUCTURE_BUILD); break;
        case TaskStage::ANY_COMMAND: ret = (daxa::PipelineStageFlagBits::ALL_COMMANDS); break;
        }
        return ret;
    }

    auto is_task_stage_shader_access(TaskStage stage) -> bool
    {
        bool used_in_shader = false;
        switch (stage)
        {
        case TaskStage::NONE: used_in_shader = false; break;
        case TaskStage::VERTEX_SHADER: used_in_shader = true; break;
        case TaskStage::TESSELLATION_CONTROL_SHADER: used_in_shader = true; break;
        case TaskStage::TESSELLATION_EVALUATION_SHADER: used_in_shader = true; break;
        case TaskStage::GEOMETRY_SHADER: used_in_shader = true; break;
        case TaskStage::FRAGMENT_SHADER: used_in_shader = true; break;
        case TaskStage::COMPUTE_SHADER: used_in_shader = true; break;
        case TaskStage::RAY_TRACING_SHADER: used_in_shader = true; break;
        case TaskStage::TASK_SHADER: used_in_shader = true; break;
        case TaskStage::MESH_SHADER: used_in_shader = true; break;
        case TaskStage::PRE_RASTERIZATION_SHADERS: used_in_shader = true; break;
        case TaskStage::RASTER_SHADER: used_in_shader = true; break;
        case TaskStage::SHADER: used_in_shader = true; break;
        case TaskStage::COLOR_ATTACHMENT: used_in_shader = false; break;
        case TaskStage::DEPTH_STENCIL_ATTACHMENT: used_in_shader = false; break;
        case TaskStage::RESOLVE: used_in_shader = false; break;
        case TaskStage::PRESENT: used_in_shader = false; break;
        case TaskStage::INDIRECT_COMMAND: used_in_shader = false; break;
        case TaskStage::INDEX_INPUT: used_in_shader = false; break;
        case TaskStage::TRANSFER: used_in_shader = false; break;
        case TaskStage::HOST: used_in_shader = false; break;
        case TaskStage::AS_BUILD: used_in_shader = false; break;
        case TaskStage::ANY_COMMAND: used_in_shader = true; break;
        }
        return used_in_shader;
    }

    auto access_to_usage(TaskAccess const & taccess) -> ImageUsageFlags
    {
        bool const used_as_read = (static_cast<u32>(taccess.type) & static_cast<u32>(TaskAccessType::READ)) != 0;
        bool const used_as_write = (static_cast<u32>(taccess.type) & static_cast<u32>(TaskAccessType::WRITE)) != 0;
        bool const used_as_sampled = (static_cast<u32>(taccess.type) & static_cast<u32>(TaskAccessType::SAMPLED)) != 0;
        bool const used_in_shader = is_task_stage_shader_access(taccess.stage);
        bool const used_as_sampled_image = used_in_shader && used_as_sampled;
        bool const used_as_storage_image = used_in_shader && (used_as_read || used_as_write);
        bool const used_as_color_attachment = taccess.stage == TaskStage::COLOR_ATTACHMENT || taccess.stage == TaskStage::RESOLVE;
        bool const used_as_ds_attachment = taccess.stage == TaskStage::DEPTH_STENCIL_ATTACHMENT;
        bool const used_as_transfer = taccess.stage == TaskStage::TRANSFER;

        ImageUsageFlags usages = {};
        if (used_as_sampled_image)
        {
            usages = usages | ImageUsageFlagBits::SHADER_SAMPLED;
        }
        if (used_as_storage_image)
        {
            usages = usages | ImageUsageFlagBits::SHADER_STORAGE;
        }
        if (used_as_color_attachment)
        {
            usages = usages | ImageUsageFlagBits::COLOR_ATTACHMENT;
        }
        if (used_as_ds_attachment)
        {
            usages = usages | ImageUsageFlagBits::DEPTH_STENCIL_ATTACHMENT;
        }
        if (used_as_transfer && used_as_read)
        {
            usages = usages | ImageUsageFlagBits::TRANSFER_SRC;
        }
        if (used_as_transfer && used_as_write)
        {
            usages = usages | ImageUsageFlagBits::TRANSFER_DST;
        }
        if (taccess.stage == TaskStage::ANY_COMMAND)
        {
            usages =
                ImageUsageFlagBits::TRANSFER_SRC |
                ImageUsageFlagBits::TRANSFER_DST |
                ImageUsageFlagBits::SHADER_SAMPLED |
                ImageUsageFlagBits::SHADER_STORAGE |
                ImageUsageFlagBits::COLOR_ATTACHMENT |
                ImageUsageFlagBits::DEPTH_STENCIL_ATTACHMENT;
        }
        return usages;
    }

    auto static constexpr view_type_to_create_flags(ImageViewType const & view_type) -> ImageCreateFlags
    {
        // NOTE: Do we need to handle other flags, such as compatible array?
        if (view_type == ImageViewType::CUBE)
        {
            return ImageCreateFlagBits::COMPATIBLE_CUBE;
        }
        else
        {
            return ImageCreateFlagBits::NONE;
        }
    }

    auto task_image_access_to_layout_access(TaskAccess const & taccess) -> std::tuple<ImageLayout, Access, TaskAccessConcurrency>
    {
        auto const [access, concurrency] = task_access_to_access(taccess);
        // Kept for future reference:
        [[maybe_unused]] bool const used_in_shader = is_task_stage_shader_access(taccess.stage);
        [[maybe_unused]] bool const used_as_attachment = taccess.stage == TaskStage::COLOR_ATTACHMENT ||
                                        taccess.stage == TaskStage::DEPTH_STENCIL_ATTACHMENT ||
                                        taccess.stage == TaskStage::RESOLVE;

        ImageLayout layout = ImageLayout::GENERAL;
        
        if (taccess.stage == TaskStage::PRESENT)
        {
            layout = ImageLayout::PRESENT_SRC;
        }
        return {layout, access, concurrency};
    }

    auto task_access_to_access(TaskAccess const & taccess) -> std::pair<Access, TaskAccessConcurrency>
    {
        TaskAccessConcurrency const concurrent = (static_cast<u8>(taccess.type) & static_cast<u8>(TaskAccessType::CONCURRENT_BIT)) != 0 ? TaskAccessConcurrency::CONCURRENT : TaskAccessConcurrency::EXCLUSIVE;
        Access const access = Access{to_pipeline_stage_flags(taccess.stage), to_access_type(taccess.type)};
        return {access, concurrent};
    }

    auto TaskGPUResourceView::is_empty() const -> bool
    {
        return index == 0 && task_graph_index == 0;
    }

    auto TaskGPUResourceView::is_external() const -> bool
    {
        return task_graph_index == std::numeric_limits<u32>::max() && !is_null();
    }

    auto TaskGPUResourceView::is_null() const -> bool
    {
        return task_graph_index == std::numeric_limits<u32>::max() &&
               index == std::numeric_limits<u32>::max();
    }

    auto to_string(TaskAccess const & taccess) -> std::string_view
    {
        std::string_view ret = {};
#define _DAXA_GENERATE_STRING_CASE_TACCESS_TYPE(STAGE)                                                          \
    switch (taccess.type)                                                                                       \
    {                                                                                                           \
    case TaskAccessType::NONE: ret = std::string_view{#STAGE "_NONE"}; break;                                   \
    case TaskAccessType::READ: ret = std::string_view{#STAGE "_READ"}; break;                                   \
    case TaskAccessType::WRITE: ret = std::string_view{#STAGE "_WRITE"}; break;                                 \
    case TaskAccessType::SAMPLED: ret = std::string_view{#STAGE "_SAMPLED"}; break;                             \
    case TaskAccessType::READ_WRITE: ret = std::string_view{#STAGE "_READ_WRITE"}; break;                       \
    case TaskAccessType::WRITE_CONCURRENT: ret = std::string_view{#STAGE "_WRITE_CONCURRENT"}; break;           \
    case TaskAccessType::READ_WRITE_CONCURRENT: ret = std::string_view{#STAGE "_READ_WRITE_CONCURRENT"}; break; \
    default: DAXA_DBG_ASSERT_TRUE_M(false, "INVALID ACCESS TYPE"); break;                                       \
    }

        switch (taccess.stage)
        {
        case TaskStage::NONE:
            _DAXA_GENERATE_STRING_CASE_TACCESS_TYPE(NONE)
            break;
        case TaskStage::VERTEX_SHADER:
            _DAXA_GENERATE_STRING_CASE_TACCESS_TYPE(VERTEX_SHADER)
            break;
        case TaskStage::TESSELLATION_CONTROL_SHADER:
            _DAXA_GENERATE_STRING_CASE_TACCESS_TYPE(TESSELLATION_CONTROL_SHADER)
            break;
        case TaskStage::TESSELLATION_EVALUATION_SHADER:
            _DAXA_GENERATE_STRING_CASE_TACCESS_TYPE(TESSELLATION_EVALUATION_SHADER)
            break;
        case TaskStage::GEOMETRY_SHADER:
            _DAXA_GENERATE_STRING_CASE_TACCESS_TYPE(GEOMETRY_SHADER)
            break;
        case TaskStage::FRAGMENT_SHADER:
            _DAXA_GENERATE_STRING_CASE_TACCESS_TYPE(FRAGMENT_SHADER)
            break;
        case TaskStage::COMPUTE_SHADER:
            _DAXA_GENERATE_STRING_CASE_TACCESS_TYPE(COMPUTE_SHADER)
            break;
        case TaskStage::RAY_TRACING_SHADER:
            _DAXA_GENERATE_STRING_CASE_TACCESS_TYPE(RAY_TRACING_SHADER)
            break;
        case TaskStage::TASK_SHADER:
            _DAXA_GENERATE_STRING_CASE_TACCESS_TYPE(TASK_SHADER)
            break;
        case TaskStage::MESH_SHADER:
            _DAXA_GENERATE_STRING_CASE_TACCESS_TYPE(MESH_SHADER)
            break;
        case TaskStage::PRE_RASTERIZATION_SHADERS:
            _DAXA_GENERATE_STRING_CASE_TACCESS_TYPE(PRE_RASTERIZATION_SHADERS)
            break;
        case TaskStage::RASTER_SHADER:
            _DAXA_GENERATE_STRING_CASE_TACCESS_TYPE(RASTER_SHADER)
            break;
        case TaskStage::SHADER:
            _DAXA_GENERATE_STRING_CASE_TACCESS_TYPE(SHADER)
            break;
        case TaskStage::COLOR_ATTACHMENT:
            _DAXA_GENERATE_STRING_CASE_TACCESS_TYPE(COLOR_ATTACHMENT)
            break;
        case TaskStage::DEPTH_STENCIL_ATTACHMENT:
            _DAXA_GENERATE_STRING_CASE_TACCESS_TYPE(DEPTH_STENCIL_ATTACHMENT)
            break;
        case TaskStage::RESOLVE:
            _DAXA_GENERATE_STRING_CASE_TACCESS_TYPE(RESOLVE)
            break;
        case TaskStage::PRESENT:
            _DAXA_GENERATE_STRING_CASE_TACCESS_TYPE(PRESENT)
            break;
        case TaskStage::INDIRECT_COMMAND:
            _DAXA_GENERATE_STRING_CASE_TACCESS_TYPE(INDIRECT_COMMAND)
            break;
        case TaskStage::INDEX_INPUT:
            _DAXA_GENERATE_STRING_CASE_TACCESS_TYPE(INDEX_INPUT)
            break;
        case TaskStage::TRANSFER:
            _DAXA_GENERATE_STRING_CASE_TACCESS_TYPE(TRANSFER)
            break;
        case TaskStage::HOST:
            _DAXA_GENERATE_STRING_CASE_TACCESS_TYPE(HOST)
            break;
        case TaskStage::AS_BUILD:
            _DAXA_GENERATE_STRING_CASE_TACCESS_TYPE(AS_BUILD)
            break;
        case TaskStage::ANY_COMMAND:
            _DAXA_GENERATE_STRING_CASE_TACCESS_TYPE(ANY_COMMAND)
            break;
        }
        return ret;
    }

    ImplPersistentTaskBufferBlasTlas::ImplPersistentTaskBufferBlasTlas(TaskBufferInfo a_info)
        : actual_ids{std::vector<BufferId>{a_info.initial_buffers.buffers.begin(), a_info.initial_buffers.buffers.end()}},
          latest_access{a_info.initial_buffers.latest_access},
          info{std::move(a_info)},
          unique_index{ImplPersistentTaskBufferBlasTlas::exec_unique_next_index++}
    {
        DAXA_DBG_ASSERT_TRUE_M(a_info.initial_buffers.buffers.size() <= 1, "TaskGraph MK2 does not support multiple runtime resources per task resource");
    }

    ImplPersistentTaskBufferBlasTlas::ImplPersistentTaskBufferBlasTlas(Device & device, BufferInfo const & a_info)
        : actual_ids{std::vector<BufferId>{device.create_buffer(a_info)}},
          latest_access{},
          info{TaskBufferInfo{.name = a_info.name.c_str().data()}},
          owned_buffer_device{device},
          owned_buffer_info{a_info},
          unique_index{ImplPersistentTaskBufferBlasTlas::exec_unique_next_index++}
    {
    }

    ImplPersistentTaskBufferBlasTlas::ImplPersistentTaskBufferBlasTlas(TaskBlasInfo a_info)
        : actual_ids{std::vector<BlasId>{a_info.initial_blas.blas.begin(), a_info.initial_blas.blas.end()}},
          latest_access{a_info.initial_blas.latest_access},
          info{std::move(a_info)},
          unique_index{ImplPersistentTaskBufferBlasTlas::exec_unique_next_index++}
    {
        DAXA_DBG_ASSERT_TRUE_M(a_info.initial_blas.blas.size() <= 1, "TaskGraph MK2 does not support multiple runtime resources per task resource");
    }

    ImplPersistentTaskBufferBlasTlas::ImplPersistentTaskBufferBlasTlas(TaskTlasInfo a_info)
        : actual_ids{std::vector<TlasId>{a_info.initial_tlas.tlas.begin(), a_info.initial_tlas.tlas.end()}},
          latest_access{a_info.initial_tlas.latest_access},
          info{std::move(a_info)},
          unique_index{ImplPersistentTaskBufferBlasTlas::exec_unique_next_index++}
    {
        DAXA_DBG_ASSERT_TRUE_M(a_info.initial_tlas.tlas.size() <= 1, "TaskGraph MK2 does not support multiple runtime resources per task resource");
    }
    ImplPersistentTaskBufferBlasTlas::~ImplPersistentTaskBufferBlasTlas() = default;

    void ImplPersistentTaskBufferBlasTlas::zero_ref_callback(ImplHandle const * handle)
    {
        auto * self = rc_cast<ImplPersistentTaskBufferBlasTlas *>(handle);
        if (self->owned_buffer_info.has_value())
        {
            BufferId const first_id = std::get<std::vector<BufferId>>(self->actual_ids)[0];
            self->owned_buffer_device.value().destroy_buffer(first_id);
        }
        delete self;
    }

    // --- TaskBuffer ---

    TaskBuffer::TaskBuffer(TaskBufferInfo const & info)
    {
        this->object = new ImplPersistentTaskBufferBlasTlas(info);
    }

    TaskBuffer::TaskBuffer(daxa::Device & device, BufferInfo const & info)
    {
        this->object = new ImplPersistentTaskBufferBlasTlas(device, info);
    }

    auto TaskBuffer::view() const -> TaskBufferView
    {
        auto & impl = *r_cast<ImplPersistentTaskBufferBlasTlas *>(this->object);
        return TaskBufferView{{.task_graph_index = std::numeric_limits<u32>::max(), .index = impl.unique_index}};
    }

    TaskBuffer::operator TaskBufferView() const
    {
        return view();
    }

    auto TaskBuffer::info() const -> TaskBufferInfo const &
    {
        auto & impl = *r_cast<ImplPersistentTaskBufferBlasTlas *>(this->object);
        return std::get<TaskBufferInfo>(impl.info);
    }

    auto TaskBuffer::get_state() const -> TrackedBuffers
    {
        auto const & impl = *r_cast<ImplPersistentTaskBufferBlasTlas const *>(this->object);
        return TrackedBuffers{
            .buffers = {std::get<std::vector<BufferId>>(impl.actual_ids).data(), std::get<std::vector<BufferId>>(impl.actual_ids).size()},
            .latest_access = impl.latest_access,
        };
    }

    auto TaskBuffer::is_owning() const -> bool
    {
        auto const & impl = *r_cast<ImplPersistentTaskBufferBlasTlas const *>(this->object);
        return impl.owned_buffer_device.has_value();
    }

    void TaskBuffer::set_buffers(TrackedBuffers const & buffers)
    {
        auto & impl = *r_cast<ImplPersistentTaskBufferBlasTlas *>(this->object);
        auto & actual_buffers = std::get<std::vector<BufferId>>(impl.actual_ids);
        actual_buffers.clear();
        actual_buffers.insert(actual_buffers.end(), buffers.buffers.begin(), buffers.buffers.end());
        impl.latest_access = buffers.latest_access;
    }

    void TaskBuffer::swap_buffers(TaskBuffer & other)
    {
        auto & impl = *r_cast<ImplPersistentTaskBufferBlasTlas *>(this->object);
        auto & actual_buffers = std::get<std::vector<BufferId>>(impl.actual_ids);
        auto & impl_other = *r_cast<ImplPersistentTaskBufferBlasTlas *>(other.object);
        auto & other_actual_buffers = std::get<std::vector<BufferId>>(impl_other.actual_ids);
        std::swap(actual_buffers, other_actual_buffers);
        std::swap(impl.latest_access, impl_other.latest_access);
    }

    auto TaskBuffer::inc_refcnt(ImplHandle const * object) -> u64
    {
        return object->inc_refcnt();
    }

    auto TaskBuffer::dec_refcnt(ImplHandle const * object) -> u64
    {
        return object->dec_refcnt(
            ImplPersistentTaskBufferBlasTlas::zero_ref_callback,
            nullptr);
    }

    // --- TaskBuffer End ---

    // --- TaskBlas ---

    TaskBlas::TaskBlas(TaskBlasInfo const & info)
    {
        this->object = new ImplPersistentTaskBufferBlasTlas(info);
    }

    auto TaskBlas::view() const -> TaskBlasView
    {
        auto & impl = *r_cast<ImplPersistentTaskBufferBlasTlas *>(this->object);
        return TaskBlasView{{.task_graph_index = std::numeric_limits<u32>::max(), .index = impl.unique_index}};
    }

    TaskBlas::operator TaskBlasView() const
    {
        return view();
    }

    auto TaskBlas::info() const -> TaskBlasInfo const &
    {
        auto & impl = *r_cast<ImplPersistentTaskBufferBlasTlas *>(this->object);
        return std::get<TaskBlasInfo>(impl.info);
    }

    auto TaskBlas::get_state() const -> TrackedBlas
    {
        auto const & impl = *r_cast<ImplPersistentTaskBufferBlasTlas const *>(this->object);
        return TrackedBlas{
            .blas = {std::get<std::vector<BlasId>>(impl.actual_ids).data(), std::get<std::vector<BlasId>>(impl.actual_ids).size()},
            .latest_access = impl.latest_access,
        };
    }

    void TaskBlas::set_blas(TrackedBlas const & other_tracked)
    {
        auto & impl = *r_cast<ImplPersistentTaskBufferBlasTlas *>(this->object);
        auto & actual_ids = std::get<std::vector<BlasId>>(impl.actual_ids);
        actual_ids.clear();
        actual_ids.insert(actual_ids.end(), other_tracked.blas.begin(), other_tracked.blas.end());
        impl.latest_access = other_tracked.latest_access;
    }

    void TaskBlas::swap_blas(TaskBlas & other)
    {
        auto & impl = *r_cast<ImplPersistentTaskBufferBlasTlas *>(this->object);
        auto & actual_buffers = std::get<std::vector<BlasId>>(impl.actual_ids);
        auto & impl_other = *r_cast<ImplPersistentTaskBufferBlasTlas *>(other.object);
        auto & other_actual_buffers = std::get<std::vector<BlasId>>(impl_other.actual_ids);
        std::swap(actual_buffers, other_actual_buffers);
        std::swap(impl.latest_access, impl_other.latest_access);
    }

    auto TaskBlas::inc_refcnt(ImplHandle const * object) -> u64
    {
        return object->inc_refcnt();
    }

    auto TaskBlas::dec_refcnt(ImplHandle const * object) -> u64
    {
        return object->dec_refcnt(
            ImplPersistentTaskBufferBlasTlas::zero_ref_callback,
            nullptr);
    }

    // --- TaskBlas End ---

    // --- TaskTlas ---

    TaskTlas::TaskTlas(TaskTlasInfo const & info)
    {
        this->object = new ImplPersistentTaskBufferBlasTlas(info);
    }

    auto TaskTlas::view() const -> TaskTlasView
    {
        auto & impl = *r_cast<ImplPersistentTaskBufferBlasTlas *>(this->object);
        return TaskTlasView{{.task_graph_index = std::numeric_limits<u32>::max(), .index = impl.unique_index}};
    }

    TaskTlas::operator TaskTlasView() const
    {
        return view();
    }

    auto TaskTlas::info() const -> TaskTlasInfo const &
    {
        auto & impl = *r_cast<ImplPersistentTaskBufferBlasTlas *>(this->object);
        return std::get<TaskTlasInfo>(impl.info);
    }

    auto TaskTlas::get_state() const -> TrackedTlas
    {
        auto const & impl = *r_cast<ImplPersistentTaskBufferBlasTlas const *>(this->object);
        return TrackedTlas{
            .tlas = {std::get<std::vector<TlasId>>(impl.actual_ids).data(), std::get<std::vector<TlasId>>(impl.actual_ids).size()},
            .latest_access = impl.latest_access,
        };
    }

    void TaskTlas::set_tlas(TrackedTlas const & other_tracked)
    {
        auto & impl = *r_cast<ImplPersistentTaskBufferBlasTlas *>(this->object);
        auto & actual_ids = std::get<std::vector<TlasId>>(impl.actual_ids);
        actual_ids.clear();
        actual_ids.insert(actual_ids.end(), other_tracked.tlas.begin(), other_tracked.tlas.end());
        impl.latest_access = other_tracked.latest_access;
    }

    void TaskTlas::swap_tlas(TaskTlas & other)
    {
        auto & impl = *r_cast<ImplPersistentTaskBufferBlasTlas *>(this->object);
        auto & actual_buffers = std::get<std::vector<TlasId>>(impl.actual_ids);
        auto & impl_other = *r_cast<ImplPersistentTaskBufferBlasTlas *>(other.object);
        auto & other_actual_ids = std::get<std::vector<TlasId>>(impl_other.actual_ids);
        std::swap(actual_buffers, other_actual_ids);
        std::swap(impl.latest_access, impl_other.latest_access);
    }

    auto TaskTlas::inc_refcnt(ImplHandle const * object) -> u64
    {
        return object->inc_refcnt();
    }

    auto TaskTlas::dec_refcnt(ImplHandle const * object) -> u64
    {
        return object->dec_refcnt(
            ImplPersistentTaskBufferBlasTlas::zero_ref_callback,
            nullptr);
    }

    // --- TaskTlas End ---

    TaskImage::TaskImage(TaskImageInfo const & a_info)
    {
        this->object = new ImplPersistentTaskImage(a_info);
    }

    ImplPersistentTaskImage::ImplPersistentTaskImage(TaskImageInfo const & a_info)
        : info{a_info},
          actual_images{a_info.initial_images.images.begin(), a_info.initial_images.images.end()},
          latest_slice_states{a_info.initial_images.latest_slice_states.begin(), a_info.initial_images.latest_slice_states.end()},
          unique_index{ImplPersistentTaskImage::exec_unique_next_index++}
    {
        DAXA_DBG_ASSERT_TRUE_M(a_info.initial_images.images.size() <= 1, "TaskGraph MK2 does not support multiple runtime resources per task resource");
    }

    ImplPersistentTaskImage::~ImplPersistentTaskImage() = default;

    void ImplPersistentTaskImage::zero_ref_callback(ImplHandle const * handle)
    {
        auto const * self = r_cast<ImplPersistentTaskImage const *>(handle);
        delete self;
    }

    TaskImage::operator TaskImageView() const
    {
        return view();
    }

    auto TaskImage::view() const -> TaskImageView
    {
        auto & impl = *r_cast<ImplPersistentTaskImage *>(this->object);
        return TaskImageView{.task_graph_index = std::numeric_limits<u32>::max(), .index = impl.unique_index};
    }

    auto TaskImage::info() const -> TaskImageInfo const &
    {
        auto & impl = *r_cast<ImplPersistentTaskImage *>(this->object);
        return impl.info;
    }

    auto TaskImage::get_state() const -> TrackedImages
    {
        auto const & impl = *r_cast<ImplPersistentTaskImage const *>(this->object);
        return TrackedImages{
            .images = {impl.actual_images.data(), impl.actual_images.size()},
            .latest_slice_states = {impl.latest_slice_states.data(), impl.latest_slice_states.size()},
        };
    }

    void TaskImage::set_images(TrackedImages const & images)
    {
        auto & impl = *r_cast<ImplPersistentTaskImage *>(this->object);
        DAXA_DBG_ASSERT_TRUE_M(!impl.info.swapchain_image || (images.images.size() == 1), "swapchain task image can only have at most one runtime image");
        impl.actual_images.clear();
        impl.actual_images.insert(impl.actual_images.end(), images.images.begin(), images.images.end());
        impl.latest_slice_states.clear();
        impl.latest_slice_states.insert(impl.latest_slice_states.end(), images.latest_slice_states.begin(), images.latest_slice_states.end());
        impl.waited_on_acquire = {};
    }

    void TaskImage::swap_images(TaskImage & other)
    {
        auto & impl = *r_cast<ImplPersistentTaskImage *>(this->object);
        auto & impl_other = *r_cast<ImplPersistentTaskImage *>(other.object);
        DAXA_DBG_ASSERT_TRUE_M(!impl.info.swapchain_image || (impl_other.actual_images.size() <= 1), "swapchain task image can only have at most one runtime image");
        std::swap(impl.actual_images, impl_other.actual_images);
        std::swap(impl.latest_slice_states, impl_other.latest_slice_states);
        std::swap(impl.waited_on_acquire, impl_other.waited_on_acquire);
    }

    auto TaskImage::inc_refcnt(ImplHandle const * object) -> u64
    {
        return object->inc_refcnt();
    }

    auto TaskImage::dec_refcnt(ImplHandle const * object) -> u64
    {
        return object->dec_refcnt(
            ImplPersistentTaskImage::zero_ref_callback,
            nullptr);
    }

    enum PipeStage : u64
    {
        S_NONE = 0x00000000ull,
        S_TOP_OF_PIPE = 0x00000001ull,
        S_DRAW_INDIRECT = 0x00000002ull,
        S_VERTEX_SHADER = 0x00000008ull,
        S_TESSELLATION_CONTROL_SHADER = 0x00000010ull,
        S_TESSELLATION_EVALUATION_SHADER = 0x00000020ull,
        S_GEOMETRY_SHADER = 0x00000040ull,
        S_FRAGMENT_SHADER = 0x00000080ull,
        S_EARLY_FRAGMENT_TESTS = 0x00000100ull,
        S_LATE_FRAGMENT_TESTS = 0x00000200ull,
        S_COLOR_ATTACHMENT_OUTPUT = 0x00000400ull,
        S_COMPUTE_SHADER = 0x00000800ull,
        S_TRANSFER = 0x00001000ull,
        S_BOTTOM_OF_PIPE = 0x00002000ull,
        S_HOST = 0x00004000ull,
        S_ALL_GRAPHICS = 0x00008000ull,
        S_ALL_COMMANDS = 0x00010000ull,
        S_COPY = 0x100000000ull,
        S_RESOLVE = 0x200000000ull,
        S_BLIT = 0x400000000ull,
        S_CLEAR = 0x800000000ull,
        S_INDEX_INPUT = 0x1000000000ull,
        S_PRE_RASTERIZATION_SHADERS = 0x4000000000ull,
        S_TASK_SHADER = 0x00080000ull,
        S_MESH_SHADER = 0x00100000ull,
        S_ACCELERATION_STRUCTURE_BUILD = 0x02000000ull,
        S_RAY_TRACING_SHADER = 0x00200000ull,
    };

    enum PipeType : u64
    {
        T_NONE = 0x00000000,
        T_READ = 0x00008000,
        T_WRITE = 0x00010000,
    };

    TaskGraph::TaskGraph(TaskGraphInfo const & info)
    {
        this->object = new ImplTaskGraph(info);
        auto & impl = *r_cast<ImplTaskGraph *>(this->object);
        impl.barriers = {&impl.task_memory};
        impl.buffer_infos = {&impl.task_memory}; 
        impl.image_infos = {&impl.task_memory};
        impl.initial_barriers = {&impl.task_memory};
        auto queue_submit_scopes = std::array{
            QueueSubmitScope{.last_minute_barrier_indices = {&impl.task_memory}, .task_batches = {&impl.task_memory}, .used_swapchain_task_images = {&impl.task_memory}},
            QueueSubmitScope{.last_minute_barrier_indices = {&impl.task_memory}, .task_batches = {&impl.task_memory}, .used_swapchain_task_images = {&impl.task_memory}},
            QueueSubmitScope{.last_minute_barrier_indices = {&impl.task_memory}, .task_batches = {&impl.task_memory}, .used_swapchain_task_images = {&impl.task_memory}},
            QueueSubmitScope{.last_minute_barrier_indices = {&impl.task_memory}, .task_batches = {&impl.task_memory}, .used_swapchain_task_images = {&impl.task_memory}},
            QueueSubmitScope{.last_minute_barrier_indices = {&impl.task_memory}, .task_batches = {&impl.task_memory}, .used_swapchain_task_images = {&impl.task_memory}},
            QueueSubmitScope{.last_minute_barrier_indices = {&impl.task_memory}, .task_batches = {&impl.task_memory}, .used_swapchain_task_images = {&impl.task_memory}},
            QueueSubmitScope{.last_minute_barrier_indices = {&impl.task_memory}, .task_batches = {&impl.task_memory}, .used_swapchain_task_images = {&impl.task_memory}},
        };
        impl.batch_submit_scopes.push_back(TaskBatchSubmitScope{
            .queue_submit_scopes = queue_submit_scopes,
        });
        impl.setup_task_barriers = {&impl.task_memory};
    }
    TaskGraph::~TaskGraph() = default;

    void TaskGraph::use_persistent_buffer(TaskBuffer const & buffer)
    {
        auto & impl = *reinterpret_cast<ImplTaskGraph *>(this->object);
        DAXA_DBG_ASSERT_TRUE_M(!impl.compiled, "completed task graphs can not record new tasks");
        DAXA_DBG_ASSERT_TRUE_M(!impl.buffer_name_to_id.contains(buffer.info().name), "task buffer names must be unique");
        TaskBufferView const task_buffer_id{{.task_graph_index = impl.unique_index, .index = static_cast<u32>(impl.global_buffer_infos.size())}};

        // Allocate and copy name.
        auto name = impl.task_memory.allocate_copy_string(buffer.info().name);

        impl.buffer_infos.push_back(PerPermTaskBuffer{
            .valid = true,
        });

        impl.global_buffer_infos.emplace_back(PermIndepTaskBufferInfo{
            .task_buffer_data = PermIndepTaskBufferInfo::External{
                .buffer_blas_tlas = buffer,
            },
            .name = name,
        });
        impl.persistent_buffer_index_to_local_index[buffer.view().index] = task_buffer_id.index;

        impl.buffer_name_to_id[name] = task_buffer_id;
    }

    void TaskGraph::use_persistent_blas(TaskBlas const & blas)
    {
        auto & impl = *reinterpret_cast<ImplTaskGraph *>(this->object);
        DAXA_DBG_ASSERT_TRUE_M(!impl.compiled, "completed task graphs can not record new tasks");
        DAXA_DBG_ASSERT_TRUE_M(!impl.blas_name_to_id.contains(blas.info().name), "task blas names must be unique");
        TaskBlasView const task_blas_id{{.task_graph_index = impl.unique_index, .index = static_cast<u32>(impl.global_buffer_infos.size())}};

        // Allocate and copy name.
        auto name = impl.task_memory.allocate_copy_string(blas.info().name);

        impl.buffer_infos.push_back(PerPermTaskBuffer{
            .valid = true,
        });

        impl.global_buffer_infos.emplace_back(PermIndepTaskBufferInfo{
            .task_buffer_data = PermIndepTaskBufferInfo::External{
                .buffer_blas_tlas = blas,
            },
            .name = name,
        });
        impl.persistent_buffer_index_to_local_index[blas.view().index] = task_blas_id.index;
        impl.blas_name_to_id[name] = task_blas_id;
    }

    void TaskGraph::use_persistent_tlas(TaskTlas const & tlas)
    {
        auto & impl = *reinterpret_cast<ImplTaskGraph *>(this->object);
        DAXA_DBG_ASSERT_TRUE_M(!impl.compiled, "completed task graphs can not record new tasks");
        DAXA_DBG_ASSERT_TRUE_M(!impl.tlas_name_to_id.contains(tlas.info().name), "task tlas names must be unique");
        TaskTlasView const task_tlas_id{{.task_graph_index = impl.unique_index, .index = static_cast<u32>(impl.global_buffer_infos.size())}};

        // Allocate and copy name.
        auto name = impl.task_memory.allocate_copy_string(tlas.info().name);

        impl.buffer_infos.push_back(PerPermTaskBuffer{
            .valid = true,
        });

        impl.global_buffer_infos.emplace_back(PermIndepTaskBufferInfo{
            .task_buffer_data = PermIndepTaskBufferInfo::External{
                .buffer_blas_tlas = tlas,
            },
            .name = name,
        });
        impl.persistent_buffer_index_to_local_index[tlas.view().index] = task_tlas_id.index;
        impl.tlas_name_to_id[name] = task_tlas_id;
    }

    void TaskGraph::use_persistent_image(TaskImage const & image)
    {
        auto & impl = *reinterpret_cast<ImplTaskGraph *>(this->object);
        DAXA_DBG_ASSERT_TRUE_M(!impl.compiled, "completed task graphs can not record new tasks");
        DAXA_DBG_ASSERT_TRUE_M(!impl.image_name_to_id.contains(image.info().name), "task image names must be unique");
        TaskImageView const task_image_id{.task_graph_index = impl.unique_index, .index = static_cast<u32>(impl.global_image_infos.size())};

        // For non-persistent resources task graph will synch on the initial to first use every execution.
        impl.image_infos.push_back({
            .last_slice_states = {&impl.task_memory},
            .first_slice_states = {&impl.task_memory},
        });
        if (image.info().swapchain_image)
        {
            DAXA_DBG_ASSERT_TRUE_M(impl.swapchain_image.is_empty(), "can only register one swapchain image per task graph permutation");
            impl.swapchain_image = task_image_id;
        }

        impl.global_image_infos.emplace_back(PermIndepTaskImageInfo{
            .task_image_data = PermIndepTaskImageInfo::External{
                .image = image,
            },
            .name = impl.task_memory.allocate_copy_string(image.info().name),
        });
        impl.persistent_image_index_to_local_index[image.view().index] = task_image_id.index;
        impl.image_name_to_id[image.info().name] = task_image_id;
    }

    auto create_buffer_helper(ImplTaskGraph & impl, ImplTaskBufferKind kind, usize size, std::string_view name)
    {
        DAXA_DBG_ASSERT_TRUE_M(!impl.compiled, "completed task graphs can not record new tasks");
        DAXA_DBG_ASSERT_TRUE_M(!impl.buffer_name_to_id.contains(name), "task buffer names must be unique");

        ImplTaskBuffer buffer = {};
        buffer.kind = kind;
        buffer.external_array_index = INVALID_EXTERNAL_ARRAY_INDEX;
        buffer.id = {};
        buffer.size = size;
        buffer.name = impl.task_memory.allocate_copy_string(name);

        impl.buffers.push_back(buffer);
        u32 const index = static_cast<u32>(impl.buffers.size()) - 1u;

        impl.buffer_name_to_index[buffer.name] = index;

        return index;
    }

    auto TaskGraph::create_transient_buffer(TaskTransientBufferInfo info) -> TaskBufferView
    {
        auto & impl = *reinterpret_cast<ImplTaskGraph *>(this->object);
        return TaskBufferView{ { .task_graph_index = impl.unique_index, .index = create_buffer_helper( impl, ImplTaskBufferKind::BUFFER, info.size, info.name ) } };
    }

    auto TaskGraph::create_transient_tlas(TaskTransientTlasInfo info) -> TaskTlasView
    {
        auto & impl = *reinterpret_cast<ImplTaskGraph *>(this->object);
        return TaskTlasView{ { .task_graph_index = impl.unique_index, .index = create_buffer_helper( impl, ImplTaskBufferKind::TLAS, info.size, info.name ) } };
    }

    auto TaskGraph::create_transient_image(TaskTransientImageInfo info) -> TaskImageView
    {
        auto & impl = *reinterpret_cast<ImplTaskGraph *>(this->object);
        DAXA_DBG_ASSERT_TRUE_M(!impl.compiled, "completed task graphs can not record new tasks");
        DAXA_DBG_ASSERT_TRUE_M(!impl.image_name_to_id.contains(info.name), "task image names must be unique");

        ImplTaskImage image = {};
        image.external_array_index = INVALID_EXTERNAL_ARRAY_INDEX;
        image.id = {};
        image.dimensions = info.dimensions;
        image.format = info.format;
        image.size = info.size;
        image.mip_level_count = info.mip_level_count;
        image.array_layer_count = info.array_layer_count;
        image.sample_count = info.sample_count;
        image.name = impl.task_memory.allocate_copy_string(info.name);

        impl.images.push_back(image);
        u32 const index = static_cast<u32>(impl.images.size()) - 1u;

        impl.image_name_to_index[image.name] = index;

        TaskImageView task_image_view = {
            .task_graph_index = impl.unique_index,
            .index = static_cast<u32>(impl.global_image_infos.size()),
            .slice = {
                info.mip_level_count,
                info.array_layer_count,
            },
        };

        return task_image_view;
    }

    DAXA_EXPORT_CXX auto TaskGraph::transient_buffer_info(TaskBufferView const & transient) -> TaskTransientBufferInfo const &
    {
        ImplTaskGraph & impl = *reinterpret_cast<ImplTaskGraph *>(this->object);
        DAXA_DBG_ASSERT_TRUE_M(!transient.is_external(), "given view must be transient");
        DAXA_DBG_ASSERT_TRUE_M(transient.task_graph_index == impl.unique_index, "given view must be created by the given task graph");
        DAXA_DBG_ASSERT_TRUE_M(transient.index < impl.global_buffer_infos.size(), "given view has invalid index");

        return daxa::get<PermIndepTaskBufferInfo::Owned>(impl.global_buffer_infos.at(transient.index).task_buffer_data).info;
    }

    DAXA_EXPORT_CXX auto TaskGraph::transient_tlas_info(TaskTlasView const & transient) -> TaskTransientTlasInfo const &
    {
        ImplTaskGraph & impl = *reinterpret_cast<ImplTaskGraph *>(this->object);
        DAXA_DBG_ASSERT_TRUE_M(!transient.is_external(), "given view must be transient");
        DAXA_DBG_ASSERT_TRUE_M(transient.task_graph_index == impl.unique_index, "given view must be created by the given task graph");
        DAXA_DBG_ASSERT_TRUE_M(transient.index < impl.global_buffer_infos.size(), "given view has invalid index");

        return daxa::get<PermIndepTaskBufferInfo::Owned>(impl.global_buffer_infos.at(transient.index).task_buffer_data).info;
    }

    DAXA_EXPORT_CXX auto TaskGraph::transient_image_info(TaskImageView const & transient) -> TaskTransientImageInfo const &
    {
        ImplTaskGraph & impl = *reinterpret_cast<ImplTaskGraph *>(this->object);
        DAXA_DBG_ASSERT_TRUE_M(!transient.is_external(), "given view must be transient");
        DAXA_DBG_ASSERT_TRUE_M(transient.task_graph_index == impl.unique_index, "given view must be created by the given task graph");
        DAXA_DBG_ASSERT_TRUE_M(transient.index < impl.global_image_infos.size(), "given view has invalid index");

        return daxa::get<PermIndepTaskImageInfo::Owned>(impl.global_image_infos.at(transient.index).task_image_data).info;
    }

    DAXA_EXPORT_CXX void TaskGraph::clear_buffer(TaskBufferClearInfo const & info)
    {
        ImplTaskGraph & impl = *reinterpret_cast<ImplTaskGraph *>(this->object);

        auto const view = impl.buffer_blas_tlas_id_to_local_id(info.buffer);

        auto name = info.name.size() > 0 ? std::string(info.name) : std::string("clear buffer: ") + std::string(impl.global_buffer_infos.at(view.index).name);

        TaskBufferClearInfo cinfo = info;
        add_task(InlineTask::Transfer(name).writes(view).uses_queue(info.queue).executes([=](TaskInterface ti)
                                                                                         {
            for (u32 runtime_buf = 0; runtime_buf < ti.get(TaskBufferAttachmentIndex{0}).ids.size(); ++runtime_buf)
            {
                auto actual_size = ti.info(TaskBufferAttachmentIndex{0}, runtime_buf).value().size;
                DAXA_DBG_ASSERT_TRUE_M(cinfo.offset < actual_size, "clear buffer offset must be smaller then actual buffer size");
                auto size = std::min(actual_size, cinfo.size) - cinfo.offset;
                ti.recorder.clear_buffer(BufferClearInfo{
                    .buffer = ti.get(TaskBufferAttachmentIndex{0}).ids[runtime_buf],
                    .offset = cinfo.offset,
                    .size = size,
                    .clear_value = cinfo.clear_value,
                });
            } }));
    }

    DAXA_EXPORT_CXX void TaskGraph::clear_image(TaskImageClearInfo const & info)
    {
        ImplTaskGraph & impl = *reinterpret_cast<ImplTaskGraph *>(this->object);

        auto const view = impl.id_to_local_id(info.view);

        auto name = info.name.size() > 0 ? std::string(info.name) : std::string("clear image: ") + std::string(impl.global_image_infos.at(view.index).name);

        TaskImageClearInfo cinfo = info;
        add_task(
            InlineTask::Transfer(name)
                .writes(view)
                .uses_queue(info.queue)
                .executes(
                    [=](TaskInterface ti)
                    {
                        for (u32 runtime_img = 0; runtime_img < ti.get(TaskImageAttachmentIndex{0}).ids.size(); ++runtime_img)
                        {
                            ti.recorder.clear_image(ImageClearInfo{
                                .clear_value = cinfo.clear_value,
                                .dst_image = ti.get(TaskImageAttachmentIndex{0}).ids[runtime_img],
                                .dst_slice = cinfo.view.slice,
                            });
                        }
                    }));
    }

    DAXA_EXPORT_CXX void TaskGraph::copy_buffer_to_buffer(TaskBufferCopyInfo const & info)
    {
        ImplTaskGraph & impl = *reinterpret_cast<ImplTaskGraph *>(this->object);
        auto src = impl.buffer_blas_tlas_id_to_local_id(info.src);
        auto dst = impl.buffer_blas_tlas_id_to_local_id(info.dst);

        auto src_i = TaskBufferAttachmentIndex{0};
        auto dst_i = TaskBufferAttachmentIndex{1};

        auto name = info.name.size() > 0 ? std::string(info.name) : std::string("copy ") + std::string(impl.global_buffer_infos.at(src.index).name) + " to " + std::string(impl.global_buffer_infos.at(dst.index).name);

        add_task(
            InlineTask::Transfer(name)
                .reads(src)
                .writes(dst)
                .uses_queue(info.queue)
                .executes(
                    [=](TaskInterface ti)
                    {
                        DAXA_DBG_ASSERT_TRUE_M(ti.get(src_i).ids.size() == ti.get(dst_i).ids.size(), "given src and dst must have the same number of runtime resources");
                        for (u32 runtime_buf = 0; runtime_buf < ti.get(TaskImageAttachmentIndex{0}).ids.size(); ++runtime_buf)
                        {
                            DAXA_DBG_ASSERT_TRUE_M(ti.info(src_i, runtime_buf).value().size == ti.info(dst_i, runtime_buf).value().size, "given src and dst must have the same size");
                            ti.recorder.copy_buffer_to_buffer(BufferCopyInfo{
                                .src_buffer = ti.get(src_i).ids[runtime_buf],
                                .dst_buffer = ti.get(src_i).ids[runtime_buf],
                                .size = ti.info(src_i, runtime_buf).value().size,
                            });
                        }
                    }));
    }

    DAXA_EXPORT_CXX void TaskGraph::copy_image_to_image(TaskImageCopyInfo const & info)
    {
        ImplTaskGraph & impl = *reinterpret_cast<ImplTaskGraph *>(this->object);
        auto src = impl.id_to_local_id(info.src);
        auto dst = impl.id_to_local_id(info.dst);

        auto src_i = TaskImageAttachmentIndex{0};
        auto dst_i = TaskImageAttachmentIndex{1};

        auto name = info.name.size() > 0 ? std::string(info.name) : std::string("copy ") + std::string(impl.global_image_infos.at(src.index).name) + " to " + std::string(impl.global_image_infos.at(dst.index).name);

        add_task(
            InlineTask::Transfer(name)
                .reads(src)
                .writes(dst)
                .uses_queue(info.queue)
                .executes(
                    [=, copy_slice = src.slice](TaskInterface ti)
                    {
                        DAXA_DBG_ASSERT_TRUE_M(ti.get(src_i).ids.size() == ti.get(dst_i).ids.size(), "given src and dst must have the same number of runtime resources");
                        for (u32 runtime_img = 0; runtime_img < ti.get(TaskImageAttachmentIndex{0}).ids.size(); ++runtime_img)
                        {
                            DAXA_DBG_ASSERT_TRUE_M(ti.info(src_i, runtime_img).value().size == ti.info(dst_i, runtime_img).value().size, "given src and dst must have the same size");
                            DAXA_DBG_ASSERT_TRUE_M(ti.info(src_i, runtime_img).value().array_layer_count == ti.info(dst_i, runtime_img).value().array_layer_count, "given src and dst must have the same array layer count");
                            DAXA_DBG_ASSERT_TRUE_M(ti.info(src_i, runtime_img).value().mip_level_count == ti.info(dst_i, runtime_img).value().mip_level_count, "given src and dst must have the same mip level count");
                            for (u32 mip = copy_slice.base_mip_level; mip < (copy_slice.base_mip_level + copy_slice.level_count); ++mip)
                            {
                                ImageArraySlice const array_copy_slice = ImageArraySlice::slice(dst.slice, mip);
                                ti.recorder.copy_image_to_image(ImageCopyInfo{
                                    .src_image = ti.get(src_i).ids[runtime_img],
                                    .dst_image = ti.get(dst_i).ids[runtime_img],
                                    .src_slice = array_copy_slice,
                                    .dst_slice = array_copy_slice,
                                    .extent = {
                                        std::max(1u, ti.info(src_i, runtime_img).value().size.x >> mip),
                                        std::max(1u, ti.info(src_i, runtime_img).value().size.y >> mip),
                                        std::max(1u, ti.info(src_i, runtime_img).value().size.z >> mip),
                                    }});
                            }
                        }
                    }));
    }

    static inline constexpr std::array<ImageId, 64> NULL_IMG_ARRAY = {};

    auto ImplTaskGraph::get_actual_images(TaskImageView id) const -> std::span<ImageId const>
    {
        if (id.is_null())
        {
            return NULL_IMG_ARRAY;
        }
        auto const & global_image = global_image_infos.at(id.index);
        if (global_image.is_external())
        {
            return {global_image.get_external().actual_images.data(),
                    global_image.get_external().actual_images.size()};
        }
        else
        {
            auto const & perm_image = image_infos.at(id.index);
            DAXA_DBG_ASSERT_TRUE_M(perm_image.valid, "Can not get actual image - image is not valid in this permutation");
            return {&perm_image.actual_image, 1};
        }
    }

    auto ImplTaskGraph::id_to_local_id(TaskImageView id) const -> TaskImageView
    {
        if (id.is_null())
            return id;
        DAXA_DBG_ASSERT_TRUE_M(!id.is_empty(), "Detected empty task image id. Please make sure to only use initialized task image ids.");
        if (id.is_external())
        {
            DAXA_DBG_ASSERT_TRUE_MS(
                persistent_image_index_to_local_index.contains(id.index),
                << "Detected invalid access of persistent task image id "
                << id.index
                << " in task graph \""
                << info.name
                << "\". Please make sure to declare persistent resource use to each task graph that uses this image with the function use_persistent_image!");
            return TaskImageView{.task_graph_index = this->unique_index, .index = persistent_image_index_to_local_index.at(id.index), .slice = id.slice};
        }
        else
        {
            DAXA_DBG_ASSERT_TRUE_MS(
                id.task_graph_index == this->unique_index,
                << "Detected invalid access of transient task image id "
                << (id.index)
                << " in task graph \""
                << info.name
                << "\". Please make sure that you only use transient image within the list they are created in!");
            return TaskImageView{.task_graph_index = this->unique_index, .index = id.index, .slice = id.slice};
        }
    }

    void validate_runtime_image_slice(ImplTaskGraph & impl, u32 use_index, u32 task_image_index, ImageMipArraySlice const & access_slice)
    {
#if DAXA_VALIDATION
        auto const actual_images = impl.get_actual_images(TaskImageView{.task_graph_index = impl.unique_index, .index = task_image_index});
        std::string_view task_name = impl.global_image_infos[task_image_index].name;
        for (u32 index = 0; index < actual_images.size(); ++index)
        {
            ImageMipArraySlice const full_slice = impl.info.device.image_view_info(actual_images[index].default_view()).value().slice;
            [[maybe_unused]] bool const use_within_runtime_image_counts =
                (access_slice.base_mip_level + access_slice.level_count <= full_slice.base_mip_level + full_slice.level_count) &&
                (access_slice.base_array_layer + access_slice.layer_count <= full_slice.base_array_layer + full_slice.layer_count);
            if (!use_within_runtime_image_counts)
            {
                auto name_sw = impl.info.device.image_info(actual_images[index]).value().name;
                std::string const & name = {name_sw.data(), name_sw.size()};
                [[maybe_unused]] std::string const error_message =
                    std::format(R"(task image argument (arg index: {}, task image: "{}", slice: {}) exceeds runtime image (index: {}, name: "{}") dimensions ({})!)",
                                use_index, task_name, to_string(access_slice), index, name, to_string(full_slice));
                DAXA_DBG_ASSERT_TRUE_M(use_within_runtime_image_counts, error_message);
            }
        }
#endif
    }

    void validate_image_attachs(ImplTaskGraph & impl, [[maybe_unused]] u32 use_index, u32 task_image_index, TaskAccess task_access, [[maybe_unused]] std::string_view task_name)
    {
#if DAXA_VALIDATION
        ImageUsageFlags const use_flags = access_to_usage(task_access);
        auto const actual_images = impl.get_actual_images(TaskImageView{.task_graph_index = impl.unique_index, .index = task_image_index});
        std::string_view task_image_name = impl.global_image_infos[task_image_index].name;
        for (auto image : actual_images)
        {
            [[maybe_unused]] bool const access_valid = (impl.info.device.image_info(image).value().usage & use_flags) != ImageUsageFlagBits::NONE;
            DAXA_DBG_ASSERT_TRUE_M(access_valid, std::format("Detected invalid runtime image \"{}\" of task image \"{}\", in use {} of task \"{}\". "
                                                             "The given runtime image does NOT have the image use flag {} set, but the task use requires this use for all runtime images!",
                                                             impl.info.device.image_info(image).value().name.view(), task_image_name, use_index, task_name, daxa::to_string(use_flags)));
        }
#endif
    }

    void ImplTaskGraph::update_image_view_cache(ImplTask & task)
    {
        for_each(
            task.attachments,
            [](u32, auto const &) {},
            [&](u32 task_image_attach_index, TaskImageAttachmentInfo const & image_attach)
            {
                // TODO:
                // Replace the validity check with a comparison of last execution actual images vs this frame actual images.
                auto & view_cache = task.image_view_cache[task_image_attach_index];
                ImageId & img_last_exec = task.runtime_images_last_execution[task_image_attach_index];

                if (image_attach.view.is_null())
                {
                    // Initialize the view array once with null ids.
                    if (image_attach.shader_array_size > 0 && view_cache.empty())
                    {
                        for (u32 index = 0; index < image_attach.shader_array_size; ++index)
                        {
                            view_cache[index] = {};
                        }
                    }
                    return;
                }
                auto const slice = image_attach.translated_view.slice;
                // The image id here is already the task graph local id.
                // The persistent ids are converted to local ids in the add_task function.
                auto const tid = image_attach.translated_view;
                auto const actual_images = get_actual_images(tid);

                DAXA_DBG_ASSERT_TRUE_M(actual_images.size() == 1, "TG MK2 ONLY SUPPORTS ONE RESOURCE PER TG RESOURCE!");

                bool cache_valid = img_last_exec == actual_images[0];
                if (!cache_valid)
                {
                    // Save current runtime images for the next time tg updates the views.
                    img_last_exec = actual_images[0];
                    validate_runtime_image_slice(*this, task_image_attach_index, tid.index, slice);
                    validate_image_attachs(*this, task_image_attach_index, tid.index, image_attach.task_access, task.name);
                    for (auto & view : view_cache)
                    {
                        if (info.device.is_id_valid(view))
                        {
                            ImageViewId const parent_image_default_view = info.device.image_view_info(view).value().image.default_view();
                            // Can not destroy the default view of an image!!!
                            if (parent_image_default_view != view)
                            {
                                info.device.destroy_image_view(view);
                            }
                        }
                    }
                    // clear cache
                    for (u32 index = 0; index < image_attach.shader_array_size; ++index)
                    {
                        view_cache[index] = {};
                    }

                    if (image_attach.shader_array_type == TaskHeadImageArrayType::RUNTIME_IMAGES)
                    {
                        auto runtime_image = actual_images.back(); // We do not support generating image views for multiple runtime images!
                        ImageViewInfo view_info = info.device.image_view_info(runtime_image.default_view()).value();
                        // Attachments that do not fill out the view type do not get a view generated!
                        if (image_attach.view_type != ImageViewType::MAX_ENUM)
                        {
                            ImageViewType const use_view_type = image_attach.view_type;

                            // When the use image view parameters match the default view,
                            // then use the default view id and avoid creating a new id here.
                            bool const is_use_default_slice = view_info.slice == slice;
                            bool const is_use_default_view_type = use_view_type == view_info.type;
                            if (is_use_default_slice && is_use_default_view_type)
                            {
                                view_cache[0] = runtime_image.default_view();
                            }
                            else
                            {
                                view_info.type = use_view_type;
                                view_info.slice = slice;
                                view_cache[0] = info.device.create_image_view(view_info);
                            }
                        }
                        else
                        {
                            view_cache[0] = {};
                        }
                    }
                    else // image_attach.shader_array_type == TaskHeadImageArrayType::MIP_LEVELS
                    {
                        u32 const base_mip_level = image_attach.translated_view.slice.base_mip_level;
                        auto filled_views = std::min(image_attach.translated_view.slice.level_count, u32(image_attach.shader_array_size));
                        for (u32 index = 0; index < filled_views; ++index)
                        {
                            ImageViewInfo view_info = info.device.image_view_info(actual_images[0].default_view()).value();
                            ImageViewType const use_view_type = (image_attach.view_type != ImageViewType::MAX_ENUM) ? image_attach.view_type : view_info.type;
                            view_info.type = use_view_type;
                            view_info.slice = image_attach.translated_view.slice;
                            view_info.slice.base_mip_level = base_mip_level + index;
                            view_info.slice.level_count = 1;
                            view_cache[index] = info.device.create_image_view(view_info);
                        }
                        // When the slice is smaller then the array size,
                        // The indices larger then the size are filled with 0 ids.
                        for (u32 index = filled_views; index < image_attach.shader_array_size; ++index)
                        {
                            view_cache[index] = {};
                        }
                    }
                }
            });
    }

    void validate_runtime_resources([[maybe_unused]] ImplTaskGraph const & impl)
    {
#if DAXA_VALIDATION
        constexpr std::string_view PERSISTENT_RESOURCE_MESSAGE = {
            "when executing a task graph, all used persistent resources must be backed by at least one and exclusively "
            "valid runtime resources"};
        for (u32 local_buffer_i = 0; local_buffer_i < impl.global_buffer_infos.size(); ++local_buffer_i)
        {
            if (!impl.buffer_infos[local_buffer_i].valid)
            {
                continue;
            }
            if (!impl.global_buffer_infos.at(local_buffer_i).is_external())
            {
                continue;
            }
            auto const & runtime_ids = impl.global_buffer_infos.at(local_buffer_i).get_external().actual_ids;
            std::visit([&](auto const & runtime_ids)
                       {
                DAXA_DBG_ASSERT_TRUE_M(
                    !runtime_ids.empty(),
                    std::format(
                        "Detected persistent task buffer \"{}\" used in task graph \"{}\" with 0 runtime buffers; {}",
                        impl.global_buffer_infos[local_buffer_i].name,
                        impl.info.name,
                        PERSISTENT_RESOURCE_MESSAGE));
                for (usize buffer_index = 0; buffer_index < runtime_ids.size(); ++buffer_index)
                {
                    DAXA_DBG_ASSERT_TRUE_M(
                        impl.info.device.is_id_valid(runtime_ids[buffer_index]),
                        std::format(
                            "Detected persistent task buffer \"{}\" used in task graph \"{}\" with invalid buffer id (runtime buffer index: {}); {}",
                            impl.global_buffer_infos[local_buffer_i].name,
                            impl.info.name,
                            buffer_index,
                            PERSISTENT_RESOURCE_MESSAGE));
                } }, runtime_ids);
        }
        for (u32 local_image_i = 0; local_image_i < impl.global_image_infos.size(); ++local_image_i)
        {
            if (!impl.image_infos[local_image_i].valid)
            {
                continue;
            }
            if (!impl.global_image_infos.at(local_image_i).is_external())
            {
                continue;
            }
            auto const & runtime_images = impl.global_image_infos.at(local_image_i).get_external().actual_images;
            DAXA_DBG_ASSERT_TRUE_M(
                !runtime_images.empty(),
                std::format(
                    "Detected persistent task image \"{}\" used in task graph \"{}\" with 0 runtime images; {}",
                    impl.global_image_infos[local_image_i].name,
                    impl.info.name,
                    PERSISTENT_RESOURCE_MESSAGE));
            for (usize image_index = 0; image_index < runtime_images.size(); ++image_index)
            {
                DAXA_DBG_ASSERT_TRUE_M(
                    impl.info.device.is_id_valid(runtime_images[image_index]),
                    std::format(
                        "Detected persistent task image \"{}\" used in task graph \"{}\" with invalid image id (runtime image index: {}); {}",
                        impl.global_image_infos[local_image_i].name,
                        impl.info.name,
                        image_index,
                        PERSISTENT_RESOURCE_MESSAGE));
            }
        }
#endif // #if DAXA_VALIDATION
    }

    constexpr usize ATTACHMENT_BLOB_MAX_SIZE = 8192;

    auto write_attachment_shader_blob(Device const & device, [[maybe_unused]] u32 data_size, std::span<TaskAttachmentInfo const> attachments) -> std::array<std::byte, ATTACHMENT_BLOB_MAX_SIZE>
    {
        // TODO: validate that we never overflow this array.
        std::array<std::byte, ATTACHMENT_BLOB_MAX_SIZE> attachment_shader_blob = {};
        usize shader_byte_blob_offset = 0;
        auto upalign = [&](size_t align_size)
        {
            if (align_size == 0)
            {
                return;
            }
            auto current_offset = shader_byte_blob_offset % align_size;
            if (current_offset != 0)
            {
                shader_byte_blob_offset += align_size - current_offset;
            }
        };
        for_each(
            attachments,
            [&](u32, auto const & attach)
            {
                if constexpr (std::is_same_v<std::decay_t<decltype(attach)>, TaskBufferAttachmentInfo>)
                {
                    TaskBufferAttachmentInfo const & buffer_attach = attach;
                    if (buffer_attach.shader_as_address)
                    {
                        upalign(sizeof(DeviceAddress));
                        for (u32 shader_array_i = 0; shader_array_i < buffer_attach.shader_array_size; ++shader_array_i)
                        {
                            BufferId const buf_id = buffer_attach.ids[shader_array_i];
                            DeviceAddress const buf_address = buffer_attach.view.is_null() ? DeviceAddress{} : device.device_address(buf_id).value();
                            auto mini_blob = std::bit_cast<std::array<std::byte, sizeof(DeviceAddress)>>(buf_address);
                            std::memcpy(attachment_shader_blob.data() + shader_byte_blob_offset, &mini_blob, sizeof(DeviceAddress));
                            shader_byte_blob_offset += sizeof(DeviceAddress);
                        }
                    }
                    else
                    {
                        upalign(sizeof(daxa_BufferId));
                        for (u32 shader_array_i = 0; shader_array_i < buffer_attach.shader_array_size; ++shader_array_i)
                        {
                            BufferId const buf_id = buffer_attach.ids[shader_array_i];
                            auto mini_blob = std::bit_cast<std::array<std::byte, sizeof(daxa_BufferId)>>(buf_id);
                            std::memcpy(attachment_shader_blob.data() + shader_byte_blob_offset, &mini_blob, sizeof(daxa_BufferId));
                            shader_byte_blob_offset += sizeof(daxa_BufferId);
                        }
                    }
                }
                if constexpr (std::is_same_v<std::decay_t<decltype(attach)>, TaskTlasAttachmentInfo>)
                {
                    TaskTlasAttachmentInfo const & tlas_attach = attach;
                    if (tlas_attach.shader_as_address)
                    {
                        upalign(sizeof(DeviceAddress));
                        TlasId const tlas_id = attach.ids[0];
                        DeviceAddress const tlas_address = attach.view.is_null() ? DeviceAddress{} : device.device_address(tlas_id).value();
                        auto mini_blob = std::bit_cast<std::array<std::byte, sizeof(DeviceAddress)>>(tlas_address);
                        std::memcpy(attachment_shader_blob.data() + shader_byte_blob_offset, &mini_blob, sizeof(DeviceAddress));
                        shader_byte_blob_offset += sizeof(DeviceAddress);
                    }
                    else
                    {
                        upalign(sizeof(daxa_TlasId));
                        TlasId const tlas_id = tlas_attach.ids[0];
                        auto mini_blob = std::bit_cast<std::array<std::byte, sizeof(daxa_TlasId)>>(tlas_id);
                        std::memcpy(attachment_shader_blob.data() + shader_byte_blob_offset, &mini_blob, sizeof(daxa_TlasId));
                        shader_byte_blob_offset += sizeof(daxa_TlasId);
                    }
                }
            },
            [&](u32, TaskImageAttachmentInfo const & image_attach)
            {
                if (image_attach.shader_as_index)
                {
                    upalign(sizeof(daxa_ImageViewIndex));
                    for (u32 shader_array_i = 0; shader_array_i < image_attach.shader_array_size; ++shader_array_i)
                    {
                        ImageViewId const img_id = image_attach.view_ids[shader_array_i];
                        auto mini_blob = std::bit_cast<std::array<std::byte, sizeof(daxa_ImageViewIndex)>>(static_cast<uint32_t>(img_id.index));
                        std::memcpy(attachment_shader_blob.data() + shader_byte_blob_offset, &mini_blob, sizeof(daxa_ImageViewIndex));
                        shader_byte_blob_offset += sizeof(daxa_ImageViewIndex);
                    }
                }
                else
                {
                    upalign(sizeof(daxa_ImageViewId));
                    for (u32 shader_array_i = 0; shader_array_i < image_attach.shader_array_size; ++shader_array_i)
                    {
                        ImageViewId const img_id = image_attach.view_ids[shader_array_i];
                        auto mini_blob = std::bit_cast<std::array<std::byte, sizeof(daxa_ImageViewId)>>(img_id);
                        std::memcpy(attachment_shader_blob.data() + shader_byte_blob_offset, &mini_blob, sizeof(daxa_ImageViewId));
                        shader_byte_blob_offset += sizeof(daxa_ImageViewId);
                    }
                }
            });
        return attachment_shader_blob;
    }

    void ImplTaskGraph::execute_task(ImplTaskRuntimeInterface & impl_runtime, usize batch_index, TaskBatchId in_batch_task_index, TaskId task_id, Queue queue)
    {
        // We always allow to reuse the last command list ONCE within the task callback.
        // When the get command list function is called in a task this is set to false.
        // TODO(refactor): create discrete validation functions and call them before doing any work here.
        impl_runtime.reuse_last_command_list = true;
        ImplTask & task = tasks[task_id];
        update_image_view_cache(task);
        for_each(
            task.attachments,
            [&](u32, auto & attach)
            {
                attach.ids = this->get_actual_buffer_blas_tlas(attach.translated_view);
                validate_task_buffer_blas_tlas_runtime_data(task, attach);
            },
            [&](u32 index, TaskImageAttachmentInfo & attach)
            {
                attach.ids = this->get_actual_images(attach.translated_view);
                attach.view_ids = std::span{task.image_view_cache[index].data(), task.image_view_cache[index].size()};
                validate_task_image_runtime_data(task, attach);
            });
        std::array<std::byte, ATTACHMENT_BLOB_MAX_SIZE> attachment_shader_blob =
            write_attachment_shader_blob(
                info.device,
                task.attachment_shader_blob_size,
                task.attachments);
        impl_runtime.current_task = &task;
        if (this->info.enable_command_labels)
        {
            static std::string tag = {};
            tag.clear();
            std::format_to(std::back_inserter(tag), "batch {} task {} \"{}\"", batch_index, in_batch_task_index, task.name);
            SmallString stag = SmallString{tag};
            impl_runtime.recorder.begin_label({
                .label_color = info.task_label_color,
                .name = stag,
            });
        }
        auto interface = TaskInterface{
            .device = this->info.device,
            .recorder = impl_runtime.recorder,
            .attachment_infos = task.attachments,
            .allocator = this->staging_memory.has_value() ? &this->staging_memory.value() : nullptr,
            .attachment_shader_blob = {attachment_shader_blob.data(), task.attachment_shader_blob_size},
            .task_name = task.name,
            .task_index = task_id,
            .queue = queue,
        };
        if (info.pre_task_callback)
        {
            info.pre_task_callback(interface);
        }
        task.task_callback(interface, task.task_callback_memory);
        if (info.post_task_callback)
        {
            info.post_task_callback(interface);
        }
        if (this->info.enable_command_labels)
        {
            impl_runtime.recorder.end_label();
        }
    }

    void TaskGraph::conditional(TaskGraphConditionalInfo const & conditional_info)
    {
    }

    template <typename TrackedState>
    struct AccessRelation
    {
        bool is_previous_none = {};
        bool is_previous_read = {};
        bool is_current_read = {};
        bool is_previous_rw_concurrent = {};
        bool is_current_rw_concurrent = {};
        bool is_current_concurrent = {};
        bool are_both_read = {};
        bool are_both_rw_concurrent = {};
        bool are_layouts_identical = {};
        bool are_both_read_and_same_layout = {};
        bool are_both_rw_concurrent_and_same_layout = {};
        bool are_both_concurrent = {};
        bool are_both_concurrent_and_same_layout = {};
        AccessRelation(TrackedState latest, Access new_access, TaskAccessConcurrency new_concurrency, ImageLayout latest_layout = ImageLayout::UNDEFINED, ImageLayout new_layout = ImageLayout::UNDEFINED)
        {
            if constexpr (requires { latest.latest_access; })
            {
                is_previous_none = latest.latest_access.type == AccessTypeFlagBits::NONE;
                is_previous_read = latest.latest_access.type == AccessTypeFlagBits::READ;
                is_previous_rw_concurrent =
                    latest.latest_access.type == AccessTypeFlagBits::READ_WRITE &&
                    latest.latest_access_concurrent == TaskAccessConcurrency::CONCURRENT;
            }
            if constexpr (requires { latest.state.latest_access; })
            {
                is_previous_none = latest.state.latest_access.type == AccessTypeFlagBits::NONE;
                is_previous_read = latest.state.latest_access.type == AccessTypeFlagBits::READ;
                is_previous_rw_concurrent =
                    latest.state.latest_access.type == AccessTypeFlagBits::READ_WRITE &&
                    latest.latest_access_concurrent == TaskAccessConcurrency::CONCURRENT;
            }
            is_current_read = new_access.type == AccessTypeFlagBits::READ;
            is_current_rw_concurrent =
                new_access.type == AccessTypeFlagBits::READ_WRITE &&
                new_concurrency == TaskAccessConcurrency::CONCURRENT;
            is_current_concurrent = is_current_read || is_current_rw_concurrent;
            are_both_read = is_previous_read && is_current_read;
            are_both_rw_concurrent = is_previous_rw_concurrent && is_current_rw_concurrent;
            are_layouts_identical = latest_layout == new_layout;
            are_both_read_and_same_layout = are_both_read && are_layouts_identical;
            are_both_rw_concurrent_and_same_layout = are_both_rw_concurrent && are_layouts_identical;
            are_both_concurrent = (is_previous_read && is_current_read) || (is_previous_rw_concurrent && is_current_rw_concurrent);
            are_both_concurrent_and_same_layout = are_both_concurrent && are_layouts_identical;
        }
    };

    auto schedule_task(
        ImplTaskGraph & impl,
        TaskBatchSubmitScope & current_submit_scope,
        usize const current_submit_scope_index,
        ImplTask & task,
        Queue queue)
        -> usize
    {
        QueueSubmitScope & queue_submit_scope = current_submit_scope.queue_submit_scopes[flat_queue_index(queue)];
        usize first_possible_batch_index = 0;
        if (!impl.info.reorder_tasks)
        {
            first_possible_batch_index = std::max(queue_submit_scope.task_batches.size(), static_cast<decltype(queue_submit_scope.task_batches.size())>(1)) - 1ull;
        }

        for_each(
            task.attachments,
            [&](u32, auto const & attach)
            {
                if (attach.view.is_null())
                    return;
                PerPermTaskBuffer const & task_buffer = impl.buffer_infos[attach.translated_view.index];
                // If the latest access is in a previous submit scope, the earliest batch we can insert into is
                // the current scopes first batch.
                if (task_buffer.latest_access_submit_scope_index < current_submit_scope_index)
                {
                    return;
                }

                auto [current_buffer_access, current_access_concurrency] = task_access_to_access(attach.task_access);
                // Every other access (NONE, READ_WRITE, WRITE) are interpreted as writes in this context.
                // When a buffer has been read in a previous use AND the current task also reads the buffer,
                // we must insert the task at or after the last use batch.
                // When two buffer accesses intersect, we potentially need to insert a ner barrier or modify an existing barrier.
                // If the access is a read on read or a rw_concurrent on rw_concurrent, the task is still allowed within the same batch as the task of the previous access.
                // This means that two tasks reading from buffer X are allowed within the same batch, using the same barrier.
                // If they are not inserted within the same batch due to dependencies of other attachments, daxa will still reuse the barriers.
                // This is only possible for read write concurrent and read access sequences!
                AccessRelation<decltype(task_buffer)> relation{task_buffer, current_buffer_access, current_access_concurrency};
                usize current_buffer_first_possible_batch_index = 0;
                if (relation.is_previous_none)
                {
                    current_buffer_first_possible_batch_index = task_buffer.latest_access_batch_index;
                }
                else if (relation.are_both_concurrent && (task_buffer.latest_concurrent_sequence_start_batch != ~0u))
                {
                    // N following concurrent accesses are allowed to be reordered to any point within a concurrent access sequence.
                    current_buffer_first_possible_batch_index = task_buffer.latest_concurrent_sequence_start_batch;
                }
                else
                {
                    current_buffer_first_possible_batch_index = task_buffer.latest_access_batch_index + 1;
                }
                first_possible_batch_index = std::max(first_possible_batch_index, current_buffer_first_possible_batch_index);
            },
            [&](u32, TaskImageAttachmentInfo const & attach)
            {
                if (attach.view.is_null())
                    return;
                PerPermTaskImage const & task_image = impl.image_infos[attach.translated_view.index];
                PermIndepTaskImageInfo const & glob_task_image = impl.global_image_infos[attach.translated_view.index];
                DAXA_DBG_ASSERT_TRUE_M(!task_image.swapchain_semaphore_waited_upon, "swapchain image is already presented!");

                if (glob_task_image.is_external() && glob_task_image.get_external().info.swapchain_image)
                {
                    if (impl.swapchain_image_first_use_submit_scope_index == std::numeric_limits<u64>::max())
                    {
                        impl.swapchain_image_first_use_submit_scope_index = current_submit_scope_index;
                        impl.swapchain_image_last_use_submit_scope_index = current_submit_scope_index;
                    }
                    else
                    {
                        impl.swapchain_image_first_use_submit_scope_index = std::min(current_submit_scope_index, impl.swapchain_image_first_use_submit_scope_index);
                        impl.swapchain_image_last_use_submit_scope_index = std::max(current_submit_scope_index, impl.swapchain_image_last_use_submit_scope_index);
                    }
                }

                auto [this_task_image_layout, this_task_image_access, current_access_concurrent] = task_image_access_to_layout_access(attach.task_access);
                // As image subresources can be in different layouts and also different synchronization scopes,
                // we need to track these image ranges individually.
                for (ExtendedImageSliceState const & tracked_slice : task_image.last_slice_states)
                {
                    // If the latest access is in a previous submit scope, the earliest batch we can insert into is
                    // the current scopes first batch.
                    // When the slices dont intersect, we dont need to do any sync or execution ordering between them.
                    if (
                        tracked_slice.latest_access_submit_scope_index < current_submit_scope_index ||
                        !tracked_slice.state.slice.intersects(attach.translated_view.slice))
                    {
                        continue;
                    }
                    // Tasks are always shedules after or with the tasks they depend on.
                    // When two image accesses intersect, we potentially need to insert a ner barrier or modify an existing barrier.
                    // If the access is a read on read or a rw_concurrent on rw_concurrent, the task is still allowed within the same batch as the task of the previous access.
                    // This means that two tasks reading from image X are allowed within the same batch, using the same barrier.
                    // If they are not inserted within the same batch due to dependencies of other attachments, daxa will still reuse the barriers.
                    // This is only possible for read write concurrent and read access sequences!
                    AccessRelation<decltype(tracked_slice)> relation{tracked_slice, this_task_image_access, current_access_concurrent, tracked_slice.state.latest_layout, this_task_image_layout};
                    usize current_image_first_possible_batch_index = 0;
                    if (relation.is_previous_none)
                    {
                        current_image_first_possible_batch_index = tracked_slice.latest_access_batch_index;
                    }
                    else if (relation.are_both_concurrent_and_same_layout && (tracked_slice.latest_concurrent_sequence_start_batch != ~0u))
                    {
                        // N following concurrent accesses are allowed to be reordered to any point within a concurrent access sequence.
                        current_image_first_possible_batch_index = tracked_slice.latest_concurrent_sequence_start_batch;
                    }
                    else
                    {
                        current_image_first_possible_batch_index = tracked_slice.latest_access_batch_index + 1;
                    }
                    first_possible_batch_index = std::max(first_possible_batch_index, current_image_first_possible_batch_index);
                }
            });
        // Make sure we have enough batches.
        if (first_possible_batch_index >= queue_submit_scope.task_batches.size())
        {
            queue_submit_scope.task_batches.resize(static_cast<u32>(first_possible_batch_index) + 1u,
                                                   TaskBatch{
                                                       .pipeline_barrier_indices = {&impl.task_memory},
                                                       .wait_split_barrier_indices = {&impl.task_memory},
                                                       .tasks = {&impl.task_memory},
                                                       .signal_split_barrier_indices = {&impl.task_memory},
                                                   });
        }
        return first_possible_batch_index;
    }

    void validate_attachment_views(ImplTaskGraph const &, ImplTask & task)
    {
        for_each(
            task.attachments,
            [&](u32 i, auto & attach)
            {
                validate_buffer_blas_tlas_task_view(task, i, attach);
            },
            [&](u32 i, TaskImageAttachmentInfo & attach)
            {
                validate_image_task_view(task, i, attach);
            });
    }

    void validate_task_type_queue(ImplTaskGraph const &, ImplTask & task, Queue queue)
    {
        TaskType task_type = task.task_type;
        switch (task_type)
        {
        case TaskType::UNDEFINED:
        {
            DAXA_DBG_ASSERT_TRUE_M(false, "Detected invalid task type UNDEFINED");
            break;
        }
        case TaskType::GENERAL: [[fallthrough]];
        case TaskType::RASTER:
        {
            DAXA_DBG_ASSERT_TRUE_M(
                queue == daxa::QUEUE_MAIN,
                std::format("Detected invalid multi-queue use."
                            " Task \"{}\" of type {} was added to be run on the queue {}."
                            " GENERAL/RASTER tasks are only allowed to be run on the main queue."
                            " This is a hardware limitation.",
                            task.name, to_string(task.task_type), to_string(queue)));
            break;
        }
        case TaskType::COMPUTE: [[fallthrough]];
        case TaskType::RAY_TRACING:
        {
            DAXA_DBG_ASSERT_TRUE_M(
                queue == daxa::QUEUE_MAIN || queue.family == daxa::QueueFamily::COMPUTE,
                std::format("Detected invalid multi-queue use."
                            " Task \"{}\" of type {} was added to be run on the queue {}."
                            " COMPUTE/RAYTRACING tasks are only allowed to be run on compute queues or the main queue."
                            " This is a hardware limitation.",
                            task.name, to_string(task.task_type), to_string(queue)));
            break;
        }
        case TaskType::TRANSFER:
        {
            break;
        }
        }
    }

    auto TaskGraph::allocate_task_memory(usize size, usize align) -> void *
    {
        auto & impl = *reinterpret_cast<ImplTaskGraph *>(this->object);
        auto task_memory = impl.task_memory.allocate(size, align);
        DAXA_DBG_ASSERT_TRUE_M(task_memory != nullptr, "TaskGraph ran out of task memory, please increase the task memory pool");
        return task_memory;
    }

    auto allocate_view_cache(ImplTaskGraph & impl, std::span<TaskAttachmentInfo> attachments) -> std::span<std::span<ImageViewId>>
    {
        std::span<std::span<ImageViewId>> view_cache = impl.task_memory.allocate_trivial_span<std::span<ImageViewId>>(attachments.size());

        for (u32 ai = 0; ai < attachments.size(); ++ai)
        {
            u32 const view_count = attachments[ai].shader_array_size() > 1 ? attachments[ai].shader_array_size() : 1;
            view_cache[ai] = impl.task_memory.allocate_trivial_span<ImageViewId>(view_count);
        }

        return view_cache;
    }

    template<typename TaskResourceIdT>
    auto validate_and_translate_view(ImplTaskGraph & impl, auto & translation_table, TaskResourceIdT id) -> TaskResourceIdT
    {
        DAXA_DBG_ASSERT_TRUE_M(!id.is_empty(), "Detected empty task buffer id. All ids must either be filled with a valid id or null.");
        
        if (id.is_null())
        {
            return id;
        }

        if (id.is_external())
        {
            DAXA_DBG_ASSERT_TRUE_M(
                translation_table.contains(id.index),
                std::format("Detected invalid access of persistent task buffer id ({}) in task graph \"{}\"; "
                            "please make sure to declare persistent resource use to each task graph that uses this buffer with the function use_persistent_buffer!",
                            id.index, impl.info.name));
            TaskResourceIdT translated_id = id;
            translated_id.task_graph_index = impl.unique_index;
            translated_id.index = translation_table.at(id.index);
            return translated_id;
        }

        DAXA_DBG_ASSERT_TRUE_M(
            id.task_graph_index == impl.unique_index,
            std::format("Detected invalid access of transient task buffer id ({}) in task graph \"{}\"; "
                        "please make sure that you only use transient buffers within the list they are created in!",
                        id.index, impl.info.name));
        return id;
    }

    void translate_persistent_ids(ImplTaskGraph const & impl, ImplTask & task)
    {
        for_each(
            task.attachments,
            [&](u32, auto & attach)
            {
                attach.translated_view = impl.buffer_blas_tlas_id_to_local_id(attach.view);
            },
            [&](u32, TaskImageAttachmentInfo & attach)
            {
                attach.translated_view = impl.id_to_local_id(attach.view);
            });
    }

    void TaskGraph::add_task(
        void (*task_callback)(daxa::TaskInterface, void*),
        u64* task_callback_memory,
        std::span<TaskAttachmentInfo> attachments,
        u32 attachment_shader_blob_size,
        u32 attachment_shader_blob_alignment,
        TaskType task_type,
        std::string_view name,
        Queue queue)
    {
        auto & impl = *reinterpret_cast<ImplTaskGraph *>(this->object);
        validate_not_compiled(impl);

        // TODO: Remove this once we remove the old backend.
        queue = queue == QUEUE_NONE ? impl.info.default_queue : queue;

        ImplTask impl_task = ImplTask{
            .task_callback = task_callback,
            .task_callback_memory = task_callback_memory,
            .attachments = attachments,
            .attachment_shader_blob_size = attachment_shader_blob_size,
            .attachment_shader_blob_alignment = attachment_shader_blob_alignment,
            .task_type = task_type,
            .name = impl.task_memory.allocate_copy_string(name),
            .queue = queue,
            .image_view_cache = allocate_view_cache(impl, attachments),
            .runtime_images_last_execution = impl.task_memory.allocate_trivial_span_fill<ImageId>( attachments.size(), daxa::ImageId{} ),
        };

        /// TODOTG: validate attachment access'es and stage's are valid for task and queue type
        /// TODOTG: validate that no resource is used by multiple attachments in the same task 

        // Translate external to native ids
        {
            for (TaskAttachmentInfo & attachment : impl_task.attachments)
            {
                switch(attachment.type)
                {
                    case TaskAttachmentType::BUFFER: 
                        attachment.value.buffer.translated_view = validate_and_translate_view(impl, impl.external_buffer_translation_table, attachment.value.buffer.view); 
                        break;
                    case TaskAttachmentType::BLAS: 
                        attachment.value.blas.translated_view = validate_and_translate_view(impl, impl.external_buffer_translation_table, attachment.value.blas.view); 
                        break;
                    case TaskAttachmentType::TLAS: 
                        attachment.value.tlas.translated_view = validate_and_translate_view(impl, impl.external_buffer_translation_table, attachment.value.tlas.view); 
                        break;
                    case TaskAttachmentType::IMAGE: 
                        attachment.value.image.translated_view = validate_and_translate_view(impl, impl.external_image_translation_table, attachment.value.image.view); 
                        break;
                    default:
                        DAXA_DBG_ASSERT_TRUE_M(false, "ERROR: DETECTED INVALID ATTACHMENT TYPE IN TASKRECORDING!");
                }
            }
        }

        impl.tasks.push_back(impl_task);
    }

    void TaskGraph::submit(TaskSubmitInfo const & info)
    {
    }

    void TaskGraph::present(TaskPresentInfo const & info)
    {
    }

    void ImplTaskGraph::create_transient_runtime_buffers_and_tlas()
    {
    }

    void ImplTaskGraph::create_transient_runtime_images()
    {
    }

    void ImplTaskGraph::allocate_transient_resources()
    {
    }

    void TaskGraph::complete(TaskCompleteInfo const & /*unused*/)
    {
    }

    auto TaskGraph::get_transient_memory_size() -> daxa::usize
    {
        auto & impl = *r_cast<ImplTaskGraph *>(this->object);
        return impl.memory_block_size;
    }

    /// Execution flow:
    /// 1. choose permutation based on conditionals
    /// 2. validate used persistent resources, based on permutation
    /// 3. runtime generate and insert runtime sync for persistent resources.
    /// 4. for every submit scope:
    ///     2.1 for every batch in scope:
    ///         3.1 wait for pipeline and split barriers
    ///         3.2 for every task:
    ///             4.1 validate runtime resources of used resources
    ///             4.2 refresh image view cache.
    ///             4.3 collect shader use handles, allocate gpu local staging memory, copy in handles and bind to constant buffer binding.
    ///             4.4 run task
    ///         3.3 signal split barriers
    ///     2.2 check if submit scope submits work, either submit or collect cmd lists and sync primitives for query
    ///     2.3 check if submit scope presents, present if true.
    void TaskGraph::execute(ExecutionInfo const & info)
    {
        auto & impl = *r_cast<ImplTaskGraph *>(this->object);
        DAXA_DBG_ASSERT_TRUE_M(info.permutation_condition_values.size() >= impl.info.permutation_condition_count, "Detected invalid permutation condition count");
        DAXA_DBG_ASSERT_TRUE_M(impl.compiled, "task graphs must be completed before execution");
    }

    ImplTaskGraph::ImplTaskGraph(TaskGraphInfo a_info)
        : unique_index{ImplTaskGraph::exec_unique_next_index++}, info{std::move(a_info)},
          task_memory{"TaskGraph task memory pool", info.task_memory_pool_size}
    {
        info.name = task_memory.allocate_copy_string(info.name);
        tasks = ArenaDynamicArray8k<ImplTask>(&task_memory);

        gpu_submit_timeline_semaphores = std::array{
            info.device.create_timeline_semaphore({.name = "Task Graph Timeline MAIN"}),
            info.device.create_timeline_semaphore({.name = "Task Graph Timeline COMPUTE_0"}),
            info.device.create_timeline_semaphore({.name = "Task Graph Timeline COMPUTE_1"}),
            info.device.create_timeline_semaphore({.name = "Task Graph Timeline COMPUTE_2"}),
            info.device.create_timeline_semaphore({.name = "Task Graph Timeline COMPUTE_3"}),
            info.device.create_timeline_semaphore({.name = "Task Graph Timeline TRANSFER_0"}),
            info.device.create_timeline_semaphore({.name = "Task Graph Timeline TRANSFER_1"}),
        };
        if (a_info.staging_memory_pool_size != 0)
        {
            this->staging_memory = TransferMemoryPool{TransferMemoryPoolInfo{.device = info.device, .capacity = info.staging_memory_pool_size, .name = "Transfer Memory Pool"}};
        }
    }

    ImplTaskGraph::~ImplTaskGraph()
    {
        for (auto & task : tasks)
        {
            for (auto & view_cache : task.image_view_cache)
            {
                for (auto & view : view_cache)
                {
                    if (info.device.is_id_valid(view))
                    {
                        ImageId const parent = info.device.image_view_info(view).value().image;
                        bool const is_default_view = parent.default_view() == view;
                        if (!is_default_view)
                        {
                            info.device.destroy_image_view(view);
                        }
                    }
                }
            }
        }
        {
            // because transient buffers are owned by the task graph, we need to destroy them
            for (u32 buffer_info_idx = 0; buffer_info_idx < static_cast<u32>(global_buffer_infos.size()); buffer_info_idx++)
            {
                auto const & global_buffer = global_buffer_infos.at(buffer_info_idx);
                PerPermTaskBuffer & perm_buffer = buffer_infos.at(buffer_info_idx);
                if (!global_buffer.is_external() &&
                    perm_buffer.valid)
                {
                    if (auto const * id = std::get_if<BufferId>(&perm_buffer.actual_id))
                    {
                        info.device.destroy_buffer(*id);
                    }
                    if (auto const * id = std::get_if<BlasId>(&perm_buffer.actual_id))
                    {
                        info.device.destroy_blas(*id);
                    }
                    if (auto const * id = std::get_if<TlasId>(&perm_buffer.actual_id))
                    {
                        info.device.destroy_tlas(*id);
                    }
                    if (!perm_buffer.as_backing_buffer_id.is_empty())
                    {
                        info.device.destroy_buffer(perm_buffer.as_backing_buffer_id);
                    }
                }
            }
            // because transient images are owned by the task graph, we need to destroy them
            for (u32 image_info_idx = 0; image_info_idx < static_cast<u32>(global_image_infos.size()); image_info_idx++)
            {
                auto const & global_image = global_image_infos.at(image_info_idx);
                auto & perm_image = image_infos.at(image_info_idx);
                if (!global_image.is_external() && perm_image.valid)
                {
                    info.device.destroy_image(get_actual_images(TaskImageView{.task_graph_index = unique_index, .index = image_info_idx})[0]);
                }
            }
        }
    }

    auto TaskGraph::get_debug_string() -> std::string
    {
        return "";
    }

    auto TaskGraph::inc_refcnt(ImplHandle const * object) -> u64
    {
        return object->inc_refcnt();
    }

    auto TaskGraph::dec_refcnt(ImplHandle const * object) -> u64
    {
        return object->dec_refcnt(
            ImplTaskGraph::zero_ref_callback,
            nullptr);
    }

    void ImplTaskGraph::zero_ref_callback(ImplHandle const * handle)
    {
        auto const * self = r_cast<ImplTaskGraph const *>(handle);
        delete self;
    }
} // namespace daxa

#endif