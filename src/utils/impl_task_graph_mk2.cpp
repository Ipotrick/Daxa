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
        MemoryArena arena = {"Test Arena", mem};

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

    template <typename TaskResourceIdT>
    auto validate_and_translate_view(ImplTaskGraph & impl, TaskResourceIdT id) -> TaskResourceIdT
    {
        DAXA_DBG_ASSERT_TRUE_M(!id.is_empty(), "Detected empty task buffer id. All ids must either be filled with a valid id or null.");

        if (id.is_null())
        {
            return id;
        }

        if (id.is_external())
        {
            DAXA_DBG_ASSERT_TRUE_M(
                impl.external_idx_to_resource_table.contains(id.index),
                std::format("Detected invalid access of external resource id ({}) in task graph \"{}\"; "
                            "please make sure to declare external resource use to each task graph that uses this buffer with the function use_persistent_buffer!",
                            id.index, impl.info.name));
            TaskResourceIdT translated_id = id;
            translated_id.task_graph_index = impl.unique_index;
            translated_id.index = impl.external_idx_to_resource_table.at(id.index).second;
            return translated_id;
        }

        DAXA_DBG_ASSERT_TRUE_M(
            id.task_graph_index == impl.unique_index,
            std::format("Detected invalid access of transient resource id ({}) in task graph \"{}\"; "
                        "please make sure that you only use transient resources within the list they are created in!",
                        id.index, impl.info.name));
        return id;
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
        case TaskAccessType::WRITE: ret = AccessTypeFlagBits::WRITE; break;
        case TaskAccessType::READ_WRITE: ret = AccessTypeFlagBits::READ_WRITE; break;
        case TaskAccessType::WRITE_CONCURRENT: ret = AccessTypeFlagBits::WRITE; break;
        case TaskAccessType::READ_WRITE_CONCURRENT: ret = AccessTypeFlagBits::READ_WRITE; break;
        default: DAXA_DBG_ASSERT_TRUE_M(false, "INVALID ACCESS TYPE"); break;
        }
        return ret;
    }

    auto task_stage_to_pipeline_stage(TaskStage stage) -> PipelineStageFlags
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

    auto access_to_image_usage(TaskAccess const & taccess) -> ImageUsageFlags
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
        Access const access = Access{task_stage_to_pipeline_stage(taccess.stage), to_access_type(taccess.type)};
        return {access, concurrent};
    }

    auto task_access_type_to_access_type(TaskAccessType const & taccess_type) -> AccessTypeFlags
    {
        AccessTypeFlags ret = {};
        if ((static_cast<u32>(taccess_type) & static_cast<u32>(TaskAccessType::READ)) != 0)
        {
            ret |= AccessTypeFlagBits::READ;
        }
        if ((static_cast<u32>(taccess_type) & static_cast<u32>(TaskAccessType::WRITE)) != 0)
        {
            ret |= AccessTypeFlagBits::WRITE;
        }
        return ret;
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
          info{std::move(a_info)},
          unique_index{exec_unique_resource_next_index++}
    {
        DAXA_DBG_ASSERT_TRUE_M(a_info.initial_buffers.buffers.size() <= 1, "TaskGraph MK2 does not support multiple runtime resources per task resource");
    }

    ImplPersistentTaskBufferBlasTlas::ImplPersistentTaskBufferBlasTlas(Device & device, BufferInfo const & a_info)
        : actual_ids{std::vector<BufferId>{device.create_buffer(a_info)}},
          info{TaskBufferInfo{.name = a_info.name.c_str().data()}},
          owned_buffer_device{device},
          owned_buffer_info{a_info},
          unique_index{exec_unique_resource_next_index++}
    {
    }

    ImplPersistentTaskBufferBlasTlas::ImplPersistentTaskBufferBlasTlas(TaskBlasInfo a_info)
        : actual_ids{std::vector<BlasId>{a_info.initial_blas.blas.begin(), a_info.initial_blas.blas.end()}},
          info{std::move(a_info)},
          unique_index{exec_unique_resource_next_index++}
    {
        DAXA_DBG_ASSERT_TRUE_M(a_info.initial_blas.blas.size() <= 1, "TaskGraph MK2 does not support multiple runtime resources per task resource");
    }

    ImplPersistentTaskBufferBlasTlas::ImplPersistentTaskBufferBlasTlas(TaskTlasInfo a_info)
        : actual_ids{std::vector<TlasId>{a_info.initial_tlas.tlas.begin(), a_info.initial_tlas.tlas.end()}},
          info{std::move(a_info)},
          unique_index{exec_unique_resource_next_index++}
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
            .latest_access = impl.pre_graph_access,
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
        impl.pre_graph_used_queues_bitfield = ~0u;
        impl.pre_graph_access = {};
    }

    void TaskBuffer::swap_buffers(TaskBuffer & other)
    {
        auto & impl = *r_cast<ImplPersistentTaskBufferBlasTlas *>(this->object);
        auto & actual_buffers = std::get<std::vector<BufferId>>(impl.actual_ids);
        auto & impl_other = *r_cast<ImplPersistentTaskBufferBlasTlas *>(other.object);
        auto & other_actual_buffers = std::get<std::vector<BufferId>>(impl_other.actual_ids);
        std::swap(actual_buffers, other_actual_buffers);
        std::swap(impl.pre_graph_access, impl_other.pre_graph_access);
        std::swap(impl.pre_graph_used_queues_bitfield, impl_other.pre_graph_used_queues_bitfield);
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
            .latest_access = impl.pre_graph_access,
        };
    }

    void TaskBlas::set_blas(TrackedBlas const & other_tracked)
    {
        auto & impl = *r_cast<ImplPersistentTaskBufferBlasTlas *>(this->object);
        auto & actual_ids = std::get<std::vector<BlasId>>(impl.actual_ids);
        actual_ids.clear();
        actual_ids.insert(actual_ids.end(), other_tracked.blas.begin(), other_tracked.blas.end());
        impl.pre_graph_used_queues_bitfield = ~0u;
        impl.pre_graph_access = {};
    }

    void TaskBlas::swap_blas(TaskBlas & other)
    {
        auto & impl = *r_cast<ImplPersistentTaskBufferBlasTlas *>(this->object);
        auto & actual_buffers = std::get<std::vector<BlasId>>(impl.actual_ids);
        auto & impl_other = *r_cast<ImplPersistentTaskBufferBlasTlas *>(other.object);
        auto & other_actual_buffers = std::get<std::vector<BlasId>>(impl_other.actual_ids);
        std::swap(actual_buffers, other_actual_buffers);
        std::swap(impl.pre_graph_access, impl_other.pre_graph_access);
        std::swap(impl.pre_graph_used_queues_bitfield, impl_other.pre_graph_used_queues_bitfield);
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
            .latest_access = impl.pre_graph_access,
        };
    }

    void TaskTlas::set_tlas(TrackedTlas const & other_tracked)
    {
        auto & impl = *r_cast<ImplPersistentTaskBufferBlasTlas *>(this->object);
        auto & actual_ids = std::get<std::vector<TlasId>>(impl.actual_ids);
        actual_ids.clear();
        actual_ids.insert(actual_ids.end(), other_tracked.tlas.begin(), other_tracked.tlas.end());
        impl.pre_graph_used_queues_bitfield = ~0u;
        impl.pre_graph_access = {};
    }

    void TaskTlas::swap_tlas(TaskTlas & other)
    {
        auto & impl = *r_cast<ImplPersistentTaskBufferBlasTlas *>(this->object);
        auto & actual_buffers = std::get<std::vector<TlasId>>(impl.actual_ids);
        auto & impl_other = *r_cast<ImplPersistentTaskBufferBlasTlas *>(other.object);
        auto & other_actual_ids = std::get<std::vector<TlasId>>(impl_other.actual_ids);
        std::swap(actual_buffers, other_actual_ids);
        std::swap(impl.pre_graph_access, impl_other.pre_graph_access);
        std::swap(impl.pre_graph_used_queues_bitfield, impl_other.pre_graph_used_queues_bitfield);
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
          unique_index{exec_unique_resource_next_index++}
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
            .latest_slice_states = {}, // Not supported any longer
        };
    }

    void TaskImage::set_images(TrackedImages const & images)
    {
        auto & impl = *r_cast<ImplPersistentTaskImage *>(this->object);
        DAXA_DBG_ASSERT_TRUE_M(!impl.info.swapchain_image || (images.images.size() == 1), "swapchain task image can only have at most one runtime image");
        impl.actual_images.clear();
        impl.actual_images.insert(impl.actual_images.end(), images.images.begin(), images.images.end());
        impl.pre_graph_access = images.latest_slice_states.size() > 0 ? images.latest_slice_states[0].latest_access : Access{};
        impl.pre_graph_used_queues_bitfield = ~0u;
        impl.pre_graph_is_undefined_layout = false;
    }

    void TaskImage::swap_images(TaskImage & other)
    {
        auto & impl = *r_cast<ImplPersistentTaskImage *>(this->object);
        auto & impl_other = *r_cast<ImplPersistentTaskImage *>(other.object);
        DAXA_DBG_ASSERT_TRUE_M(!impl.info.swapchain_image || (impl_other.actual_images.size() <= 1), "swapchain task image can only have at most one runtime image");
        std::swap(impl.pre_graph_access, impl_other.pre_graph_access);
        std::swap(impl.pre_graph_used_queues_bitfield, impl_other.pre_graph_used_queues_bitfield);
        std::swap(impl.pre_graph_is_undefined_layout, impl_other.pre_graph_is_undefined_layout);
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

    TaskGraph::TaskGraph(TaskGraphInfo const & info)
    {
        this->object = new ImplTaskGraph(info);
        auto & impl = *r_cast<ImplTaskGraph *>(this->object);
        auto queue_submit_scopes = std::array{
            QueueSubmitScope{.last_minute_barrier_indices = {&impl.task_memory}, .task_batches = {&impl.task_memory}, .used_swapchain_task_images = {&impl.task_memory}},
            QueueSubmitScope{.last_minute_barrier_indices = {&impl.task_memory}, .task_batches = {&impl.task_memory}, .used_swapchain_task_images = {&impl.task_memory}},
            QueueSubmitScope{.last_minute_barrier_indices = {&impl.task_memory}, .task_batches = {&impl.task_memory}, .used_swapchain_task_images = {&impl.task_memory}},
            QueueSubmitScope{.last_minute_barrier_indices = {&impl.task_memory}, .task_batches = {&impl.task_memory}, .used_swapchain_task_images = {&impl.task_memory}},
            QueueSubmitScope{.last_minute_barrier_indices = {&impl.task_memory}, .task_batches = {&impl.task_memory}, .used_swapchain_task_images = {&impl.task_memory}},
            QueueSubmitScope{.last_minute_barrier_indices = {&impl.task_memory}, .task_batches = {&impl.task_memory}, .used_swapchain_task_images = {&impl.task_memory}},
            QueueSubmitScope{.last_minute_barrier_indices = {&impl.task_memory}, .task_batches = {&impl.task_memory}, .used_swapchain_task_images = {&impl.task_memory}},
        };
    }
    TaskGraph::~TaskGraph() = default;

    void register_external_buffer_helper(ImplTaskGraph & impl, TaskResourceKind kind, ImplPersistentTaskBufferBlasTlas * external)
    {
        u32 const global_unique_external_index = external->unique_index;
        auto name = impl.task_memory.allocate_copy_string(std::visit([&](auto const & info)
                                                                     { return info.name; }, external->info));

        DAXA_DBG_ASSERT_TRUE_M(!impl.compiled, "completed task graphs can not record new tasks");
        DAXA_DBG_ASSERT_TRUE_M(!impl.name_to_resource_table.contains(name), "Detected duplicate external resource name. All external resources must have a unique name.");
        DAXA_DBG_ASSERT_TRUE_M(!impl.external_idx_to_resource_table.contains(global_unique_external_index), "Detected duplicate registration of external resource. All external resources must only be added to a graph once.");

        external->inc_refcnt();

        ImplTaskResource buffer = {};
        buffer.name = name;
        buffer.kind = kind;
        buffer.external = static_cast<void *>(external);
        buffer.id = {};          // Set in execution preparation.
        buffer.info.buffer = {}; // Set in execution preparation.
        impl.resources.push_back(buffer);
        u32 const index = static_cast<u32>(impl.resources.size()) - 1u;

        impl.name_to_resource_table[name] = std::pair{&impl.resources.back(), index};

        impl.external_idx_to_resource_table[global_unique_external_index] = std::pair{&impl.resources.back(), index};
    }

    void TaskGraph::use_persistent_buffer(TaskBuffer const & buffer)
    {
        auto & impl = *reinterpret_cast<ImplTaskGraph *>(this->object);
        register_external_buffer_helper(impl, TaskResourceKind::BUFFER, buffer.get());
    }

    void TaskGraph::use_persistent_blas(TaskBlas const & blas)
    {
        auto & impl = *reinterpret_cast<ImplTaskGraph *>(this->object);
        register_external_buffer_helper(impl, TaskResourceKind::BLAS, blas.get());
    }

    void TaskGraph::use_persistent_tlas(TaskTlas const & tlas)
    {
        auto & impl = *reinterpret_cast<ImplTaskGraph *>(this->object);
        register_external_buffer_helper(impl, TaskResourceKind::TLAS, tlas.get());
    }

    void TaskGraph::use_persistent_image(TaskImage const & timg)
    {
        auto & impl = *reinterpret_cast<ImplTaskGraph *>(this->object);

        ImplPersistentTaskImage * external = timg.get();
        u32 const global_unique_external_index = external->unique_index;
        auto name = impl.task_memory.allocate_copy_string(external->info.name);

        DAXA_DBG_ASSERT_TRUE_M(!impl.compiled, "completed task graphs can not record new tasks");
        DAXA_DBG_ASSERT_TRUE_M(!impl.name_to_resource_table.contains(name), "Detected duplicate external resource name. All external resources must have a unique name.");
        DAXA_DBG_ASSERT_TRUE_M(!impl.external_idx_to_resource_table.contains(global_unique_external_index), "Detected duplicate registration of external resource. All external resources must only be added to a graph once.");

        external->inc_refcnt();

        ImplTaskResource image = {};
        image.kind = TaskResourceKind::IMAGE;
        image.name = name;
        image.external = static_cast<void *>(external);
        image.id = {};   // Set in execution preparation.
        image.info = {}; // Set in execution preparation.
        impl.resources.push_back(image);
        u32 const index = static_cast<u32>(impl.resources.size()) - 1u;

        impl.name_to_resource_table[name] = std::pair{&impl.resources.back(), index};

        impl.external_idx_to_resource_table[global_unique_external_index] = std::pair{&impl.resources.back(), index};
    }

    auto create_buffer_helper(ImplTaskGraph & impl, TaskResourceKind kind, usize size, std::string_view name)
    {
        DAXA_DBG_ASSERT_TRUE_M(!impl.compiled, "completed task graphs can not record new tasks");
        DAXA_DBG_ASSERT_TRUE_M(!impl.name_to_resource_table.contains(name), "task buffer names must be unique");

        ImplTaskResource buffer = {};
        buffer.name = impl.task_memory.allocate_copy_string(name);
        buffer.kind = kind;
        buffer.external = {};
        buffer.id = {}; // Set in transient resource creation.
        buffer.info.buffer.size = size;

        impl.resources.push_back(buffer);
        u32 const index = static_cast<u32>(impl.resources.size()) - 1u;

        impl.name_to_resource_table[buffer.name] = std::pair{&impl.resources.back(), index};

        return index;
    }

    auto TaskGraph::create_transient_buffer(TaskTransientBufferInfo info) -> TaskBufferView
    {
        auto & impl = *reinterpret_cast<ImplTaskGraph *>(this->object);
        return TaskBufferView{{.task_graph_index = impl.unique_index, .index = create_buffer_helper(impl, TaskResourceKind::BUFFER, info.size, info.name)}};
    }

    auto TaskGraph::create_transient_tlas(TaskTransientTlasInfo info) -> TaskTlasView
    {
        auto & impl = *reinterpret_cast<ImplTaskGraph *>(this->object);
        return TaskTlasView{{.task_graph_index = impl.unique_index, .index = create_buffer_helper(impl, TaskResourceKind::TLAS, info.size, info.name)}};
    }

    auto TaskGraph::create_transient_image(TaskTransientImageInfo info) -> TaskImageView
    {
        auto & impl = *reinterpret_cast<ImplTaskGraph *>(this->object);
        DAXA_DBG_ASSERT_TRUE_M(!impl.compiled, "completed task graphs can not record new tasks");
        DAXA_DBG_ASSERT_TRUE_M(!impl.name_to_resource_table.contains(info.name), "task image names must be unique");

        ImplTaskResource image = {};
        image.name = impl.task_memory.allocate_copy_string(info.name);
        image.kind = TaskResourceKind::IMAGE;
        image.external = {};
        image.id = {}; // Set in transient resource creation.
        image.info = {};
        image.info.image.dimensions = info.dimensions;
        image.info.image.format = info.format;
        image.info.image.size = info.size;
        image.info.image.mip_level_count = info.mip_level_count;
        image.info.image.array_layer_count = info.array_layer_count;
        image.info.image.sample_count = info.sample_count;
        impl.resources.push_back(image);
        u32 const index = static_cast<u32>(impl.resources.size()) - 1u;

        impl.name_to_resource_table[image.name] = std::pair{&impl.resources.back(), index};

        auto task_image_view = TaskImageView{
            .task_graph_index = impl.unique_index,
            .index = index,
            .slice = {
                info.mip_level_count,
                info.array_layer_count,
            },
        };

        return task_image_view;
    }

    auto validate_resource_view_is_owned_by_graph(ImplTaskGraph & impl, auto transient)
    {
        bool is_external = !transient.is_external() && impl.resources[transient.index].external != nullptr;
        DAXA_DBG_ASSERT_TRUE_M(!is_external, "ERROR: TaskGraph can only return task image infos for non external resources!");
        DAXA_DBG_ASSERT_TRUE_M(transient.task_graph_index == impl.unique_index, "ERROR: Given resource view was created by different TaskGraph!");
        DAXA_DBG_ASSERT_TRUE_M(transient.index < impl.resources.size(), "ERROR: Given resource view is invalid!");
    }

    DAXA_EXPORT_CXX auto TaskGraph::transient_buffer_info(TaskBufferView const & transient) -> TaskTransientBufferInfo
    {
        ImplTaskGraph & impl = *reinterpret_cast<ImplTaskGraph *>(this->object);
        validate_resource_view_is_owned_by_graph(impl, transient);

        return TaskTransientBufferInfo{
            .size = impl.resources[transient.index].info.buffer.size,
            .name = impl.resources[transient.index].name,
        };
    }

    DAXA_EXPORT_CXX auto TaskGraph::transient_tlas_info(TaskTlasView const & transient) -> TaskTransientTlasInfo
    {
        ImplTaskGraph & impl = *reinterpret_cast<ImplTaskGraph *>(this->object);
        validate_resource_view_is_owned_by_graph(impl, transient);

        return TaskTransientTlasInfo{
            .size = impl.resources[transient.index].info.buffer.size,
            .name = impl.resources[transient.index].name,
        };
    }

    DAXA_EXPORT_CXX auto TaskGraph::transient_image_info(TaskImageView const & transient) -> TaskTransientImageInfo
    {
        ImplTaskGraph & impl = *reinterpret_cast<ImplTaskGraph *>(this->object);
        validate_resource_view_is_owned_by_graph(impl, transient);

        auto const & resource_image_info = impl.resources[transient.index].info.image;
        return TaskTransientImageInfo{
            .temporal = false,
            .dimensions = resource_image_info.dimensions,
            .format = resource_image_info.format,
            .size = resource_image_info.size,
            .mip_level_count = resource_image_info.mip_level_count,
            .array_layer_count = resource_image_info.array_layer_count,
            .sample_count = resource_image_info.sample_count,
            .name = impl.resources[transient.index].name,
        };
    }

    DAXA_EXPORT_CXX void TaskGraph::clear_buffer(TaskBufferClearInfo const & info)
    {
        ImplTaskGraph & impl = *reinterpret_cast<ImplTaskGraph *>(this->object);

        auto const view = validate_and_translate_view(impl, info.buffer);

        auto name = info.name.size() > 0 ? std::string(info.name) : std::string("clear buffer: ") + std::string(impl.resources[view.index].name);

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

        auto const view = validate_and_translate_view(impl, info.view);

        auto name = info.name.size() > 0 ? std::string(info.name) : std::string("clear image: ") + std::string(impl.resources[view.index].name);

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
        auto src = validate_and_translate_view(impl, info.src);
        auto dst = validate_and_translate_view(impl, info.dst);

        auto src_i = TaskBufferAttachmentIndex{0};
        auto dst_i = TaskBufferAttachmentIndex{1};

        auto name = info.name.size() > 0 ? std::string(info.name) : std::string("copy ") + std::string(impl.resources[src.index].name) + " to " + std::string(impl.resources[dst.index].name);

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
        auto src = validate_and_translate_view(impl, info.src);
        auto dst = validate_and_translate_view(impl, info.dst);

        auto src_i = TaskImageAttachmentIndex{0};
        auto dst_i = TaskImageAttachmentIndex{1};

        auto name = info.name.size() > 0 ? std::string(info.name) : std::string("copy ") + std::string(impl.resources[src.index].name) + " to " + std::string(impl.resources[dst.index].name);

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

    auto initialize_attachment_ids(ImplTaskGraph & impl, ImplTask & task)
    {
        for (u32 attach_i = 0; attach_i < task.attachments.size(); ++attach_i)
        {
            TaskAttachmentInfo & attachment_info = task.attachments[attach_i];
            switch (attachment_info.type)
            {
            case TaskAttachmentType::UNDEFINED:
                DAXA_DBG_ASSERT_TRUE_M(false, "IMPOSSIBLE CASE, POSSIBLY CAUSED BY DATA CORRUPTION!");
                break;
            case TaskAttachmentType::BUFFER:
            {
                ImplTaskResource const & resource = impl.resources[attachment_info.value.buffer.translated_view.index];
                attachment_info.value.buffer.ids = std::span{&resource.id.buffer, 1ull};
            }
            break;
            case TaskAttachmentType::TLAS:
            {
                ImplTaskResource const & resource = impl.resources[attachment_info.value.tlas.translated_view.index];
                attachment_info.value.tlas.ids = std::span{&resource.id.tlas, 1ull};
            }
            break;
            case TaskAttachmentType::BLAS:
            {
                ImplTaskResource const & resource = impl.resources[attachment_info.value.blas.translated_view.index];
                attachment_info.value.blas.ids = std::span{&resource.id.blas, 1ull};
            }
            break;
            case TaskAttachmentType::IMAGE:
            {
                ImplTaskResource const & resource = impl.resources[attachment_info.value.image.translated_view.index];
                attachment_info.value.image.ids = std::span{&resource.id.image, 1ull};
            }
            break;
            }
        }
    }

    auto initialize_attachment_image_views(ImplTaskGraph & impl, ImplTask & task)
    {
        for (u32 attach_i = 0; attach_i < task.attachments.size(); ++attach_i)
        {
            TaskAttachmentInfo & attachment_info = task.attachments[attach_i];
            if (attachment_info.type == TaskAttachmentType::IMAGE && !attachment_info.value.image.translated_view.is_null())
            {
                ImplTaskResource const & resource = impl.resources[attachment_info.value.image.translated_view.index];

                if (attachment_info.value.image.shader_array_size == 0)
                {
                    continue;
                }

                if (attachment_info.value.image.shader_array_type == TaskHeadImageArrayType::MIP_LEVELS)
                {
                    for (u32 mip = 0; mip < attachment_info.value.image.shader_array_size; ++mip)
                    {
                        ImageViewId view = {};
                        if (resource.external == nullptr && mip < attachment_info.value.image.translated_view.slice.level_count)
                        {
                            ImageId id = resource.id.image;
                            view = id.default_view();
                            ImageMipArraySlice mip_slice = attachment_info.value.image.translated_view.slice;
                            mip_slice.base_mip_level = mip + attachment_info.value.image.translated_view.slice.base_mip_level;
                            mip_slice.level_count = 1u;
                            auto default_view_info = impl.info.device.image_view_info(id.default_view()).value();
                            auto attachment_view_type = attachment_info.value.image.view_type != ImageViewType::MAX_ENUM ? attachment_info.value.image.view_type : default_view_info.type;
                            auto identical_type = default_view_info.type == attachment_view_type;
                            auto identical_slice = default_view_info.slice == mip_slice;
                            if (!identical_type || !identical_slice)
                            {
                                auto new_view_info = impl.info.device.image_view_info(id.default_view()).value();
                                new_view_info.type = attachment_view_type;
                                new_view_info.slice = mip_slice;
                                view = impl.info.device.create_image_view(new_view_info);
                            }
                        }
                        task.attachment_image_views[attach_i][mip] = view;
                    }
                }
                else
                {
                    ImageViewId view = {};
                    if (resource.external == nullptr)
                    {
                        ImageId id = resource.id.image;
                        view = id.default_view();
                        auto default_view_info = impl.info.device.image_view_info(id.default_view()).value();
                        auto attachment_view_type = attachment_info.value.image.view_type != ImageViewType::MAX_ENUM ? attachment_info.value.image.view_type : default_view_info.type;
                        auto identical_type = default_view_info.type == attachment_view_type;
                        auto identical_slice = default_view_info.slice == attachment_info.value.image.translated_view.slice;
                        if (!identical_type || !identical_slice)
                        {
                            auto new_view_info = impl.info.device.image_view_info(id.default_view()).value();
                            new_view_info.type = attachment_view_type;
                            new_view_info.slice = attachment_info.value.image.translated_view.slice;
                            view = impl.info.device.create_image_view(new_view_info);
                        }
                    }
                    task.attachment_image_views[attach_i][0] = view;
                }
            }
        }
    }

    auto initialize_attachment_shader_blob(ImplTaskGraph & impl, ImplTask & task)
    {
        u64 current_offset = {};
        for (u32 attach_i = 0; attach_i < task.attachments.size(); ++attach_i)
        {
            TaskAttachmentInfo const & attachment_info = task.attachments[attach_i];
            u64 blob_offset = {};
            u64 blob_size = {};
            switch (attachment_info.type)
            {
            case TaskAttachmentType::UNDEFINED:
                DAXA_DBG_ASSERT_TRUE_M(false, "IMPOSSIBLE CASE, POSSIBLY CAUSED BY DATA CORRUPTION!");
                break;
            case TaskAttachmentType::BUFFER:
            {
                if (attachment_info.value.buffer.shader_array_size == 0 ||
                    attachment_info.value.buffer.translated_view.is_null())
                {
                    break;
                }
                ImplTaskResource const & resource = impl.resources[attachment_info.value.buffer.translated_view.index];
                if (attachment_info.value.buffer.shader_as_address)
                {
                    blob_offset = align_up(current_offset, sizeof(DeviceAddress));
                    blob_size = sizeof(DeviceAddress);

                    DeviceAddress address = {};
                    if (resource.external == nullptr)
                    {
                        address = impl.info.device.device_address(resource.id.buffer).value();
                    }
                    *reinterpret_cast<DeviceAddress *>(task.attachment_shader_blob.data() + blob_offset) = address;
                }
                else
                {
                    blob_offset = align_up(current_offset, sizeof(BufferId));
                    blob_size = sizeof(BufferId);

                    BufferId id = {};
                    if (resource.external == nullptr)
                    {
                        id = resource.id.buffer;
                    }
                    *reinterpret_cast<BufferId *>(task.attachment_shader_blob.data() + blob_offset) = id;
                }
            }
            break;
            case TaskAttachmentType::TLAS:
            {
                if (attachment_info.value.tlas.shader_array_size == 0 ||
                    attachment_info.value.tlas.translated_view.is_null())
                {
                    break;
                }
                ImplTaskResource const & resource = impl.resources[attachment_info.value.tlas.translated_view.index];
                blob_offset = align_up(current_offset, sizeof(TlasId));
                blob_size = sizeof(TlasId);

                TlasId id = {};
                if (resource.external == nullptr)
                {
                    id = resource.id.tlas;
                }
                *reinterpret_cast<TlasId *>(task.attachment_shader_blob.data() + blob_offset) = id;
            }
            break;
            case TaskAttachmentType::BLAS:
                // BLAS can not be in the attachment blob
                break;
            case TaskAttachmentType::IMAGE:
            {
                if (attachment_info.value.image.shader_array_size == 0 ||
                    attachment_info.value.image.translated_view.is_null())
                {
                    break;
                }
                if (attachment_info.value.image.shader_as_index)
                {
                    blob_offset = align_up(current_offset, sizeof(ImageViewIndex));
                    blob_size = sizeof(ImageViewIndex) * attachment_info.value.image.shader_array_size;
                }
                else
                {
                    blob_offset = align_up(current_offset, sizeof(ImageViewId));
                    blob_size = sizeof(ImageViewId) * attachment_info.value.image.shader_array_size;
                }
                for (u32 i = 0; i < attachment_info.value.image.shader_array_size; ++i)
                {
                    ImageViewId id = task.attachment_image_views[attach_i][i];
                    if (attachment_info.value.image.shader_as_index)
                    {
                        *reinterpret_cast<ImageViewIndex *>(task.attachment_shader_blob.data() + blob_offset + sizeof(ImageViewIndex) * i) = static_cast<ImageViewIndex>(id.index);
                    }
                    else
                    {
                        *reinterpret_cast<ImageViewId *>(task.attachment_shader_blob.data() + blob_offset + sizeof(ImageViewId) * i) = id;
                    }
                }
            }
            break;
            }

            task.attachment_in_blob_offsets[attach_i] = static_cast<u32>(blob_offset);
            current_offset = blob_offset + blob_size;
        }
    }

    auto patch_attachment_image_views(ImplTaskGraph & impl, ImplTask & task, u32 attach_i, ImplTaskResource & resource)
    {
        TaskAttachmentInfo const & attachment_info = task.attachments[attach_i];
        if (attachment_info.type != TaskAttachmentType::IMAGE)
        {
            return;
        }

        DAXA_DBG_ASSERT_TRUE_M(!attachment_info.value.image.translated_view.is_null(), "IMPOSSIBLE CASE, WE SHOULD NEVER TRY TO PATCH NULL ATTACHMENTS!");
        DAXA_DBG_ASSERT_TRUE_M(resource.external != nullptr, "IMPOSSIBLE CASE, WE SHOULD NEVER TRY TO PATCH NON EXTERNAL RESOURCE ATTACHMENTS!");

        if (attachment_info.value.image.shader_array_size == 0)
        {
            return;
        }

        if (attachment_info.value.image.shader_array_type == TaskHeadImageArrayType::MIP_LEVELS)
        {
            for (u32 mip = 0; mip < attachment_info.value.image.shader_array_size; ++mip)
            {
                ImageViewId & view = task.attachment_image_views[attach_i][mip];

                if (!view.is_empty())
                {
                    ImageId parent_image = impl.info.device.image_view_info(view).value().image;
                    ImageViewId default_view = parent_image.default_view();
                    bool is_default_view = default_view == view;
                    if (!is_default_view)
                    {
                        impl.info.device.destroy_image_view(view);
                    }
                }

                view = {};

                if (mip < attachment_info.value.image.translated_view.slice.level_count)
                {
                    ImageId id = resource.id.image;
                    view = id.default_view();
                    ImageMipArraySlice mip_slice = attachment_info.value.image.translated_view.slice;
                    mip_slice.base_mip_level = mip + attachment_info.value.image.translated_view.slice.base_mip_level;
                    mip_slice.level_count = 1u;
                    auto default_view_info = impl.info.device.image_view_info(id.default_view()).value();
                    auto attachment_view_type = attachment_info.value.image.view_type != ImageViewType::MAX_ENUM ? attachment_info.value.image.view_type : default_view_info.type;
                    auto identical_type = default_view_info.type == attachment_view_type;
                    auto identical_slice = default_view_info.slice == mip_slice;
                    if (!identical_type || !identical_slice)
                    {
                        auto new_view_info = impl.info.device.image_view_info(id.default_view()).value();
                        new_view_info.type = attachment_view_type;
                        new_view_info.slice = mip_slice;
                        view = impl.info.device.create_image_view(new_view_info);
                    }
                }
            }
        }
        else
        {
            ImageViewId & view = task.attachment_image_views[attach_i][0];

            if (!view.is_empty())
            {
                ImageId parent_image = impl.info.device.image_view_info(view).value().image;
                ImageViewId default_view = parent_image.default_view();
                bool is_default_view = default_view == view;
                if (!is_default_view)
                {
                    impl.info.device.destroy_image_view(view);
                }
            }

            ImageId id = resource.id.image;
            view = id.default_view();
            auto default_view_info = impl.info.device.image_view_info(id.default_view()).value();
            auto attachment_view_type = attachment_info.value.image.view_type != ImageViewType::MAX_ENUM ? attachment_info.value.image.view_type : default_view_info.type;
            auto identical_type = default_view_info.type == attachment_view_type;
            auto identical_slice = default_view_info.slice == attachment_info.value.image.translated_view.slice;
            if (!identical_type || !identical_slice)
            {
                auto new_view_info = impl.info.device.image_view_info(id.default_view()).value();
                new_view_info.type = attachment_view_type;
                new_view_info.slice = attachment_info.value.image.translated_view.slice;
                view = impl.info.device.create_image_view(new_view_info);
            }
        }
    }

    auto patch_attachment_shader_blob(ImplTaskGraph & impl, ImplTask & task, u32 attach_i, ImplTaskResource & resource)
    {
        DAXA_DBG_ASSERT_TRUE_M(resource.external != nullptr, "IMPOSSIBLE CASE, PATCHING IS ONLY POSSIBLE FOR EXTERNAL RESOURCES!");
        TaskAttachmentInfo const & attachment_info = task.attachments[attach_i];
        u64 blob_offset = task.attachment_in_blob_offsets[attach_i];
        switch (attachment_info.type)
        {
        case TaskAttachmentType::UNDEFINED:
            DAXA_DBG_ASSERT_TRUE_M(false, "IMPOSSIBLE CASE, POSSIBLY CAUSED BY DATA CORRUPTION!");
            break;
        case TaskAttachmentType::BUFFER:
        {
            if (attachment_info.value.buffer.shader_array_size == 0)
            {
                break;
            }
            if (attachment_info.value.buffer.shader_as_address)
            {
                DeviceAddress address = impl.info.device.device_address(resource.id.buffer).value();
                *reinterpret_cast<DeviceAddress *>(task.attachment_shader_blob.data() + blob_offset) = address;
            }
            else
            {
                *reinterpret_cast<BufferId *>(task.attachment_shader_blob.data() + blob_offset) = resource.id.buffer;
            }
        }
        break;
        case TaskAttachmentType::TLAS:
        {
            if (attachment_info.value.tlas.shader_array_size == 0)
            {
                break;
            }
            *reinterpret_cast<TlasId *>(task.attachment_shader_blob.data() + blob_offset) = resource.id.tlas;
        }
        break;
        case TaskAttachmentType::BLAS:
            // BLAS can not be in the attachment blob
            break;
        case TaskAttachmentType::IMAGE:
        {
            for (u32 i = 0; i < attachment_info.value.image.shader_array_size; ++i)
            {
                ImageViewId id = task.attachment_image_views[attach_i][i];
                if (attachment_info.value.image.shader_as_index)
                {
                    *reinterpret_cast<ImageViewIndex *>(task.attachment_shader_blob.data() + blob_offset + sizeof(ImageViewIndex) * i) = static_cast<ImageViewIndex>(id.index);
                }
                else
                {
                    *reinterpret_cast<ImageViewId *>(task.attachment_shader_blob.data() + blob_offset + sizeof(ImageViewId) * i) = id;
                }
            }
        }
        break;
        }
    }

    void execute_task(ImplTaskRuntimeInterface & impl_runtime, usize batch_index, TaskBatchId in_batch_task_index, TaskId task_id, Queue queue)
    {
    }

    void TaskGraph::conditional(TaskGraphConditionalInfo const & conditional_info)
    {
    }

    auto TaskGraph::allocate_task_memory(usize size, usize align) -> void *
    {
        auto & impl = *reinterpret_cast<ImplTaskGraph *>(this->object);
        auto task_memory = impl.task_memory.allocate(size, align);
        DAXA_DBG_ASSERT_TRUE_M(task_memory != nullptr, "TaskGraph ran out of task memory, please increase the task memory pool");
        return task_memory;
    }

    void TaskGraph::add_task(
        void (*task_callback)(daxa::TaskInterface, void *),
        u64 * task_callback_memory,
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

        std::span<std::span<ImageViewId>> attachment_image_views = impl.task_memory.allocate_trivial_span<std::span<ImageViewId>>(attachments.size());
        for (u32 attach_i = 0u; attach_i < attachments.size(); ++attach_i)
        {
            if (attachments[attach_i].type == TaskAttachmentType::IMAGE)
            {
                TaskImageAttachmentInfo const & attachment_info = attachments[attach_i].value.image;
                attachment_image_views[attach_i] = impl.task_memory.allocate_trivial_span<ImageViewId>(attachment_info.shader_array_size);
            }
            else
            {
                attachment_image_views[attach_i] = std::span<ImageViewId>{};
            }
        }

        ImplTask impl_task = ImplTask{
            .name = impl.task_memory.allocate_copy_string(name),
            .task_callback = task_callback,
            .task_callback_memory = task_callback_memory,
            .attachments = attachments,
            .attachment_access_groups = impl.task_memory.allocate_trivial_span<AccessGroup *>(attachments.size()),
            .attachment_shader_blob_size = attachment_shader_blob_size,
            .attachment_shader_blob_alignment = attachment_shader_blob_alignment,
            .attachment_shader_blob = impl.task_memory.allocate_trivial_span<u8>(attachment_shader_blob_size, attachment_shader_blob_alignment),
            .attachment_in_blob_offsets = impl.task_memory.allocate_trivial_span<u32>(attachments.size()),
            .attachment_image_views = attachment_image_views,
            .task_type = task_type,
            .queue = queue,
            .submit_index = static_cast<u32>(impl.submits.size()),
            .runtime_images_last_execution = impl.task_memory.allocate_trivial_span_fill<ImageId>(attachments.size(), daxa::ImageId{}),
        };

        /// TODOTG: validate attachment access'es and stage's are valid for task and queue type
        /// TODOTG: validate that no resource is used by multiple attachments in the same task

        // Translate external to native ids
        {
            for (TaskAttachmentInfo & attachment : impl_task.attachments)
            {
                switch (attachment.type)
                {
                case TaskAttachmentType::BUFFER:
                    attachment.value.buffer.translated_view = validate_and_translate_view(impl, attachment.value.buffer.view);
                    break;
                case TaskAttachmentType::BLAS:
                    attachment.value.blas.translated_view = validate_and_translate_view(impl, attachment.value.blas.view);
                    break;
                case TaskAttachmentType::TLAS:
                    attachment.value.tlas.translated_view = validate_and_translate_view(impl, attachment.value.tlas.view);
                    break;
                case TaskAttachmentType::IMAGE:
                    attachment.value.image.translated_view = validate_and_translate_view(impl, attachment.value.image.view);
                    break;
                default:
                    DAXA_DBG_ASSERT_TRUE_M(false, "IMPOSSIBLE CASE, STRONG LIKELYHOOD OF UNINITIALIZED DATA OR CORRUPTION!");
                }
            }
        }

        impl.tasks.push_back(impl_task);
    }

    void TaskGraph::submit([[maybe_unused]] TaskSubmitInfo const & info)
    {
        auto & impl = *reinterpret_cast<ImplTaskGraph *>(this->object);
        validate_not_compiled(impl);

        u32 const tasks_since_last_submit = static_cast<u32>(
            impl.submits.size() == 0 ? impl.tasks.size() : impl.tasks.size() - (impl.submits.back().first_task + impl.submits.back().task_count));

        impl.submits.push_back(TasksSubmit{
            .first_task = static_cast<u32>(impl.tasks.size()),
            .task_count = tasks_since_last_submit,
            .used_queues_bitfield = 0u,
        });
    }

    void TaskGraph::present(TaskPresentInfo const & info)
    {
    }

    void TaskGraph::complete(TaskCompleteInfo const & /*unused*/)
    {
        ImplTaskGraph & impl = *r_cast<ImplTaskGraph *>(this->object);
        u32 required_tmp_size = 1u << 23u; /* 8MB */
        MemoryArena tmp_memory = MemoryArena{"TaskGraph::complete tmp memory", required_tmp_size};

        /// ================================
        /// ==== EARLY INPUT VALIDATION ====
        /// ================================

        DAXA_DBG_ASSERT_TRUE_M(impl.submits.size() > 0, "ERROR: All task graphs must have at least one submission!");
        DAXA_DBG_ASSERT_TRUE_M(!impl.compiled, "ERROR: TaskGraph was already completed!");

        // Validate that all attachments of each task refer to unique resources.
        // Vaidate that no TaskImageView slice goes out of bounds.
        for (u32 task_i = 0; task_i < impl.tasks.size(); ++task_i)
        {
            ImplTask & task = impl.tasks.at(task_i);
            for (u32 attach_i = 0; attach_i < task.attachments.size(); ++attach_i)
            {
                TaskAttachmentInfo const & attachment = task.attachments[attach_i];

                // Vaidate that no TaskImageView slice goes out of bounds.
                // if (attachment.type == TaskAttachmentType::IMAGE)
                // {
                //
                // }

                // Validate that all attachments of each task refer to unique resources.
                for (u32 other_i = attach_i + 1; other_i < task.attachments.size(); ++other_i)
                {
                    TaskAttachmentInfo const & other_attachment = task.attachments[other_i];

                    if (attachment.type == other_attachment.type)
                    {
                        u32 resource_index = {};
                        u32 other_resource_index = {};
                        char const * attachment_name = {};
                        char const * other_attachment_name = {};
                        if (attachment.type != TaskAttachmentType::IMAGE)
                        {
                            // buffer, blas, tlas attach infos are identical memory layout :)
                            resource_index = attachment.value.buffer.translated_view.index;
                            other_resource_index = other_attachment.value.buffer.translated_view.index;
                            attachment_name = attachment.value.buffer.name;
                            other_attachment_name = other_attachment.value.buffer.name;
                        }
                        else
                        {
                            resource_index = attachment.value.image.translated_view.index;
                            other_resource_index = other_attachment.value.image.translated_view.index;
                            attachment_name = attachment.value.image.name;
                            other_attachment_name = other_attachment.value.image.name;
                        }

                        bool const either_resource_null = resource_index == ~0u || other_resource_index == ~0u;
                        DAXA_DBG_ASSERT_TRUE_M(
                            resource_index != other_resource_index || either_resource_null,
                            std::format(
                                "ERROR: All attachments must have different task resources assigned to them! "
                                "Detected error in task \"{}\" (index: {}), attachment \"{}\" (index: {}) and attachment \"{}\" (index: {}) refer to the same resource \"{}\" (index: {}).",
                                task.name, task_i, attachment_name, attach_i, other_attachment_name, other_i, impl.resources[resource_index].name, resource_index)
                                .c_str()

                        );
                    }
                }
            }
        }

        /// =========================================
        /// ==== BUILD RESOURCE ACCESS TIMELINES ====
        /// =========================================

        struct TmpAccessGroup
        {
            PipelineStageFlags used_stages = {};
            TaskAccessType type = {};
            u32 used_queues_bitfield = {};
            ArenaDynamicArray8k<TaskAttachmentAccess> tasks = {};
        };
        auto tmp_resource_access_timelines = tmp_memory.allocate_trivial_span_fill<ArenaDynamicArray8k<TmpAccessGroup>>(impl.resources.size(), ArenaDynamicArray8k<TmpAccessGroup>(&tmp_memory));
        auto tmp_resource_latest_submit_access = tmp_memory.allocate_trivial_span_fill(impl.resources.size(), 0u);

        // Build access timelines in tmp allocations
        for (u32 task_i = 0; task_i < impl.tasks.size(); ++task_i)
        {
            ImplTask & task = impl.tasks.at(task_i);
            for (u32 attach_i = 0; attach_i < task.attachments.size(); ++attach_i)
            {
                TaskAttachmentInfo const & attachment = task.attachments[attach_i];

                task.attachment_access_groups[attach_i] = nullptr; // Zero init attachment_access_groups. Only non-null attachments are assigned access groups later.

                TaskAccessType access_type = {};
                ArenaDynamicArray8k<TmpAccessGroup> * access_timeline = nullptr;
                u32 * latest_access_submit_index = nullptr;
                PipelineStageFlags attachment_stages = {};
                ImplTaskResource * resource = nullptr;
                if (attachment.type != TaskAttachmentType::IMAGE)
                {
                    // buffer, blas, tlas attach infos are identical memory layout :)
                    access_type = attachment.value.buffer.task_access.type;
                    if (!attachment.value.buffer.translated_view.is_null())
                    {
                        u32 const resource_index = attachment.value.buffer.translated_view.index;
                        access_timeline = &tmp_resource_access_timelines[resource_index];
                        latest_access_submit_index = &tmp_resource_latest_submit_access[resource_index];
                        attachment_stages = task_stage_to_pipeline_stage(attachment.value.buffer.task_access.stage);
                        resource = &impl.resources[resource_index];
                    }
                }
                else
                {
                    access_type = attachment.value.image.task_access.type;
                    if (!attachment.value.image.translated_view.is_null())
                    {
                        u32 const resource_index = attachment.value.image.translated_view.index;
                        access_timeline = &tmp_resource_access_timelines[resource_index];
                        latest_access_submit_index = &tmp_resource_latest_submit_access[resource_index];
                        impl.resources[resource_index].info.image.usage |= access_to_image_usage(attachment.value.image.task_access);
                        if (task.queue != daxa::QUEUE_MAIN)
                        {
                            impl.resources[resource_index].info.image.sharing_mode = SharingMode::CONCURRENT;
                        }
                        if (attachment.value.image.view_type == ImageViewType::CUBE)
                        {
                            impl.resources[resource_index].info.image.flags |= ImageCreateFlagBits::COMPATIBLE_CUBE;
                        }
                        if (attachment.value.image.view_type == ImageViewType::REGULAR_2D_ARRAY && impl.resources[resource_index].info.image.dimensions == 3u)
                        {
                            impl.resources[resource_index].info.image.flags |= ImageCreateFlagBits::COMPATIBLE_2D_ARRAY;
                        }
                        attachment_stages = task_stage_to_pipeline_stage(attachment.value.image.task_access.stage);
                        resource = &impl.resources[resource_index];
                    }
                }

                if (access_timeline != nullptr && latest_access_submit_index != nullptr)
                {
                    bool const append_new_group =
                        access_timeline->size() == 0 ||
                        access_timeline->back().type != access_type ||
                        (static_cast<u8>(access_type) & static_cast<u8>(TaskAccessType::CONCURRENT_BIT)) == 0 ||
                        *latest_access_submit_index != task.submit_index;
                    if (append_new_group)
                    {
                        access_timeline->push_back(TmpAccessGroup{
                            .used_stages = {},
                            .type = access_type,
                            .used_queues_bitfield = 0u,
                            .tasks = ArenaDynamicArray8k<TaskAttachmentAccess>(&tmp_memory),
                        });
                    }
                    access_timeline->back().tasks.push_back(TaskAttachmentAccess{&task, task_i, attach_i});
                    access_timeline->back().used_queues_bitfield |= (1u << flat_queue_index(task.queue));
                    access_timeline->back().used_stages |= attachment_stages;
                    *latest_access_submit_index = task.submit_index;
                    resource->used_queues_bitfield |= (1u << flat_queue_index(task.queue));
                }
            }
        }

        // Reallocate access timelines into task memory
        for (u32 i = 0; i < tmp_resource_access_timelines.size(); ++i)
        {
            ArenaDynamicArray8k<TmpAccessGroup> const & src_access_timelines = tmp_resource_access_timelines[i];
            std::span<AccessGroup> & dst_access_timeline = impl.resources[i].access_timeline;

            dst_access_timeline = impl.task_memory.allocate_trivial_span<AccessGroup>(src_access_timelines.size());
            for (u32 ati = 0; ati < src_access_timelines.size(); ++ati)
            {
                AccessGroup & dst_access_group = dst_access_timeline[ati];
                dst_access_group = {};
                dst_access_group.used_stages = src_access_timelines.at(ati).used_stages;
                dst_access_group.type = src_access_timelines.at(ati).type;
                dst_access_group.used_queues_bitfield = src_access_timelines.at(ati).used_queues_bitfield;
                dst_access_group.tasks = impl.task_memory.allocate_trivial_span<TaskAttachmentAccess>(src_access_timelines.at(ati).tasks.size());
                for (u32 t = 0; t < src_access_timelines.at(ati).tasks.size(); ++t)
                {
                    dst_access_group.tasks[t] = src_access_timelines.at(ati).tasks.at(t);
                    TaskAttachmentAccess & taa = dst_access_group.tasks[t];
                    taa.task->attachment_access_groups[taa.attachment_index] = &dst_access_group;
                }
            }
        }

        /// =====================================
        /// ==== VALIDATE MULTI QUEUE ACCESS ====
        /// =====================================

        for (u32 i = 0; i < impl.resources.size(); ++i)
        {
            std::span<AccessGroup> & access_timeline = impl.resources[i].access_timeline;

            u32 current_submit_index = ~0u;
            u32 current_submit_queue_bitfield = {};
            bool current_submit_multi_queue_allowed = {};
            TaskAccessType current_submit_first_access_type = {};

            for (u32 ati = 0; ati < access_timeline.size(); ++ati)
            {
                AccessGroup const & access_group = access_timeline[ati];

                DAXA_DBG_ASSERT_TRUE_M(access_group.tasks.size() > 0, "Should be impossible, check logic in Build access timelines loop when adding groups");
                if (current_submit_index != access_group.tasks[0].task->submit_index)
                {
                    current_submit_index = access_group.tasks[0].task->submit_index;
                    current_submit_queue_bitfield = 0u;
                    current_submit_multi_queue_allowed = true;
                    current_submit_first_access_type = access_timeline.back().type;
                }

                current_submit_queue_bitfield |= access_group.used_queues_bitfield;
                current_submit_multi_queue_allowed =
                    current_submit_multi_queue_allowed &&
                    current_submit_first_access_type == access_group.type &&
                    (static_cast<u8>(access_group.type) & static_cast<u8>(TaskAccessType::CONCURRENT_BIT)) != 0;

                if (!current_submit_multi_queue_allowed && std::popcount(current_submit_queue_bitfield) > 1)
                {
                    std::string queue_string = {};

                    u32 queue_iter = current_submit_queue_bitfield;
                    while (queue_iter != 0)
                    {
                        u32 const first_significant_bit_idx = 31u - static_cast<u32>(std::countl_zero(queue_iter));
                        queue_iter &= ~(1u << first_significant_bit_idx);

                        queue_string += std::string(daxa::to_string(flat_index_to_queue(first_significant_bit_idx)));
                        if (queue_iter != 0)
                        {
                            queue_string += ", ";
                        }
                    }
                    DAXA_DBG_ASSERT_TRUE_M(
                        false,
                        std::format("Illegal multi-queue resource access! Resource {} is accessed in multiple queues {} with multiple access types. "
                                    "All access to a resource used on multiple queues (within one submit) must be identical and concurrent.",
                                    impl.resources[i].name,
                                    queue_string)
                            .c_str());
                }
            }
        }

        /// ==============================================
        /// ==== DETERMINE MIN SCHEUDLE FOR ALL TASKS ====
        /// ==============================================

        // In the Min-Schedule, all tasks are inserted into batches as early as possible.
        // This is not optimal, but it allows us to quickly find a schedule with the mimimal possible number of batches.
        // Once we have the Min-Schedule, we can continue optimizing the order of tasks that are not in the critical path.

        auto tmp_minsh_task_batches = ArenaDynamicArray8k<ArenaDynamicArray8k<std::pair<ImplTask *, u32>>>(&tmp_memory);

        struct TmpLatestAccessGroup
        {
            AccessGroup const * access_group = nullptr;
            u32 batch_index = {};
        };
        auto tmp_minsh_resource_latest_access_groups = tmp_memory.allocate_trivial_span_fill(impl.resources.size(), TmpLatestAccessGroup{});

        u32 latest_submit_index = 0u;
        u32 first_batch_after_latest_submit = 0u;
        for (u32 task_i = 0; task_i < impl.tasks.size(); ++task_i)
        {
            ImplTask & task = impl.tasks.at(task_i);

            if (task.submit_index != latest_submit_index)
            {
                tmp_minsh_task_batches.push_back(ArenaDynamicArray8k<std::pair<ImplTask *, u32>>(&tmp_memory));
                first_batch_after_latest_submit = static_cast<u32>(tmp_minsh_task_batches.size()) - 1u;
            }
            latest_submit_index = task.submit_index;

            // Find min batch index
            u32 min_batch_index = first_batch_after_latest_submit;
            for (u32 attach_i = 0; attach_i < task.attachments.size(); ++attach_i)
            {
                TaskAttachmentInfo const & attachment = task.attachments[attach_i];
                AccessGroup const * access_group = task.attachment_access_groups[attach_i];

                TmpLatestAccessGroup * tmp_latest_access_group = nullptr;
                if (attachment.type != TaskAttachmentType::IMAGE)
                {
                    // buffer, blas, tlas attach infos are identical memory layout :)
                    if (!attachment.value.buffer.translated_view.is_null())
                    {
                        tmp_latest_access_group = &tmp_minsh_resource_latest_access_groups[attachment.value.buffer.translated_view.index];
                    }
                }
                else
                {
                    if (!attachment.value.image.translated_view.is_null())
                    {
                        tmp_latest_access_group = &tmp_minsh_resource_latest_access_groups[attachment.value.image.translated_view.index];
                    }
                }

                if (tmp_latest_access_group != nullptr && tmp_latest_access_group->access_group != nullptr)
                {
                    if (tmp_latest_access_group->access_group == access_group)
                    {
                        min_batch_index = std::max(min_batch_index, tmp_latest_access_group->batch_index);
                    }
                    else
                    {
                        min_batch_index = std::max(min_batch_index, tmp_latest_access_group->batch_index + 1);
                    }
                }
            }

            // Update tmp latest access groups
            for (u32 attach_i = 0; attach_i < task.attachments.size(); ++attach_i)
            {
                TaskAttachmentInfo const & attachment = task.attachments[attach_i];
                AccessGroup const * access_group = task.attachment_access_groups[attach_i];

                TmpLatestAccessGroup * tmp_latest_access_group = nullptr;
                if (attachment.type != TaskAttachmentType::IMAGE)
                {
                    // buffer, blas, tlas attach infos are identical memory layout :)
                    if (!attachment.value.buffer.translated_view.is_null())
                    {
                        tmp_latest_access_group = &tmp_minsh_resource_latest_access_groups[attachment.value.buffer.translated_view.index];
                    }
                }
                else
                {
                    if (!attachment.value.image.translated_view.is_null())
                    {
                        tmp_latest_access_group = &tmp_minsh_resource_latest_access_groups[attachment.value.image.translated_view.index];
                    }
                }

                if (tmp_latest_access_group != nullptr)
                {
                    tmp_latest_access_group->access_group = access_group;
                    tmp_latest_access_group->batch_index = min_batch_index;
                }
            }

            // Update batches
            if (min_batch_index >= tmp_minsh_task_batches.size())
            {
                tmp_minsh_task_batches.push_back(ArenaDynamicArray8k<std::pair<ImplTask *, u32>>(&tmp_memory));
            }
            tmp_minsh_task_batches.at(min_batch_index).push_back(std::pair{&task, task_i});
        }

        /// ===============================
        /// ==== COMPACT TASKS FORWARD ====
        /// ===============================

        // With the min scheudle, all first resource access tasks will be as early as possible.
        // This causes transient resources to have unnecessarily long lifetimes, as all their clears are happening earlier than they have to.
        // In this pass, we move all tasks as much forward as possible WITHOUT adding new batches or increasing the critical path.
        // After this pass, transient resource lifetimes are minimized.

        /// ======================================
        /// ==== COMPACT TASKS WITHIN BATCHES ====
        /// ======================================

        // Some gpus, such as all nvidia gpus before the BLACKWELL architecture have to perform a subchannel switch (full barrier) when switching between compute/gfx/transfer work.
        // In this pass we reroder all tasks within each batch to be the same type to avoid unneccesary subchannel switches.

        /// ================================================================
        /// ==== ASSIGN FINAL BATCH INDICES FOR TASKS AND ACCESS GROUPS ====
        /// ================================================================

        auto & final_batches = tmp_minsh_task_batches;

        for (u32 b = 0; b < final_batches.size(); ++b)
        {
            auto & batch = final_batches[b];
            for (u32 t = 0; t < batch.size(); ++t)
            {
                u32 const task_index = batch[t].second;

                ImplTask & task = impl.tasks[task_index];
                task.final_schedule_batch = b;

                for (u32 attach_i = 0u; attach_i < task.attachments.size(); ++attach_i)
                {
                    bool const null_resource_in_attachment = task.attachment_access_groups[attach_i] == nullptr;
                    if (!null_resource_in_attachment)
                    {
                        task.attachment_access_groups[attach_i]->final_schedule_first_batch = std::min(task.attachment_access_groups[attach_i]->final_schedule_first_batch, b);
                        task.attachment_access_groups[attach_i]->final_schedule_last_batch = std::max(task.attachment_access_groups[attach_i]->final_schedule_last_batch, b);
                    }
                }
            }
        }

        /// ======================================================
        /// ==== DETERMINE TRANSIENT RESOURCE BATCH LIFETIMES ====
        /// ======================================================

        // The lifetimes of resources are bound by the first and the last task that accesses that resource.
        // As we perform all sync on a batch granularity we only care about the lifetime of the resources relative to batches.
        // Here we determine the first and last batch each resource is accessed in.

        // There are two lifetime granularities: batch granularity and submit granularity.
        // Both are relevant when determining transient resource aliasing.

        for (u32 resource_i = 0u; resource_i < impl.resources.size(); ++resource_i)
        {
            ImplTaskResource & resource = impl.resources[resource_i];

            if (resource.access_timeline.size() == 0)
            {
                continue;
            }

            resource.final_schedule_first_batch = ~0u;
            resource.final_schedule_last_batch = 0u;
            resource.final_schedule_first_submit = ~0u;
            resource.final_schedule_last_submit = 0u;
            for (u32 g = 0u; g < 2; ++g)
            {
                // Tasks can not be reordered across access groups.
                // We only consider the first and last access group.
                // Those will always contain the first and last access to the resource.
                AccessGroup & access_group = g == 0 ? resource.access_timeline[0] : resource.access_timeline.back();

                for (u32 t = 0; t < access_group.tasks.size(); ++t)
                {
                    u32 task_index = access_group.tasks[t].task_index;
                    u32 batch_index = impl.tasks[task_index].final_schedule_batch;
                    u32 submit_index = impl.tasks[task_index].submit_index;
                    resource.final_schedule_first_batch = std::min(resource.final_schedule_first_batch, batch_index);
                    resource.final_schedule_last_batch = std::max(resource.final_schedule_last_batch, batch_index);
                    resource.final_schedule_first_submit = std::min(resource.final_schedule_first_submit, submit_index);
                    resource.final_schedule_last_submit = std::max(resource.final_schedule_last_submit, submit_index);
                }
            }
        }

        /// ==================================================
        /// ==== DETERMINE TRANSIENT RESOURCE ALLOCATIONS ====
        /// ==================================================

        // By detault, TaskGraph will attempt to alias as many transient resource allocations as possible.
        // To find possible aliasing opportunities, for each transient resourcce,
        // it scans all existing allocations, placing the new allocation into a memory hole left by other allocations that are already past their lifetime.
        // When finding memory locations for allocations like this, a strong heuristic is to sort all resources by lifetime before starting to allocate.
        // This way, all allocations with longer lifetimes will be done early and short lives allocations will be made later.
        // This works out good in most cases as the short lived allocations will then "sit on top" of many long lived allocations, leaving larger holes for aliasing overlapping.

        // Filter and sort transient resources by lifetime
        auto transient_resources_sorted_by_lifetime = tmp_memory.allocate_trivial_span<ImplTaskResource *>(impl.resources.size());
        auto transient_resource_count = 0u;
        for (u32 r = 0; r < impl.resources.size(); ++r)
        {
            ImplTaskResource & resource = impl.resources[r];
            if (resource.external == nullptr)
            {
                transient_resources_sorted_by_lifetime[transient_resource_count++] = &resource;
            }
        }
        transient_resources_sorted_by_lifetime = std::span{transient_resources_sorted_by_lifetime.data(), static_cast<usize>(transient_resource_count)};
        std::sort(transient_resources_sorted_by_lifetime.begin(), transient_resources_sorted_by_lifetime.end(), [&](ImplTaskResource const * r0, ImplTaskResource const * r1)
                  {
            u32 const r0_lifetime = r0->final_schedule_last_batch - r0->final_schedule_first_batch + 1u;
            u32 const r1_lifetime = r1->final_schedule_last_batch - r1->final_schedule_first_batch + 1u;
            return r0_lifetime > r1_lifetime; });

        // Calculate transient heap size and allocation offsets.
        auto transient_heap_size = 0ull;
        auto transient_heap_alignment = 0ull;
        auto transient_heap_memory_bits = ~0u;
        struct TransientResourceAllocation
        {
            ImplTaskResource * resource = {};
            u64 offset = {};
            u64 size = {};
        };
        auto transient_resource_allocations = tmp_memory.allocate_trivial_span<TransientResourceAllocation>(transient_resource_count);
        for (u32 tr = 0; tr < transient_resource_count; ++tr)
        {
            u32 const allocation_count = tr;
            ImplTaskResource & resource = *transient_resources_sorted_by_lifetime[tr];
            MemoryRequirements new_allocation_memory_requirements = {};
            if (resource.kind != TaskResourceKind::IMAGE)
            {
                new_allocation_memory_requirements = impl.info.device.buffer_memory_requirements(BufferInfo{
                    .size = resource.info.buffer.size,
                });
            }
            else
            {
                new_allocation_memory_requirements = impl.info.device.image_memory_requirements(ImageInfo{
                    .flags = resource.info.image.flags,
                    .dimensions = resource.info.image.dimensions,
                    .format = resource.info.image.format,
                    .size = resource.info.image.size,
                    .mip_level_count = resource.info.image.mip_level_count,
                    .array_layer_count = resource.info.image.array_layer_count,
                    .sample_count = resource.info.image.sample_count,
                    .usage = resource.info.image.usage,
                    .sharing_mode = resource.info.image.sharing_mode,
                });
            }

            auto new_allocation = TransientResourceAllocation{
                .resource = &resource,
                .offset = 0u,
                .size = new_allocation_memory_requirements.size,
            };
            auto new_allocation_first_batch = transient_resources_sorted_by_lifetime[tr]->final_schedule_first_batch;
            auto new_allocation_last_batch = transient_resources_sorted_by_lifetime[tr]->final_schedule_last_batch;
            auto new_allocation_first_submit = transient_resources_sorted_by_lifetime[tr]->final_schedule_first_submit;
            auto new_allocation_last_submit = transient_resources_sorted_by_lifetime[tr]->final_schedule_last_submit;

            if (impl.info.alias_transients)
            {
                // Walk over all allocations made so far.
                // Allocations are always sorted by their memory offset.
                // This ensures that when we push back the offset of the new allocation on a collision,
                // we do not have to go back to check all previous allocations we already checked,
                // as they are guaranteedd to all the previous allocations we checked have a smaller offset + size than our current offset,
                // so they could never collide if we bump the new allocations offset.
                u32 last_colliding_allocation = 0u;
                for (u32 alloc_i = 0; alloc_i < allocation_count; ++alloc_i)
                {
                    auto const & other_allocation = transient_resource_allocations[alloc_i];

                    auto other_allocation_first_batch = other_allocation.resource->final_schedule_first_batch;
                    auto other_allocation_last_batch = other_allocation.resource->final_schedule_last_batch;
                    auto other_allocation_first_submit = other_allocation.resource->final_schedule_first_submit;
                    auto other_allocation_last_submit = other_allocation.resource->final_schedule_last_submit;

                    // When considering a single queue, the batches imply a strong ordering between tasks and resource lifetimes.
                    // But execution ordering of batches is not guaranteed across queues within a submit!
                    // Across queues the only ordering guarantees are given by the submits.
                    // Thus, when aliasing resources used across queues, we have to use the submit lifetimes.
                    // For resource aliasing between resources used on the same queue, we can use the batch lifetimes.
                    auto allocation_resource_queue_access_identical = new_allocation.resource->used_queues_bitfield == other_allocation.resource->used_queues_bitfield;
                    auto allocations_used_across_multiple_queues = std::popcount(new_allocation.resource->used_queues_bitfield) > 1u || std::popcount(other_allocation.resource->used_queues_bitfield) > 1u;
                    bool use_submit_lifetime_granularity = !allocation_resource_queue_access_identical || allocations_used_across_multiple_queues;

                    auto allocation_lifetimes_collide = false;
                    if (use_submit_lifetime_granularity)
                    {
                        bool const new_is_before_other = new_allocation_last_submit < other_allocation_first_submit;
                        bool const new_is_after_other = new_allocation_first_submit > other_allocation_last_submit;
                        allocation_lifetimes_collide = !new_is_before_other && !new_is_after_other;
                    }
                    else // batch lifetime granularity
                    {
                        bool const new_is_before_other = new_allocation_last_batch < other_allocation_first_batch;
                        bool const new_is_after_other = new_allocation_first_batch > other_allocation_last_batch;
                        allocation_lifetimes_collide = !new_is_before_other && !new_is_after_other;
                    }

                    if (allocation_lifetimes_collide)
                    {
                        bool const new_is_below_other = (new_allocation.offset + new_allocation.size) < other_allocation.offset;
                        bool const new_is_above_other = new_allocation.offset > (other_allocation.offset + other_allocation.size);
                        bool const allocation_memory_ranges_collide = !new_is_below_other && !new_is_above_other;
                        if (allocation_memory_ranges_collide)
                        {
                            new_allocation.offset = align_up(other_allocation.offset + other_allocation.size, new_allocation_memory_requirements.alignment);
                            last_colliding_allocation = alloc_i;
                        }
                    }
                }

                // Insert the new allocation so that we keep the allocations sorted by offset.
                // We can already skip all allocations before the last colliding allocation,
                // as they are guaranteed to have a smaller offset than the new allocation.
                // Search in relevant present allocations for a spot to insert the new allocation.
                for (u32 alloc_i = last_colliding_allocation; alloc_i < allocation_count; ++alloc_i)
                {
                    bool const insert = new_allocation.offset >= transient_resource_allocations[alloc_i].offset;
                    if (insert)
                    {
                        // Insert new allocation after alloc_i
                        // Move back all other allocations after alloc_i
                        // last_new_allocation_index is correct, as we are adding a new element here.
                        u32 const last_new_allocation_index = allocation_count;
                        for (u32 i = last_new_allocation_index; i > (alloc_i + 1); --i)
                        {
                            transient_resource_allocations[i] = transient_resource_allocations[i - 1];
                        }
                        transient_resource_allocations[alloc_i + 1] = new_allocation;
                        break;
                    }
                }
                if (tr == 0)
                {
                    transient_resource_allocations[0] = new_allocation;
                }
            }
            else
            {
                new_allocation.offset = align_up(transient_heap_size, new_allocation_memory_requirements.alignment);
                transient_resource_allocations[tr] = new_allocation;
            }

            transient_heap_size = std::max(transient_heap_size, new_allocation.offset + new_allocation.size);
            transient_heap_alignment = std::max(transient_heap_alignment, new_allocation_memory_requirements.alignment);
            transient_heap_memory_bits &= new_allocation_memory_requirements.memory_type_bits;
        }

        // Allocate transient resource heap
        if (transient_heap_size > 0)
        {
            impl.transient_memory_block = impl.info.device.create_memory({
                .requirements = {
                    .size = transient_heap_size,
                    .alignment = transient_heap_alignment,
                    .memory_type_bits = transient_heap_memory_bits,
                },
                .flags = {},
            });
        }

        /// ====================================
        /// ==== CREATE TRANSIENT RESOURCES ====
        /// ====================================

        for (u32 alloc_i = 0; alloc_i < transient_resource_allocations.size(); ++alloc_i)
        {
            TransientResourceAllocation & allocation = transient_resource_allocations[alloc_i];

            switch (allocation.resource->kind)
            {
            case TaskResourceKind::BUFFER:
            {
                auto info = BufferInfo{
                    .size = allocation.resource->info.buffer.size,
                    .name = allocation.resource->name,
                };
                allocation.resource->id.buffer = impl.info.device.create_buffer_from_memory_block(MemoryBlockBufferInfo{
                    .buffer_info = info,
                    .memory_block = impl.transient_memory_block,
                    .offset = allocation.offset,
                });
            }
            break;
            case TaskResourceKind::TLAS:
            {
                auto info = TlasInfo{
                    .size = allocation.resource->info.buffer.size,
                    .name = allocation.resource->name,
                };
                allocation.resource->id.tlas = impl.info.device.create_tlas_from_memory_block(MemoryBlockTlasInfo{
                    .tlas_info = info,
                    .memory_block = impl.transient_memory_block,
                    .offset = allocation.offset,
                });
            }
            break;
            case TaskResourceKind::BLAS:
                DAXA_DBG_ASSERT_TRUE_M(false, "IMPOSSIBLE CASE, THERE IS NO SUPPORT FOR TRANSIENT BLAS!");
                break;
            case TaskResourceKind::IMAGE:
            {
                auto info = ImageInfo{
                    .flags = allocation.resource->info.image.flags,
                    .dimensions = allocation.resource->info.image.dimensions,
                    .format = allocation.resource->info.image.format,
                    .size = allocation.resource->info.image.size,
                    .mip_level_count = allocation.resource->info.image.mip_level_count,
                    .array_layer_count = allocation.resource->info.image.array_layer_count,
                    .sample_count = allocation.resource->info.image.sample_count,
                    .usage = allocation.resource->info.image.usage,
                    .sharing_mode = allocation.resource->info.image.sharing_mode,
                    .name = allocation.resource->name,
                };
                allocation.resource->id.image = impl.info.device.create_image_from_memory_block(MemoryBlockImageInfo{
                    .image_info = info,
                    .memory_block = impl.transient_memory_block,
                    .offset = allocation.offset,
                });
            }
            break;
            }
        }

        /// ========================================
        /// ==== CREATE RESOURCE BATCH BARRIERS ====
        /// ========================================

        // Within each AccessGroup, all tasks MUST have the same (concurrent) access to the resource.
        // Between each AccessGroup within an access timeline, the access will be different.
        // This means between all the access groups within a access timeline, there must be a pipeline barrier.
        // In many cases, there will be multiple batches between access groups, in these cases we could use split barriers to hide potential cache flushes.
        // Currently, taskgraph does a very simple strategy, placing a normal barrier just before each access groups first batch.

        struct BatchBarriers
        {
            ArenaDynamicArray8k<ImageBarrierInfo> image_barriers = {};
            ArenaDynamicArray8k<BarrierInfo> barriers = {};
        };
        auto tmp_batch_barriers = tmp_memory.allocate_trivial_span_fill<BatchBarriers>(
            final_batches.size(),
            BatchBarriers{
                ArenaDynamicArray8k<ImageBarrierInfo>(&tmp_memory),
                ArenaDynamicArray8k<BarrierInfo>(&tmp_memory),
            });

        // All transient images have to be transformed from UNDEFINED to GENERAL layout before their first usage
        for (u32 tr = 0u; tr < transient_resources_sorted_by_lifetime.size(); ++tr)
        {
            ImplTaskResource & resource = *transient_resources_sorted_by_lifetime[tr];

            if (resource.access_timeline.size() == 0)
            {
                continue;
            }

            auto const & first_access_group = resource.access_timeline[0];
            auto const stages = first_access_group.used_stages;
            auto const access_type_flags = task_access_type_to_access_type(first_access_group.type);
            auto const first_use_batch = resource.final_schedule_first_batch;
            tmp_batch_barriers[first_use_batch].image_barriers.push_back(ImageBarrierInfo{
                .src_access = AccessConsts::NONE,
                .dst_access = Access{stages, access_type_flags},
                .image_id = resource.id.image,
                .layout_operation = ImageLayoutOperation::TO_GENERAL,
            });
        }

        // We need a barrier between all access groups for all resources
        for (u32 r = 0; r < impl.resources.size(); ++r)
        {
            ImplTaskResource & resource = impl.resources[r];

            for (u32 ag = 1u; ag < resource.access_timeline.size(); ++ag)
            {
                AccessGroup & first_ag = resource.access_timeline[ag - 1];
                AccessGroup & second_ag = resource.access_timeline[ag];
                auto const first_ag_access = Access{first_ag.used_stages, task_access_type_to_access_type(first_ag.type)};
                auto const second_ag_access = Access{second_ag.used_stages, task_access_type_to_access_type(second_ag.type)};

                // Add support for split barriers in the future.
                // Investigate smarter barrier insertion tactics.
                u32 barrier_insertion_batch = second_ag.final_schedule_first_batch;

                if (impl.info.amd_rdna3_4_image_barrier_fix && resource.kind == TaskResourceKind::IMAGE)
                {
                    tmp_batch_barriers[barrier_insertion_batch].image_barriers.push_back(ImageBarrierInfo{
                        .src_access = first_ag_access,
                        .dst_access = second_ag_access,
                        .image_id = resource.id.image,
                    });
                }
                else
                {
                    tmp_batch_barriers[barrier_insertion_batch].barriers.push_back(BarrierInfo{
                        .src_access = first_ag_access,
                        .dst_access = second_ag_access,
                    });
                }
            }
        }

        /// =============================
        /// ==== FILL EXECUTION DATA ====
        /// =============================

        u32 external_resource_count = static_cast<u32>(impl.resources.size()) - transient_resource_count;
        impl.external_resources = impl.task_memory.allocate_trivial_span<std::pair<ImplTaskResource *, u32>>(external_resource_count);
        u32 tmp_current_external_resource = 0;
        for (u32 r = 0; r < impl.resources.size(); ++r)
        {
            if (impl.resources[r].external != nullptr)
            {
                impl.external_resources[tmp_current_external_resource++] = std::pair{&impl.resources[r], r};
            }
        }

        /// =========================================
        /// ==== INITIALIZE TASK ATTACHMENT DATA ====
        /// =========================================

        // TaskAttachmentInfo ids and view are initialized here.
        // The AttachmentShaderBlob is precalculated and filled here as well.
        // External resource values are left empty here.
        // The external resource ids, views and attachment shader blob entries are patched when external resources change once before execution.

        for (u32 task_i = 0; task_i < impl.tasks.size(); ++task_i)
        {
            ImplTask & task = impl.tasks.at(task_i);
            initialize_attachment_ids(impl, task);
            initialize_attachment_image_views(impl, task);
            initialize_attachment_shader_blob(impl, task);
        }

        impl.compiled = true;
    }

    auto TaskGraph::get_transient_memory_size() -> daxa::usize
    {
        auto & impl = *r_cast<ImplTaskGraph *>(this->object);
        return impl.transient_memory_block.info().requirements.size;
    }

    void TaskGraph::execute([[maybe_unused]] ExecutionInfo const & info)
    {
        auto & impl = *r_cast<ImplTaskGraph *>(this->object);
        DAXA_DBG_ASSERT_TRUE_M(impl.compiled, "ERROR: TaskGraph must be completed before execution!");

        u32 required_tmp_size = 1u << 23u; /* 8MB */
        MemoryArena tmp_memory = MemoryArena{"TaskGraph::execute tmp memory", required_tmp_size};

        /// =============================================================================
        /// ==== VALIDATE, PATCH AND GENERATE CONNECTING SYNC FOR EXTERNAL RESOURCES ====
        /// =============================================================================

        // External resources can be changed outside of the taskgraph.
        // Calculate required sync from the pre graph access to the first access of external resources here.
        // Validate all external resources: Are the ids valid? Do the resources match the access requirements?
        // Update ids and data of the external resource inside taskgraph.
        // If the id changed, patch attachment blobs and recreate image views for tasks accessing the resource.
        // All image views and attachment blobs are pre-created and keept over executions.
        // Only patch the outdated parts of attachment blobs and image view arrays when a external resource changed. 

        struct ExternalResourceSyncNeeds
        {
            u32 src_queues = {};
            u32 dst_queues = {};
            Access src_access = {};
            Access dst_access = {};
            ImageId transform_to_general_image = {};
        };
        auto external_resource_sync_needs = ArenaDynamicArray8k<ExternalResourceSyncNeeds>(&tmp_memory);

        // Validate and patch external resources
        for (u32 er = 0; er < impl.external_resources.size(); ++er)
        {
            auto [resource, resource_i] = impl.external_resources[er];

            DAXA_DBG_ASSERT_TRUE_M(resource->external != nullptr, "IMPOSSIBLE CASE, POSSIBLY CAUSED BY DATA CORRUPTION!");

            // Skip unused resources
            if (resource->access_timeline.size() == 0)
            {
                continue;
            }

            // Validate id
            // Check if id changed
            // Set new id
            bool did_id_change = false;
            auto validate_id_count = [&](auto id_count)
            {
                DAXA_DBG_ASSERT_TRUE_M(
                    id_count > 0,
                    std::format(
                        "ERROR: All external task resources must have valid ids assigned to them when executing a TaskGraph! "
                        "Detected unassigned id for task resource \"{}\" (index: {}).",
                        resource->name, resource_i)
                        .c_str());
                DAXA_DBG_ASSERT_TRUE_M(
                    id_count == 1,
                    std::format(
                        "ERROR: All external task resources must have at most one id assigned to them when executing a TaskGraph! "
                        "Detected multiple ids for task resource \"{}\" (index: {}).",
                        resource->name, resource_i)
                        .c_str());
            };
            auto validate_id = [&](auto id)
            {
                DAXA_DBG_ASSERT_TRUE_M(
                    impl.info.device.is_id_valid(id),
                    std::format(
                        "ERROR: All external task resources must have non valid id assigned to them when executing a TaskGraph! "
                        "Detected invalid id for task resource \"{}\" (index: {}).",
                        resource->name, resource_i)
                        .c_str());
            };
            switch (resource->kind)
            {
            case TaskResourceKind::BUFFER:
            {
                ImplPersistentTaskBufferBlasTlas * external = static_cast<ImplPersistentTaskBufferBlasTlas *>(resource->external);
                auto & ids = std::get<std::vector<BufferId>>(external->actual_ids);
                validate_id_count(ids.size());
                validate_id(ids[0]);
                did_id_change = resource->id.buffer != ids[0];
                resource->id.buffer = ids[0];
            }
            break;
            case TaskResourceKind::TLAS:
            {
                ImplPersistentTaskBufferBlasTlas * external = static_cast<ImplPersistentTaskBufferBlasTlas *>(resource->external);
                auto & ids = std::get<std::vector<TlasId>>(external->actual_ids);
                validate_id_count(ids.size());
                validate_id(ids[0]);
                did_id_change = resource->id.tlas != ids[0];
                resource->id.tlas = ids[0];
            }
            break;
            case TaskResourceKind::BLAS:
            {
                ImplPersistentTaskBufferBlasTlas * external = static_cast<ImplPersistentTaskBufferBlasTlas *>(resource->external);
                auto & ids = std::get<std::vector<BlasId>>(external->actual_ids);
                validate_id_count(ids.size());
                validate_id(ids[0]);
                did_id_change = resource->id.blas != ids[0];
                resource->id.blas = ids[0];
            }
            break;
            case TaskResourceKind::IMAGE:
            {
                ImplPersistentTaskImage * external = static_cast<ImplPersistentTaskImage *>(resource->external);
                validate_id_count(external->actual_images.size());
                validate_id(external->actual_images[0]);
                did_id_change = resource->id.image != external->actual_images[0];
                resource->id.image = external->actual_images[0];
            }
            break;
            }

            // Only have to patch when id changed
            // Patch all image views that reference the external image
            // Patch all attachments that reference the external resource
            if (did_id_change)
            {
                for (u32 access_group_i = 0; access_group_i < resource->access_timeline.size(); ++access_group_i)
                {
                    AccessGroup const & access_group = resource->access_timeline[access_group_i];

                    for (u32 t = 0; t < access_group.tasks.size(); ++t)
                    {
                        TaskAttachmentAccess const & task_attachment_access = access_group.tasks[t];

                        if (resource->kind == TaskResourceKind::IMAGE)
                        {
                            patch_attachment_image_views(impl, *task_attachment_access.task, task_attachment_access.attachment_index, *resource);
                        }
                        patch_attachment_shader_blob(impl, *task_attachment_access.task, task_attachment_access.attachment_index, *resource);
                    }
                }
            }

            // Determine sync requirements to the pre graph access
            // Update external resources pre graph access for the next time they are used in a graph.
            if (resource->kind != TaskResourceKind::IMAGE)
            {
                ImplPersistentTaskBufferBlasTlas * external = static_cast<ImplPersistentTaskBufferBlasTlas *>(resource->external);

                // Determine if there needs to be sync between the pre graph and first graph access.
                AccessGroup first_access_group = resource->access_timeline[0];
                Access pre_graph_access = external->pre_graph_access;
                Access first_graph_access = Access{ first_access_group.used_stages, task_access_type_to_access_type(first_access_group.type) };
                bool const access_matches = pre_graph_access == first_graph_access;
                bool const queues_match = external->pre_graph_used_queues_bitfield == first_access_group.used_queues_bitfield;
                if (!access_matches || !queues_match)
                {
                    external_resource_sync_needs.push_back(ExternalResourceSyncNeeds{
                        .src_queues = external->pre_graph_used_queues_bitfield,
                        .dst_queues = first_access_group.used_queues_bitfield,
                        .src_access = external->pre_graph_access,
                        .dst_access = first_graph_access,
                    });
                }

                // update pre graph information for the next graph execution using this resource
                AccessGroup last_access_group = resource->access_timeline.back();
                Access last_graph_access = Access{ last_access_group.used_stages, task_access_type_to_access_type(last_access_group.type) };
                external->pre_graph_access = last_graph_access;
                external->pre_graph_used_queues_bitfield = last_access_group.used_queues_bitfield;
            }
            else
            {
                ImplPersistentTaskImage * external = static_cast<ImplPersistentTaskImage *>(resource->external);

                // Determine if there needs to be sync between the pre graph and first graph access.
                AccessGroup first_access_group = resource->access_timeline[0];
                Access pre_graph_access = external->pre_graph_access;
                Access first_graph_access = Access{ first_access_group.used_stages, task_access_type_to_access_type(first_access_group.type) };
                bool const access_matches = pre_graph_access == first_graph_access;
                bool const queues_match = external->pre_graph_used_queues_bitfield == first_access_group.used_queues_bitfield;
                bool const transform_to_general = external->pre_graph_is_undefined_layout;
                if (!access_matches || !queues_match)
                {
                    external_resource_sync_needs.push_back(ExternalResourceSyncNeeds{
                        .src_queues = external->pre_graph_used_queues_bitfield,
                        .dst_queues = first_access_group.used_queues_bitfield,
                        .src_access = external->pre_graph_access,
                        .dst_access = first_graph_access,
                        .transform_to_general_image = transform_to_general ? external->actual_images[0] : ImageId{},
                    });
                }
                
                // update pre graph information for the next graph execution using this resource
                AccessGroup last_access_group = resource->access_timeline.back();
                Access last_graph_access = Access{ last_access_group.used_stages, task_access_type_to_access_type(last_access_group.type) };
                external->pre_graph_access = last_graph_access;
                external->pre_graph_used_queues_bitfield = last_access_group.used_queues_bitfield;
                external->pre_graph_is_undefined_layout = false;
            }
        }

        /// ================================
        /// ==== PREPARE TASK EXECUTION ====
        /// ================================

        /// ====================================
        /// ==== RECORD AND SUBMIT COMMANDS ====
        /// ====================================
    }

    ImplTaskGraph::ImplTaskGraph(TaskGraphInfo a_info)
        : unique_index{ImplTaskGraph::exec_unique_next_index++}, info{std::move(a_info)},
          task_memory{"TaskGraph task memory pool", a_info.task_memory_pool_size}
    {
        info.name = task_memory.allocate_copy_string(info.name);
        tasks = ArenaDynamicArray8k<ImplTask>(&task_memory);
        resources = ArenaDynamicArray8k<ImplTaskResource>(&task_memory);
        submits = ArenaDynamicArray8k<TasksSubmit>(&task_memory);
    }

    ImplTaskGraph::~ImplTaskGraph()
    {
        // Destroy image views
        for (u32 t = 0u; t < this->tasks.size(); ++t)
        {
            ImplTask & task = this->tasks[t];
            for (u32 attach_i = 0; attach_i < task.attachments.size(); ++attach_i)
            {
                // Null Attachments have to be skipped
                if (task.attachment_access_groups[attach_i] == nullptr)
                {
                    continue;
                }

                std::span<ImageViewId> views = task.attachment_image_views[attach_i];
                for (u32 view_i = 0u; view_i < views.size(); ++view_i)
                {
                    // Tail of image views can be empty when the task image view slice is smaller than the shader array size.
                    // Skip tail views.
                    if (views[view_i].is_empty())
                    {
                        break;
                    }
                    ImageId parent_image = this->info.device.image_view_info(views[view_i]).value().image;
                    ImageViewId default_view = parent_image.default_view();
                    bool is_default_view = default_view == views[view_i];
                    if (!is_default_view)
                    {
                        this->info.device.destroy_image_view(views[view_i]);
                    }
                }
            }
        }

        // Destroy transient resources
        // Decrement external resource reference counters
        for (u32 i = 0; i < this->resources.size(); ++i)
        {
            ImplTaskResource & resource = this->resources[i];
            if (resource.external != nullptr)
            {
                // TaskBuffer is idential to Tlas and Blas internals.
                if (resource.kind != TaskResourceKind::IMAGE)
                {
                    static_cast<ImplPersistentTaskBufferBlasTlas*>(resource.external)->dec_refcnt(
                        ImplPersistentTaskBufferBlasTlas::zero_ref_callback,
                        nullptr);
                }
                else
                {
                    static_cast<ImplPersistentTaskImage*>(resource.external)->dec_refcnt(
                        ImplPersistentTaskImage::zero_ref_callback,
                        nullptr);
                }
                continue;
            }
            switch (resource.kind)
            {
            case TaskResourceKind::BUFFER:
                if (!resource.id.buffer.is_empty())
                    this->info.device.destroy_buffer(resource.id.buffer);
                break;
            case TaskResourceKind::TLAS:
                if (!resource.id.tlas.is_empty())
                    this->info.device.destroy_tlas(resource.id.tlas);
                break;
            case TaskResourceKind::BLAS:
                if (!resource.id.blas.is_empty())
                    this->info.device.destroy_blas(resource.id.blas);
                break;
            case TaskResourceKind::IMAGE:
                if (!resource.id.image.is_empty())
                    this->info.device.destroy_image(resource.id.image);
                break;
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