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

    auto queue_to_queue_index(daxa::Queue queue) -> u32
    {
        u32 offsets[3] = {
            0,
            1,
            1 + DAXA_MAX_COMPUTE_QUEUE_COUNT,
        };
        return offsets[static_cast<u32>(queue.family)] + queue.index;
    }

    auto queue_index_to_queue(u32 flat_index) -> daxa::Queue
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

    auto queue_bits_to_first_queue_index(u32 queue_bits) -> u32
    {
        return 31u - static_cast<u32>(std::countl_zero(queue_bits));
    }

    auto queue_index_to_queue_bit(u32 queue_index) -> u32
    {
        return 1u << queue_index;
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
        case TaskType::GENERAL: return TaskStage::NONE;
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
        case TaskAccessType::SAMPLED: ret = AccessTypeFlagBits::READ; break;
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
        return static_cast<daxa::PipelineStageFlags>(static_cast<u64>(stage) & ~static_cast<u64>(TaskStage::JOKER));
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
    case TaskAccessType::SAMPLED: ret = std::string_view{#STAGE "_SAMPLED"}; break;                             \
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

    /// --- ImplExternalResource Begin

    ImplExternalResource::ImplExternalResource(TaskBufferInfo a_info)
    {
        this->name = std::move(a_info.name);
        kind = TaskResourceKind::BUFFER;
        DAXA_DBG_ASSERT_TRUE_M(a_info.initial_buffers.buffers.size() <= 1, "ERROR: External resources support exactly one actual resource!");
        this->id.buffer = a_info.initial_buffers.buffers.size() > 0 ? a_info.initial_buffers.buffers[0] : BufferId{};
        this->unique_index = exec_unique_resource_next_index++;
    }

    ImplExternalResource::ImplExternalResource(Device & device, BufferInfo const & a_info)
    {
        this->name = std::string(a_info.name.c_str());
        this->kind = TaskResourceKind::BUFFER;
        this->id.buffer = device.create_buffer(a_info);
        this->owns_resource = true;
        this->device = device;
        this->unique_index = exec_unique_resource_next_index++;
    }

    ImplExternalResource::ImplExternalResource(TaskBlasInfo a_info)
    {
        this->name = std::move(a_info.name);
        this->kind = TaskResourceKind::BLAS;
        DAXA_DBG_ASSERT_TRUE_M(a_info.initial_blas.blas.size() <= 1, "ERROR: External resources support exactly one actual resource!");
        this->id.blas = a_info.initial_blas.blas.size() > 0 ? a_info.initial_blas.blas[0] : BlasId{};
        this->unique_index = exec_unique_resource_next_index++;
    }

    ImplExternalResource::ImplExternalResource(TaskTlasInfo a_info)
    {
        this->name = std::move(a_info.name);
        this->kind = TaskResourceKind::TLAS;
        DAXA_DBG_ASSERT_TRUE_M(a_info.initial_tlas.tlas.size() <= 1, "ERROR: External resources support exactly one actual resource!");
        this->id.tlas = a_info.initial_tlas.tlas.size() > 0 ? a_info.initial_tlas.tlas[0] : TlasId{};
        this->unique_index = exec_unique_resource_next_index++;
    }

    ImplExternalResource::ImplExternalResource(TaskImageInfo a_info)
    {
        this->name = std::move(a_info.name);
        this->kind = TaskResourceKind::IMAGE;
        DAXA_DBG_ASSERT_TRUE_M(a_info.initial_images.images.size() <= 1, "ERROR: External resources support exactly one actual resource!");
        this->id.image = a_info.initial_images.images.size() > 0 ? a_info.initial_images.images[0] : ImageId{};
        this->is_swapchain_image = a_info.swapchain_image;
        this->unique_index = exec_unique_resource_next_index++;
    }

    ImplExternalResource::~ImplExternalResource()
    {
    }

    void ImplExternalResource::zero_ref_callback(ImplHandle const * handle)
    {
        auto * self = rc_cast<ImplExternalResource *>(handle);
        if (self->kind == TaskResourceKind::BUFFER && self->owns_resource)
        {
            self->device->destroy_buffer(self->id.buffer);
        }
        delete self;
    }

    /// --- ImplExternalResource End

    // --- TaskBuffer ---

    TaskBuffer::TaskBuffer(TaskBufferInfo const & info)
    {
        this->object = new ImplExternalResource(info);
    }

    TaskBuffer::TaskBuffer(daxa::Device & device, BufferInfo const & info)
    {
        this->object = new ImplExternalResource(device, info);
    }

    auto TaskBuffer::view() const -> TaskBufferView
    {
        auto & impl = *r_cast<ImplExternalResource *>(this->object);
        return TaskBufferView{{.task_graph_index = std::numeric_limits<u32>::max(), .index = impl.unique_index}};
    }

    TaskBuffer::operator TaskBufferView() const
    {
        return view();
    }

    auto TaskBuffer::info() const -> TaskBufferInfo
    {
        auto & impl = *r_cast<ImplExternalResource *>(this->object);
        return TaskBufferInfo{
            .initial_buffers = TrackedBuffers{
                .buffers = std::span{&impl.id.buffer, impl.id.buffer.is_empty() ? 0ull : 1ull},
                .latest_access = {},
            },
            .name = std::string(impl.name),
        };
    }

    auto TaskBuffer::get_state() const -> TrackedBuffers
    {
        auto const & impl = *r_cast<ImplExternalResource const *>(this->object);
        return TrackedBuffers{
            .buffers = {&impl.id.buffer, impl.id.buffer.is_empty() ? 0ull : 1ull},
            .latest_access = {},
        };
    }

    auto TaskBuffer::is_owning() const -> bool
    {
        auto const & impl = *r_cast<ImplExternalResource const *>(this->object);
        return impl.owns_resource;
    }

    void TaskBuffer::set_buffers(TrackedBuffers const & buffers)
    {
        auto & impl = *r_cast<ImplExternalResource *>(this->object);

        DAXA_DBG_ASSERT_TRUE_M(buffers.buffers.size() == 1, "ERROR: External resources support exactly one actual resource!");
        DAXA_DBG_ASSERT_TRUE_M(!impl.owns_resource, "ERROR: Can not set actual resource when task resource is owning!");

        impl.id.buffer = buffers.buffers[0];
        impl.pre_graph_queue_bits = {};
    }

    void TaskBuffer::swap_buffers(TaskBuffer & other)
    {
        auto & impl = *r_cast<ImplExternalResource *>(this->object);
        auto & impl_other = *r_cast<ImplExternalResource *>(other.object);

        DAXA_DBG_ASSERT_TRUE_M(!impl.owns_resource, "ERROR: Can not swap actual resource when task resource is owning!");
        DAXA_DBG_ASSERT_TRUE_M(!impl_other.owns_resource, "ERROR: Can not swap actual resource when task resource is owning!");

        std::swap(impl.id.buffer, impl_other.id.buffer);
        std::swap(impl.pre_graph_queue_bits, impl_other.pre_graph_queue_bits);
    }

    auto TaskBuffer::inc_refcnt(ImplHandle const * object) -> u64
    {
        return object->inc_refcnt();
    }

    auto TaskBuffer::dec_refcnt(ImplHandle const * object) -> u64
    {
        return object->dec_refcnt(
            ImplExternalResource::zero_ref_callback,
            nullptr);
    }

    // --- TaskBuffer End ---

    // --- TaskBlas ---

    TaskBlas::TaskBlas(TaskBlasInfo const & info)
    {
        this->object = new ImplExternalResource(info);
    }

    auto TaskBlas::view() const -> TaskBlasView
    {
        auto & impl = *r_cast<ImplExternalResource *>(this->object);
        return TaskBlasView{{.task_graph_index = std::numeric_limits<u32>::max(), .index = impl.unique_index}};
    }

    TaskBlas::operator TaskBlasView() const
    {
        return view();
    }

    auto TaskBlas::info() const -> TaskBlasInfo
    {
        auto & impl = *r_cast<ImplExternalResource *>(this->object);
        return TaskBlasInfo{
            .initial_blas = TrackedBlas{
                .blas = std::span{&impl.id.blas, impl.id.blas.is_empty() ? 0ull : 1ull},
                .latest_access = {},
            },
            .name = std::string(impl.name),
        };
    }

    auto TaskBlas::get_state() const -> TrackedBlas
    {
        auto const & impl = *r_cast<ImplExternalResource const *>(this->object);
        return TrackedBlas{
            .blas = {&impl.id.blas, impl.id.blas.is_empty() ? 0ull : 1ull},
            .latest_access = {},
        };
    }

    void TaskBlas::set_blas(TrackedBlas const & other_tracked)
    {
        auto & impl = *r_cast<ImplExternalResource *>(this->object);

        DAXA_DBG_ASSERT_TRUE_M(other_tracked.blas.size() == 1, "ERROR: External resources support exactly one actual resource!");
        DAXA_DBG_ASSERT_TRUE_M(!impl.owns_resource, "ERROR: Can not set actual resource when task resource is owning!");

        impl.id.blas = other_tracked.blas[0];
        impl.pre_graph_queue_bits = {};
    }

    void TaskBlas::swap_blas(TaskBlas & other)
    {
        auto & impl = *r_cast<ImplExternalResource *>(this->object);
        auto & impl_other = *r_cast<ImplExternalResource *>(other.object);

        DAXA_DBG_ASSERT_TRUE_M(!impl.owns_resource, "ERROR: Can not swap actual resource when task resource is owning!");
        DAXA_DBG_ASSERT_TRUE_M(!impl_other.owns_resource, "ERROR: Can not swap actual resource when task resource is owning!");

        std::swap(impl.id.blas, impl_other.id.blas);
        std::swap(impl.pre_graph_queue_bits, impl_other.pre_graph_queue_bits);
    }

    auto TaskBlas::inc_refcnt(ImplHandle const * object) -> u64
    {
        return object->inc_refcnt();
    }

    auto TaskBlas::dec_refcnt(ImplHandle const * object) -> u64
    {
        return object->dec_refcnt(
            ImplExternalResource::zero_ref_callback,
            nullptr);
    }

    // --- TaskBlas End ---

    // --- TaskTlas ---

    TaskTlas::TaskTlas(TaskTlasInfo const & info)
    {
        this->object = new ImplExternalResource(info);
    }

    auto TaskTlas::view() const -> TaskTlasView
    {
        auto & impl = *r_cast<ImplExternalResource *>(this->object);
        return TaskTlasView{{.task_graph_index = std::numeric_limits<u32>::max(), .index = impl.unique_index}};
    }

    TaskTlas::operator TaskTlasView() const
    {
        return view();
    }

    auto TaskTlas::info() const -> TaskTlasInfo
    {
        auto & impl = *r_cast<ImplExternalResource *>(this->object);
        return TaskTlasInfo{
            .initial_tlas = TrackedTlas{
                .tlas = std::span{&impl.id.tlas, impl.id.tlas.is_empty() ? 0ull : 1ull},
                .latest_access = {},
            },
            .name = std::string(impl.name),
        };
    }

    auto TaskTlas::get_state() const -> TrackedTlas
    {
        auto const & impl = *r_cast<ImplExternalResource const *>(this->object);
        return TrackedTlas{
            .tlas = {&impl.id.tlas, impl.id.tlas.is_empty() ? 0ull : 1ull},
            .latest_access = {},
        };
    }

    void TaskTlas::set_tlas(TrackedTlas const & other_tracked)
    {
        auto & impl = *r_cast<ImplExternalResource *>(this->object);

        DAXA_DBG_ASSERT_TRUE_M(other_tracked.tlas.size() == 1, "ERROR: External resources support exactly one actual resource!");
        DAXA_DBG_ASSERT_TRUE_M(!impl.owns_resource, "ERROR: Can not set actual resource when task resource is owning!");

        impl.id.tlas = other_tracked.tlas[0];
        impl.pre_graph_queue_bits = {};
    }

    void TaskTlas::swap_tlas(TaskTlas & other)
    {
        auto & impl = *r_cast<ImplExternalResource *>(this->object);
        auto & impl_other = *r_cast<ImplExternalResource *>(other.object);

        DAXA_DBG_ASSERT_TRUE_M(!impl.owns_resource, "ERROR: Can not swap actual resource when task resource is owning!");
        DAXA_DBG_ASSERT_TRUE_M(!impl_other.owns_resource, "ERROR: Can not swap actual resource when task resource is owning!");

        std::swap(impl.id.tlas, impl_other.id.tlas);
        std::swap(impl.pre_graph_queue_bits, impl_other.pre_graph_queue_bits);
    }

    auto TaskTlas::inc_refcnt(ImplHandle const * object) -> u64
    {
        return object->inc_refcnt();
    }

    auto TaskTlas::dec_refcnt(ImplHandle const * object) -> u64
    {
        return object->dec_refcnt(
            ImplExternalResource::zero_ref_callback,
            nullptr);
    }

    // --- TaskTlas End ---

    TaskImage::TaskImage(TaskImageInfo const & a_info)
    {
        this->object = new ImplExternalResource(a_info);
    }

    TaskImage::operator TaskImageView() const
    {
        return view();
    }

    auto TaskImage::view() const -> TaskImageView
    {
        auto & impl = *r_cast<ImplExternalResource *>(this->object);
        return TaskImageView{.task_graph_index = std::numeric_limits<u32>::max(), .index = impl.unique_index};
    }

    auto TaskImage::info() const -> TaskImageInfo
    {
        auto & impl = *r_cast<ImplExternalResource *>(this->object);
        return TaskImageInfo{
            .initial_images = TrackedImages{
                .images = std::span{&impl.id.image, impl.id.image.is_empty() ? 0ull : 1ull},
                .latest_slice_states = {},
            },
            .name = std::string(impl.name),
        };
    }

    auto TaskImage::get_state() const -> TrackedImages
    {
        auto const & impl = *r_cast<ImplExternalResource const *>(this->object);
        return TrackedImages{
            .images = {&impl.id.image, impl.id.image.is_empty() ? 0ull : 1ull},
            .latest_slice_states = {},
        };
    }

    void TaskImage::set_images(TrackedImages const & images)
    {
        auto & impl = *r_cast<ImplExternalResource *>(this->object);

        DAXA_DBG_ASSERT_TRUE_M(images.images.size() == 1, "ERROR: External resources support exactly one actual resource!");
        DAXA_DBG_ASSERT_TRUE_M(!impl.owns_resource, "ERROR: Can not set actual resource when task resource is owning!");

        impl.id.image = images.images[0];
        impl.pre_graph_queue_bits = {};
        impl.pre_graph_is_general_layout = false;
        impl.was_presented = false;
    }

    void TaskImage::swap_images(TaskImage & other)
    {
        auto & impl = *r_cast<ImplExternalResource *>(this->object);
        auto & impl_other = *r_cast<ImplExternalResource *>(other.object);

        DAXA_DBG_ASSERT_TRUE_M(!impl.owns_resource, "ERROR: Can not swap actual resource when task resource is owning!");
        DAXA_DBG_ASSERT_TRUE_M(!impl_other.owns_resource, "ERROR: Can not swap actual resource when task resource is owning!");

        std::swap(impl.id.image, impl_other.id.image);
        std::swap(impl.pre_graph_queue_bits, impl_other.pre_graph_queue_bits);
        std::swap(impl.pre_graph_is_general_layout, impl_other.pre_graph_is_general_layout);
    }

    auto TaskImage::inc_refcnt(ImplHandle const * object) -> u64
    {
        return object->inc_refcnt();
    }

    auto TaskImage::dec_refcnt(ImplHandle const * object) -> u64
    {
        return object->dec_refcnt(
            ImplExternalResource::zero_ref_callback,
            nullptr);
    }

    TaskGraph::TaskGraph(TaskGraphInfo const & info)
    {
        this->object = new ImplTaskGraph(info);
        auto & impl = *r_cast<ImplTaskGraph *>(this->object);
    }
    TaskGraph::~TaskGraph() = default;

    void register_external_buffer_helper(ImplTaskGraph & impl, TaskResourceKind kind, ImplExternalResource * external)
    {
        u32 const global_unique_external_index = external->unique_index;
        auto name = impl.task_memory.allocate_copy_string(external->name);

        DAXA_DBG_ASSERT_TRUE_M(!impl.compiled, "completed task graphs can not record new tasks");
        DAXA_DBG_ASSERT_TRUE_M(!impl.name_to_resource_table.contains(name), "Detected duplicate external resource name. All external resources must have a unique name.");
        DAXA_DBG_ASSERT_TRUE_M(!impl.external_idx_to_resource_table.contains(global_unique_external_index), "Detected duplicate registration of external resource. All external resources must only be added to a graph once.");

        external->inc_refcnt();

        ImplTaskResource buffer = {};
        buffer.name = name;
        buffer.kind = kind;
        buffer.external = external;
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

        ImplExternalResource * external = timg.get();
        u32 const global_unique_external_index = external->unique_index;
        auto name = impl.task_memory.allocate_copy_string(external->name);

        DAXA_DBG_ASSERT_TRUE_M(!impl.compiled, "ERROR: Completed task graphs can not record new tasks!");
        DAXA_DBG_ASSERT_TRUE_M(!impl.name_to_resource_table.contains(name),
                               std::format(
                                   "ERROR: Every task resource must have unique name per task graph! "
                                   "Duplicate external resource name \"{}\".",
                                   name)
                                   .c_str());
        DAXA_DBG_ASSERT_TRUE_M(!impl.external_idx_to_resource_table.contains(global_unique_external_index),
                               std::format(
                                   "ERROR: Every external resource can only be registered once to a task graph! "
                                   "Duplicate external resource \"{}\".",
                                   name)
                                   .c_str());
        DAXA_DBG_ASSERT_TRUE_M(!(impl.swapchain_image != nullptr && external->is_swapchain_image),
                               std::format(
                                   "ERROR: Every task graph can only register a single swapchain image! "
                                   "Attempted to add new swapchain image \"{}\" while there is already a swapchain image registered \"{}\" registered.",
                                   name, impl.swapchain_image->name)
                                   .c_str());
        DAXA_DBG_ASSERT_TRUE_M(!(external->is_swapchain_image && !impl.info.swapchain.has_value()),
                               std::format(
                                   "ERROR: When using a swapchain image in a graph, the graph must have the swapchain for that image! "
                                   "Attempted to add new swapchain image \"{}\" while there is no swapchain passed to the graph info.",
                                   name, impl.swapchain_image->name)
                                   .c_str());

        external->inc_refcnt();

        ImplTaskResource image = {};
        image.kind = TaskResourceKind::IMAGE;
        image.name = name;
        image.external = external;
        image.id = {};   // Set in execution preparation.
        image.info = {}; // Set in execution preparation.
        impl.resources.push_back(image);
        u32 const index = static_cast<u32>(impl.resources.size()) - 1u;

        if (external->is_swapchain_image)
        {
            impl.swapchain_image = &impl.resources[index];
        }

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
                .base_mip_level = 0,
                .level_count = info.mip_level_count,
                .base_array_layer = 0,
                .layer_count = info.array_layer_count,
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
            if (attachment_info.type == TaskAttachmentType::IMAGE)
            {
                ImplTaskResource const * resource = nullptr;
                if (!attachment_info.value.image.translated_view.is_null())
                {
                    resource = &impl.resources[attachment_info.value.image.translated_view.index];
                }

                if (attachment_info.value.image.shader_array_type == TaskHeadImageArrayType::MIP_LEVELS)
                {
                    for (u32 mip = 0; mip < attachment_info.value.image.shader_array_size; ++mip)
                    {
                        ImageViewId view = {};
                        if (resource != nullptr && resource->external == nullptr && mip < attachment_info.value.image.translated_view.slice.level_count)
                        {
                            ImageId id = resource->id.image;
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
                    if (resource != nullptr && resource->external == nullptr)
                    {
                        ImageId id = resource->id.image;
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

                task.attachments[attach_i].value.image.view_ids = task.attachment_image_views[attach_i];
            }
        }
    }

    auto initialize_attachment_shader_blob(ImplTaskGraph & impl, ImplTask & task)
    {
        u64 current_offset = {};
        for (u32 attach_i = 0; attach_i < task.attachments.size(); ++attach_i)
        {
            TaskAttachmentInfo const & attachment_info = task.attachments[attach_i];
            u64 additional_size = {};
            switch (attachment_info.type)
            {
            case TaskAttachmentType::UNDEFINED:
                DAXA_DBG_ASSERT_TRUE_M(false, "IMPOSSIBLE CASE, POSSIBLY CAUSED BY DATA CORRUPTION!");
                break;
            case TaskAttachmentType::BUFFER:
            {
                if (attachment_info.value.buffer.shader_array_size == 0)
                {
                    task.attachment_shader_blob_sections[attach_i].buffer_address = {};
                    break;
                }
                ImplTaskResource const * resource = nullptr;
                if (!attachment_info.value.buffer.translated_view.is_null())
                {
                    resource = &impl.resources[attachment_info.value.buffer.translated_view.index];
                }
                if (attachment_info.value.buffer.shader_as_address)
                {
                    current_offset = align_up(current_offset, sizeof(DeviceAddress));
                    additional_size = sizeof(DeviceAddress);

                    DeviceAddress address = {};
                    if (resource != nullptr && resource->external == nullptr)
                    {
                        address = impl.info.device.device_address(resource->id.buffer).value();
                    }
                    task.attachment_shader_blob_sections[attach_i].buffer_address = reinterpret_cast<DeviceAddress *>(task.attachment_shader_blob.data() + current_offset);
                    *task.attachment_shader_blob_sections[attach_i].buffer_address = address;
                }
                else
                {
                    current_offset = align_up(current_offset, sizeof(BufferId));
                    additional_size = sizeof(BufferId);

                    BufferId id = {};
                    if (resource != nullptr && resource->external == nullptr)
                    {
                        id = resource->id.buffer;
                    }
                    *reinterpret_cast<BufferId *>(task.attachment_shader_blob.data() + current_offset) = id;
                    task.attachment_shader_blob_sections[attach_i].buffer_id = reinterpret_cast<BufferId *>(task.attachment_shader_blob.data() + current_offset);
                    *task.attachment_shader_blob_sections[attach_i].buffer_id = id;
                }
            }
            break;
            case TaskAttachmentType::TLAS:
            {
                if (attachment_info.value.tlas.shader_array_size == 0)
                {
                    task.attachment_shader_blob_sections[attach_i].tlas_id = {};
                    break;
                }
                ImplTaskResource const * resource = nullptr;
                if (!attachment_info.value.tlas.translated_view.is_null())
                {
                    resource = &impl.resources[attachment_info.value.tlas.translated_view.index];
                }
                if (attachment_info.value.tlas.shader_as_address)
                {
                    current_offset = align_up(current_offset, sizeof(DeviceAddress));
                    additional_size = sizeof(DeviceAddress);

                    DeviceAddress address = {};
                    if (resource != nullptr && resource->external == nullptr)
                    {
                        address = impl.info.device.device_address(resource->id.tlas).value();
                    }
                    task.attachment_shader_blob_sections[attach_i].tlas_address = reinterpret_cast<DeviceAddress *>(task.attachment_shader_blob.data() + current_offset);
                    *task.attachment_shader_blob_sections[attach_i].tlas_address = address;
                }
                else
                {
                    current_offset = align_up(current_offset, sizeof(TlasId));
                    additional_size = sizeof(TlasId);

                    TlasId id = {};
                    if (resource != nullptr && resource->external == nullptr)
                    {
                        id = resource->id.tlas;
                    }
                    task.attachment_shader_blob_sections[attach_i].tlas_id = reinterpret_cast<TlasId *>(task.attachment_shader_blob.data() + current_offset);
                    *task.attachment_shader_blob_sections[attach_i].tlas_id = id;
                }
            }
            break;
            case TaskAttachmentType::BLAS:
                DAXA_DBG_ASSERT_TRUE_M(false, "IMPOSSIBLE CASE! THIS STRONGLY INDICATES A DATA CORRUPTION OR UNINITIALIZED DATA!");
                break;
            case TaskAttachmentType::IMAGE:
            {
                if (attachment_info.value.image.shader_array_size == 0)
                {
                    task.attachment_shader_blob_sections[attach_i].image_view_ids = {};
                    break;
                }
                if (attachment_info.value.image.shader_as_index)
                {
                    current_offset = align_up(current_offset, sizeof(ImageViewIndex));
                    additional_size = sizeof(ImageViewIndex) * attachment_info.value.image.shader_array_size;
                }
                else
                {
                    current_offset = align_up(current_offset, sizeof(ImageViewId));
                    additional_size = sizeof(ImageViewId) * attachment_info.value.image.shader_array_size;
                }
                if (attachment_info.value.image.shader_as_index)
                {
                    task.attachment_shader_blob_sections[attach_i].image_view_indices = { reinterpret_cast<ImageViewIndex *>(task.attachment_shader_blob.data() + current_offset), attachment_info.value.image.shader_array_size };
                }
                else
                {
                    task.attachment_shader_blob_sections[attach_i].image_view_ids = { reinterpret_cast<ImageViewId *>(task.attachment_shader_blob.data() + current_offset), attachment_info.value.image.shader_array_size };
                }
                for (u32 i = 0; i < attachment_info.value.image.shader_array_size; ++i)
                {
                    ImageViewId id = task.attachment_image_views[attach_i][i];
                    if (attachment_info.value.image.shader_as_index)
                    {
                        task.attachment_shader_blob_sections[attach_i].image_view_indices[i] = static_cast<ImageViewIndex>(id.index);
                    }
                    else
                    {
                        task.attachment_shader_blob_sections[attach_i].image_view_ids[i] = id;
                    }
                }
            }
            break;
            }

            current_offset += additional_size;
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
        auto asb_section = task.attachment_shader_blob_sections[attach_i];

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
                *asb_section.buffer_address = impl.info.device.device_address(resource.id.buffer).value();
            }
            else
            {
                *asb_section.buffer_id = resource.id.buffer;
            }
        }
        break;
        case TaskAttachmentType::TLAS:
        {
            if (attachment_info.value.tlas.shader_array_size == 0)
            {
                break;
            }
            if (attachment_info.value.tlas.shader_as_address)
            {
                *asb_section.tlas_address = impl.info.device.device_address(resource.id.tlas).value();
            }
            else
            {
                *asb_section.tlas_id = resource.id.tlas;
            }
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
                    asb_section.image_view_indices[i] = static_cast<ImageViewIndex>(id.index);
                }
                else
                {
                    asb_section.image_view_ids[i] = id;
                }
            }
        }
        break;
        }
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

        queue = queue == QUEUE_NONE ? impl.info.default_queue : queue;

        std::span<std::span<ImageViewId>> attachment_image_views = impl.task_memory.allocate_trivial_span<std::span<ImageViewId>>(attachments.size());
        for (u32 attach_i = 0u; attach_i < attachments.size(); ++attach_i)
        {
            if (attachments[attach_i].type == TaskAttachmentType::IMAGE)
            {
                TaskImageAttachmentInfo const & attachment_info = attachments[attach_i].value.image;
                attachment_image_views[attach_i] = impl.task_memory.allocate_trivial_span<ImageViewId>(std::max(1u, static_cast<u32>(attachment_info.shader_array_size)));
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
            .attachment_resources = impl.task_memory.allocate_trivial_span<ImplTaskResource *>(attachments.size()),
            .attachment_access_groups = impl.task_memory.allocate_trivial_span<AccessGroup *>(attachments.size()),
            .attachment_shader_blob_size = attachment_shader_blob_size,
            .attachment_shader_blob_alignment = attachment_shader_blob_alignment,
            .attachment_shader_blob = impl.task_memory.allocate_trivial_span<std::byte>(attachment_shader_blob_size, attachment_shader_blob_alignment),
            .attachment_shader_blob_sections = impl.task_memory.allocate_trivial_span<AttachmentShaderBlobSection>(attachments.size()),
            .attachment_image_views = attachment_image_views,
            .task_type = task_type,
            .queue = queue,
            .submit_index = static_cast<u32>(impl.submits.size()),
            .runtime_images_last_execution = impl.task_memory.allocate_trivial_span_fill<ImageId>(attachments.size(), daxa::ImageId{}),
        };

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
            .first_task = static_cast<u32>(impl.tasks.size()) - tasks_since_last_submit,
            .task_count = tasks_since_last_submit,
            .queue_bits = 0u,
        });
    }

    void TaskGraph::present(TaskPresentInfo const & info)
    {
        ImplTaskGraph & impl = *r_cast<ImplTaskGraph *>(this->object);

        DAXA_DBG_ASSERT_TRUE_M(!impl.present.has_value(), "ERROR: A task graph can only record up to a single present!");
        DAXA_DBG_ASSERT_TRUE_M(impl.info.swapchain.has_value(), "ERROR: Can only record a present to a task graph that has a swapchain given on creation!");
        DAXA_DBG_ASSERT_TRUE_M(impl.submits.size() > 0, "ERROR: A task graph present can only be recorded AFTER one or more submits!");

        impl.present = TaskGraphPresent{
            .submit_index = static_cast<u32>(impl.submits.size()) - 1u,
            .queue = info.queue,
        };
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
        if (impl.present.has_value())
        {
            DAXA_DBG_ASSERT_TRUE_M(impl.swapchain_image != nullptr, "ERROR: No swapchain image registered but a present was recorded!");
        }

        // Validate that all attachments of each task refer to unique resources.
        for (u32 task_i = 0; task_i < impl.tasks.size(); ++task_i)
        {
            ImplTask & task = impl.tasks.at(task_i);
            for (u32 attach_i = 0; attach_i < task.attachments.size(); ++attach_i)
            {
                TaskAttachmentInfo const & attachment = task.attachments[attach_i];

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
            PipelineStageFlags stages = {};
            TaskAccessType type = {};
            u32 queue_bits = {};
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
                task.attachment_resources[attach_i] = nullptr;

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
                task.attachment_resources[attach_i] = resource;

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
                            .stages = {},
                            .type = access_type,
                            .queue_bits = 0u,
                            .tasks = ArenaDynamicArray8k<TaskAttachmentAccess>(&tmp_memory),
                        });
                    }

                    *latest_access_submit_index = task.submit_index;
                    access_timeline->back().tasks.push_back(TaskAttachmentAccess{&task, task_i, attach_i});
                    access_timeline->back().stages |= attachment_stages;

                    u32 queue_bit = (1u << queue_to_queue_index(task.queue)); // set queue bit to every relevant field:
                    // AccessGroup:
                    access_timeline->back().queue_bits |= queue_bit;
                    // Resource:
                    resource->queue_bits |= queue_bit;
                    // Submit:
                    impl.submits[task.submit_index].queue_bits |= queue_bit;
                    // TaskGraph:
                    impl.queue_bits |= queue_bit;
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
                dst_access_group.stages = src_access_timelines.at(ati).stages;
                dst_access_group.type = src_access_timelines.at(ati).type;
                dst_access_group.queue_bits = src_access_timelines.at(ati).queue_bits;
                dst_access_group.tasks = impl.task_memory.allocate_trivial_span<TaskAttachmentAccess>(src_access_timelines.at(ati).tasks.size());
                for (u32 t = 0; t < src_access_timelines.at(ati).tasks.size(); ++t)
                {
                    dst_access_group.tasks[t] = src_access_timelines.at(ati).tasks.at(t);
                    TaskAttachmentAccess & taa = dst_access_group.tasks[t];
                    taa.task->attachment_access_groups[taa.attachment_index] = &dst_access_group;
                }
            }
        }

        /// ============================================
        /// ==== VALIDATE SWAPCHAIN ACCESS TIMELINE ====
        /// ============================================

        if (impl.present.has_value())
        {
            DAXA_DBG_ASSERT_TRUE_M(impl.swapchain_image->access_timeline.size() > 0, "ERROR: When presenting a swapchain image, it MUST be used in the graph aside from the present itself!");
            DAXA_DBG_ASSERT_TRUE_M(impl.present->submit_index == impl.swapchain_image->access_timeline.back().tasks[0].task->submit_index, "ERROR: The present of a swapchain image MUST be directly after the last submit using the image!");
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

            if (&impl.resources[i] == impl.swapchain_image)
            {
                AccessGroup const & last_access_group = access_timeline.back();

                bool const is_last_multi_queue = std::popcount(last_access_group.queue_bits) > 1;
                bool const is_presented = impl.present.has_value();
                DAXA_DBG_ASSERT_TRUE_M(!(is_last_multi_queue && is_presented), "ERROR: Swapchain image's last access must not be multi queue concurrent when its presented in the task graph!");
            }

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

                current_submit_queue_bitfield |= access_group.queue_bits;
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

                        queue_string += std::string(daxa::to_string(queue_index_to_queue(first_significant_bit_idx)));
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
        /// ==== DETERMINE MIN SCHEDULE FOR ALL TASKS ====
        /// ==============================================

        // In the Min-Schedule, all tasks are inserted into batches as early as possible.
        // This is not optimal, but it allows us to quickly find a schedule with the minimal possible number of batches.
        // Once we have the Min-Schedule, we can continue optimizing the order of tasks that are not in the critical path.

        struct TmpBatch
        {
            ArenaDynamicArray8k<std::pair<ImplTask *, u32>> tasks = {};
            u32 queue_bits = {};
        };

        auto tmp_minsh_task_batches = ArenaDynamicArray8k<TmpBatch>(&tmp_memory);

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
                tmp_minsh_task_batches.push_back(TmpBatch{
                    .tasks = ArenaDynamicArray8k<std::pair<ImplTask *, u32>>(&tmp_memory),
                    .queue_bits = {},
                });
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
                tmp_minsh_task_batches.push_back(TmpBatch{.tasks = ArenaDynamicArray8k<std::pair<ImplTask *, u32>>(&tmp_memory), .queue_bits = {}});
            }
            tmp_minsh_task_batches.at(min_batch_index).tasks.push_back(std::pair{&task, task_i});
            tmp_minsh_task_batches.at(min_batch_index).queue_bits |= queue_index_to_queue_bit(queue_to_queue_index(task.queue));
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
            for (u32 t = 0; t < batch.tasks.size(); ++t)
            {
                u32 const task_index = batch.tasks[t].second;

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
                    auto allocation_resource_queue_access_identical = new_allocation.resource->queue_bits == other_allocation.resource->queue_bits;
                    auto allocations_used_across_multiple_queues = std::popcount(new_allocation.resource->queue_bits) > 1u || std::popcount(other_allocation.resource->queue_bits) > 1u;
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

        /// =============================================
        /// ==== STORE SUBMIT TASK BATCHES PER QUEUE ====
        /// =============================================

        // Go over all batches, calculate the batch count per queue per submit and allocate spans.
        for (u32 s = 0; s < impl.submits.size(); ++s)
        {
            TasksSubmit & submit = impl.submits[s];

            u32 const submit_first_batch = impl.tasks[submit.first_task].final_schedule_batch;
            u32 const submit_last_batch = impl.tasks[submit.first_task + submit.task_count - 1].final_schedule_batch;
            u32 const submit_batch_count = submit_last_batch - submit_first_batch + 1;
            impl.submits[s].first_batch = submit_first_batch;
            impl.submits[s].batch_count = submit_batch_count;

            // Calculate batch count per queue.
            std::array<u32, DAXA_MAX_TOTAL_QUEUE_COUNT> queue_batch_counts = {};
            for (u32 batch_i = 0; batch_i < submit_batch_count; ++batch_i)
            {
                u32 const global_batch_i = batch_i + submit_first_batch;
                auto const & tmp_batch = final_batches[global_batch_i];

                for (u32 q = 0; q < DAXA_MAX_TOTAL_QUEUE_COUNT; ++q)
                {
                    bool const queue_used = (tmp_batch.queue_bits & queue_index_to_queue_bit(q)) != 0;

                    if (queue_used)
                    {
                        DAXA_DBG_ASSERT_TRUE_M(
                            queue_batch_counts[q] == batch_i,
                            "IMPOSSIBLE CASE! "
                            "A CORE ASSUMPTION OF THE TASKGRAPH IS THAT ALL TASKS AND BATCHES ARE COMPACT AND NEVER LEAVE HOLES WITHIN A SUBMIT! "
                            "ALL BATCHES ON ALL QUEUES MUST START AT submit_first_batch AND END AT submit_first_batch AND END AT THE FINAL queue_batch_counts[q]!");
                        queue_batch_counts[q] += 1;
                    }
                }
            }

            // Create tmp queue batch task arrays.
            auto tmp_queue_batch_tasks = std::array<std::span<ArenaDynamicArray8k<std::pair<ImplTask *, u32>>>, DAXA_MAX_TOTAL_QUEUE_COUNT>{};
            for (u32 q = 0; q < DAXA_MAX_TOTAL_QUEUE_COUNT; ++q)
            {
                tmp_queue_batch_tasks[q] = tmp_memory.allocate_trivial_span<ArenaDynamicArray8k<std::pair<ImplTask *, u32>>>(queue_batch_counts[q]);
                for (u32 queue_batch_i = 0; queue_batch_i < queue_batch_counts[q]; ++queue_batch_i)
                {
                    tmp_queue_batch_tasks[q][queue_batch_i] = ArenaDynamicArray8k<std::pair<ImplTask *, u32>>(&tmp_memory);
                }
            }

            // Filter tasks from batches to corresponding queue batches.
            for (u32 batch_i = 0; batch_i < submit_batch_count; ++batch_i)
            {
                u32 const global_batch_i = batch_i + submit_first_batch;
                TmpBatch const & tmp_batch = final_batches[global_batch_i];

                for (u32 batch_task_i = 0; batch_task_i < tmp_batch.tasks.size(); ++batch_task_i)
                {
                    auto [task, task_i] = tmp_batch.tasks[batch_task_i];
                    auto const queue = task->queue;
                    auto const queue_index = queue_to_queue_index(queue);
                    tmp_queue_batch_tasks[queue_index][batch_i].push_back(std::pair{task, task_i});
                }
            }

            // Store created queue batches in submits:
            for (u32 q = 0; q < DAXA_MAX_TOTAL_QUEUE_COUNT; ++q)
            {
                submit.queue_batches[q] = impl.task_memory.allocate_trivial_span<TasksBatch>(queue_batch_counts[q]);

                for (u32 queue_batch_i = 0; queue_batch_i < queue_batch_counts[q]; ++queue_batch_i)
                {
                    submit.queue_batches[q][queue_batch_i].tasks = tmp_queue_batch_tasks[q][queue_batch_i].clone_to_contiguous(&impl.task_memory);
                    submit.queue_batches[q][queue_batch_i].pre_batch_barriers = {};
                    submit.queue_batches[q][queue_batch_i].pre_batch_image_barriers = {};
                }
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

        // While we need barriers between batches on a single queue, we do NOT need barriers between resource access of different queues, that are synchronized via semaphores.
        // Quote for semaphore signal operation:
        // > The first access scope includes all memory access performed by the device.
        // Quote for semaphore wait operation:
        // > The second access scope includes all memory access performed by the device.
        // https://vulkan.lunarg.com/doc/view/1.4.328.1/windows/antora/spec/latest/chapters/synchronization.html#synchronization-semaphores-signaling

        // Also, we mark all images used across queues as concurrent AND we perform very few if at all layout transitions, removing the need for inter queue image barriers as well.

        // Build temp barrier data structure.
        struct TmpBatchBarriers
        {
            ArenaDynamicArray8k<TaskBarrier> barriers = {};
            ArenaDynamicArray8k<TaskImageBarrier> image_barriers = {};
        };
        struct TmpSubmitBarriers
        {
            std::array<std::span<TmpBatchBarriers>, DAXA_MAX_TOTAL_QUEUE_COUNT> per_queue_batch_barriers = {};
        };
        auto tmp_submit_queue_batch_barriers = tmp_memory.allocate_trivial_span<TmpSubmitBarriers>(impl.submits.size());
        for (u32 submit_index = 0; submit_index < impl.submits.size(); ++submit_index)
        {
            TasksSubmit const & submit = impl.submits[submit_index];
            for (u32 queue_index = 0; queue_index < DAXA_MAX_TOTAL_QUEUE_COUNT; ++queue_index)
            {
                std::span<TasksBatch> batches = submit.queue_batches[queue_index];
                tmp_submit_queue_batch_barriers[submit_index].per_queue_batch_barriers[queue_index] = tmp_memory.allocate_trivial_span<TmpBatchBarriers>(batches.size());
                for (u32 queue_batch_i = 0; queue_batch_i < batches.size(); ++queue_batch_i)
                {
                    tmp_submit_queue_batch_barriers[submit_index].per_queue_batch_barriers[queue_index][queue_batch_i].barriers = ArenaDynamicArray8k<TaskBarrier>(&tmp_memory);
                    tmp_submit_queue_batch_barriers[submit_index].per_queue_batch_barriers[queue_index][queue_batch_i].image_barriers = ArenaDynamicArray8k<TaskImageBarrier>(&tmp_memory);
                }
            }
        }

        // All transient images have to be transformed from UNDEFINED to GENERAL layout before their first usage
        for (u32 tr = 0u; tr < transient_resources_sorted_by_lifetime.size(); ++tr)
        {
            ImplTaskResource & resource = *transient_resources_sorted_by_lifetime[tr];

            if (resource.access_timeline.size() == 0 || resource.kind != TaskResourceKind::IMAGE)
            {
                continue;
            }

            auto const & first_access_group = resource.access_timeline[0];
            DAXA_DBG_ASSERT_TRUE_M(std::popcount(first_access_group.queue_bits) == 1, "IMPOSSIBLE CASE! First image access can never be concurrent on two queues");
            auto const submit_index = first_access_group.tasks[0].task->submit_index;
            auto const queue_index = queue_bits_to_first_queue_index(first_access_group.queue_bits);
            auto const stages = first_access_group.stages;
            auto const access_type_flags = task_access_type_to_access_type(first_access_group.type);
            auto const first_use_batch = resource.final_schedule_first_batch;
            DAXA_DBG_ASSERT_TRUE_M(impl.submits[submit_index].first_batch <= first_access_group.final_schedule_first_batch, "IMPOSSIBLE CASE! COULD INDICATE ERROR IN SUBMIT CONSTRUCTION PHASE!");
            auto const submit_local_batch_index = first_access_group.final_schedule_first_batch - impl.submits[submit_index].first_batch;
            tmp_submit_queue_batch_barriers[submit_index].per_queue_batch_barriers[queue_index][submit_local_batch_index].image_barriers.push_back(TaskImageBarrier{
                .src_access_group = nullptr,
                .dst_access_group = &first_access_group,
                .src_access = AccessConsts::NONE,
                .dst_access = Access{stages, access_type_flags},
                .resource = &resource,
                .layout_operation = ImageLayoutOperation::TO_GENERAL,
            });
        }

        // Create barriers between each access group for each resource
        for (u32 r = 0; r < impl.resources.size(); ++r)
        {
            ImplTaskResource & resource = impl.resources[r];

            for (u32 ag = 1u; ag < resource.access_timeline.size(); ++ag)
            {
                AccessGroup & first_ag = resource.access_timeline[ag - 1];
                AccessGroup & second_ag = resource.access_timeline[ag];
                auto const first_ag_access = Access{first_ag.stages, task_access_type_to_access_type(first_ag.type)};
                auto const second_ag_access = Access{second_ag.stages, task_access_type_to_access_type(second_ag.type)};

                // If access 0 and access 1 are between two different queues, memory and execution dependencies are done via semaphores.
                // In this case, we generate no barrier at all.
                if (first_ag.tasks[0].task->submit_index != second_ag.tasks[0].task->submit_index)
                {
                    continue;
                }

                DAXA_DBG_ASSERT_TRUE_M(std::popcount(first_ag.queue_bits) == 1, "IMPOSSIBLE CASE! Can not insert barriers for resources used on multiple queues within the same submit!");
                DAXA_DBG_ASSERT_TRUE_M(std::popcount(second_ag.queue_bits) == 1, "IMPOSSIBLE CASE! Can not insert barriers for resources used on multiple queues within the same submit!");
                DAXA_DBG_ASSERT_TRUE_M(first_ag.queue_bits == second_ag.queue_bits, "IMPOSSIBLE CASE! Can not insert barriers for resources used on multiple queues within the same submit!");

                u32 const submit_index = second_ag.tasks[0].task->submit_index;
                u32 const queue_index = queue_bits_to_first_queue_index(second_ag.queue_bits);

                DAXA_DBG_ASSERT_TRUE_M(impl.submits[submit_index].first_batch <= second_ag.final_schedule_first_batch, "IMPOSSIBLE CASE! COULD INDICATE ERROR IN SUBMIT CONSTRUCTION PHASE!");
                auto const second_ag_submit_local_batch_index = second_ag.final_schedule_first_batch - impl.submits[submit_index].first_batch;

                // Add support for split barriers in the future.
                // Investigate smarter barrier insertion tactics.
                u32 submit_local_barrier_insertion_index = second_ag_submit_local_batch_index;

                if (impl.info.amd_rdna3_4_image_barrier_fix && resource.kind == TaskResourceKind::IMAGE)
                {
                    tmp_submit_queue_batch_barriers[submit_index].per_queue_batch_barriers[queue_index][submit_local_barrier_insertion_index].image_barriers.push_back(TaskImageBarrier{
                        .src_access_group = &first_ag,
                        .dst_access_group = &second_ag,
                        .src_access = first_ag_access,
                        .dst_access = second_ag_access,
                        .resource = &resource,
                    });
                }
                else
                {
                    tmp_submit_queue_batch_barriers[submit_index].per_queue_batch_barriers[queue_index][submit_local_barrier_insertion_index].barriers.push_back(TaskBarrier{
                        .src_access_group = &first_ag,
                        .dst_access_group = &second_ag,
                        .src_access = first_ag_access,
                        .dst_access = second_ag_access,
                        .resource = &resource,
                    });
                }
            }
        }

        /// ===============================================
        /// ==== STORE SUBMIT BATCH TASKS AND BARRIERS ====
        /// ===============================================

        for (u32 submit_index = 0; submit_index < impl.submits.size(); ++submit_index)
        {
            TasksSubmit & submit = impl.submits[submit_index];
            for (u32 queue_index = 0; queue_index < DAXA_MAX_TOTAL_QUEUE_COUNT; ++queue_index)
            {
                for (u32 queue_batch_i = 0; queue_batch_i < submit.queue_batches[queue_index].size(); ++queue_batch_i)
                {
                    submit.queue_batches[queue_index][queue_batch_i].pre_batch_barriers =
                        tmp_submit_queue_batch_barriers[submit_index].per_queue_batch_barriers[queue_index][queue_batch_i].barriers.clone_to_contiguous(&impl.task_memory);
                    submit.queue_batches[queue_index][queue_batch_i].pre_batch_image_barriers =
                        tmp_submit_queue_batch_barriers[submit_index].per_queue_batch_barriers[queue_index][queue_batch_i].image_barriers.clone_to_contiguous(&impl.task_memory);
                }
            }
        }

        /// ================================================
        /// ==== PREPARE TIGHT SUBMIT QUEUE INDEX LISTS ====
        /// ================================================

        // To avoid ugly bit iteration in execution, we generate a tight list of used queues for each submit.

        for (u32 s = 0; s < impl.submits.size(); ++s)
        {
            TasksSubmit & submit = impl.submits[s];

            u32 const queue_count = std::popcount(submit.queue_bits);
            submit.queue_indices = impl.task_memory.allocate_trivial_span<u32>(queue_count);

            // Fill tight list of signalled semaphores.
            u32 queue_iter = submit.queue_bits;
            u32 linear_index = 0;
            while (queue_iter)
            {
                u32 const queue_index = queue_bits_to_first_queue_index(queue_iter);
                queue_iter &= ~queue_index_to_queue_bit(queue_index);
                submit.queue_indices[linear_index] = queue_index;
                ++linear_index;
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
            ImplTask & task = impl.tasks[task_i];
            initialize_attachment_ids(impl, task);
            initialize_attachment_image_views(impl, task);
            initialize_attachment_shader_blob(impl, task);
        }

        /// =========================================
        /// ==== MAKE LIST OF EXTERNAL RESOURCES ====
        /// =========================================

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
        // All image views and attachment blobs are pre-created and kept between executions.
        // Only patch the outdated parts of attachment blobs and image view arrays when a external resource changed.

        // Memory and exeuction dependencies are done via semaphores.
        // Only image initializations require additional pipeline barriers.
        // Image initialization is inserted in the first submit for the queue that uses the image, before all other commands.
        auto image_initializations = tmp_memory.allocate_trivial_span<std::array<ArenaDynamicArray8k<ImageBarrierInfo>, DAXA_MAX_TOTAL_QUEUE_COUNT>>(impl.submits.size());
        for (u32 s = 0; s < impl.submits.size(); ++s)
        {
            for (u32 q = 0; q < DAXA_MAX_TOTAL_QUEUE_COUNT; ++q)
            {
                image_initializations[s][q] = ArenaDynamicArray8k<ImageBarrierInfo>(&tmp_memory);
            }
        }

        // Taskgraph performs a full barrier between all queues it uses at every submit.
        // Task graph finds all queues exterbal resources were used on prior to its exection.
        // It then constructs a dependency which synchronizes all queues found in the previous step with all queues used by this taskgraph.
        // This synchronization waits on all previously recorded commands to finish before any commands from this taskgraph are executed.
        u32 external_resource_queue_bits = {};

        // Validate the swapchain image we use was not yet presented to
        if (impl.swapchain_image)
        {
            bool const swapchain_image_unused = impl.swapchain_image->access_timeline.size() == 0 && !impl.present.has_value();
            DAXA_DBG_ASSERT_TRUE_M(!impl.swapchain_image->external->was_presented || swapchain_image_unused, "ERROR: The swapchain image was already presented to and can not be used in actions of this graph!");
        }

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
            ImplExternalResource * external = resource->external;
            switch (resource->kind)
            {
            case TaskResourceKind::BUFFER:
            {
                BufferId id = external->id.buffer;
                validate_id(id);
                did_id_change = resource->id.buffer != id;
                resource->id.buffer = id;
            }
            break;
            case TaskResourceKind::TLAS:
            {
                TlasId id = external->id.tlas;
                validate_id(id);
                did_id_change = resource->id.tlas != id;
                resource->id.tlas = id;
            }
            break;
            case TaskResourceKind::BLAS:
            {
                BlasId id = external->id.blas;
                validate_id(id);
                did_id_change = resource->id.blas != id;
                resource->id.blas = id;
            }
            break;
            case TaskResourceKind::IMAGE:
            {
                ImageId id = external->id.image;
                validate_id(id);
                did_id_change = resource->id.image != id;
                resource->id.image = id;
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

            // Determine synchronization needs
            if (resource->kind != TaskResourceKind::IMAGE)
            {
                external_resource_queue_bits |= external->pre_graph_queue_bits;
            }
            else
            {
                external_resource_queue_bits |= external->pre_graph_queue_bits;

                if (resource->access_timeline.size() > 0)
                {
                    // Validate edge case for multi queue access on images:
                    bool const transform_to_general = !external->pre_graph_is_general_layout;
                    bool const first_access_multi_queue_concurrent = std::popcount(resource->access_timeline[0].queue_bits) > 1;
                    bool const illegal_first_access = first_access_multi_queue_concurrent && transform_to_general;
                    DAXA_DBG_ASSERT_TRUE_M(
                        !illegal_first_access,
                        std::format(
                            "ERROR: External images that have a concurrent multi-queue first access MUST be initialized to layout general! "
                            "Note: This is very likely a bug, its very uncommon to not initialize and clear a image before concurrent multi queue use! "
                            "External image \"{}\" (index: {}) is NOT in layout general and its first access is concurrent on multiple queues in submit {} in task graph \"{}\"!",
                            resource->name, resource_i, resource->access_timeline[0].tasks[0].task->submit_index, impl.info.name)
                            .c_str());

                    if (transform_to_general)
                    {
                        AccessGroup const & first_access_group = resource->access_timeline[0];
                        u32 const first_access_submit = first_access_group.tasks[0].task->submit_index;
                        u32 const first_access_queue_index = queue_bits_to_first_queue_index(first_access_group.queue_bits);
                        image_initializations[first_access_submit][first_access_queue_index].push_back(ImageBarrierInfo{
                            .dst_access = Access{first_access_group.stages, to_access_type(first_access_group.type)},
                            .image_id = external->id.image,
                            .layout_operation = ImageLayoutOperation::TO_GENERAL,
                        });
                    }
                }
            }
        }

        /// ====================================
        /// ==== RECORD AND SUBMIT COMMANDS ====
        /// ====================================

        daxa::Device & device = impl.info.device;
        for (u32 submit_index = 0; submit_index < impl.submits.size(); ++submit_index)
        {
            TasksSubmit & submit = impl.submits[submit_index];

            // Inter Queue Sync
            // Build list of queue submit indices to wait on for the current submit.
            std::span<std::pair<Queue, u64>> wait_queue_submit_indices = {};
            if (submit_index == 0)
            {
                // In the first submission, we wait on all queues that touched external resource prior to this graph.
                u32 initial_wait_queue_bits = external_resource_queue_bits;
                wait_queue_submit_indices = tmp_memory.allocate_trivial_span<std::pair<Queue, u64>>(std::popcount(initial_wait_queue_bits));
                u32 queue_iter = initial_wait_queue_bits;
                u32 i = 0;
                while (queue_iter)
                {
                    u32 queue_index = queue_bits_to_first_queue_index(queue_iter);
                    Queue queue = queue_index_to_queue(queue_index);
                    queue_iter &= ~queue_index_to_queue_bit(queue_index);
                    wait_queue_submit_indices[i] = std::pair{queue, device.latest_queue_submit_index(queue)};
                    ++i;
                }
            }
            else
            {
                // For every following submission we wait on all queues used in the prior submission.
                TasksSubmit & previous_submit = impl.submits[submit_index - 1];

                // Add a wait on every queue used in the previous submit:
                wait_queue_submit_indices = tmp_memory.allocate_trivial_span<std::pair<Queue, u64>>(previous_submit.queue_indices.size());
                for (u32 qi = 0; qi < previous_submit.queue_indices.size(); ++qi)
                {
                    u32 queue_index = previous_submit.queue_indices[qi];
                    Queue queue = queue_index_to_queue(queue_index);
                    wait_queue_submit_indices[qi] = std::pair{queue, device.latest_queue_submit_index(queue)};
                }
            }

            for (u32 qi = 0; qi < submit.queue_indices.size(); ++qi)
            {
                /// =========================================
                /// ==== PREPARE QUEUE COMMAND RECORDING ====
                /// =========================================

                u32 queue_index = submit.queue_indices[qi];
                Queue queue = queue_index_to_queue(queue_index);
                std::array<BinarySemaphore, 256> wait_semaphores = {};
                u64 wait_semaphore_count = {};
                std::array<BinarySemaphore, 256> signal_semaphores = {};
                u64 signal_semaphore_count = {};
                std::array<std::pair<TimelineSemaphore, u64>, 256> signal_timeline_semaphores = {};
                u64 signal_timeline_semaphore_count = {};

                auto cr = device.create_command_recorder({
                    .queue_family = queue.family,
                    .name = std::format("\"{}\" submit {} queue \"{}\"", impl.info.name, submit_index, to_string(queue)),
                });

                ImplTaskRuntimeInterface impl_runtime{.task_graph = impl, .recorder = cr};

                // Add image initialization barriers.
                auto const & initialization_barriers = image_initializations[submit_index][queue_index];
                for (u32 ib = 0; ib < initialization_barriers.size(); ++ib)
                {
                    cr.pipeline_image_barrier(initialization_barriers[ib]);
                }

                // Record task batches and inter batch barriers:
                std::span<TasksBatch> batches = submit.queue_batches[queue_index];
                for (u32 batch_i = 0; batch_i < batches.size(); ++batch_i)
                {
                    TasksBatch const & batch = batches[batch_i];

                    /// ===================================
                    /// ==== RECORD PRE BATCH BARRIERS ====
                    /// ===================================

                    for (u32 b = 0; b < batch.pre_batch_barriers.size(); ++b)
                    {
                        cr.pipeline_barrier(BarrierInfo{
                            .src_access = batch.pre_batch_barriers[b].src_access,
                            .dst_access = batch.pre_batch_barriers[b].dst_access,
                        });
                    }
                    for (u32 b = 0; b < batch.pre_batch_image_barriers.size(); ++b)
                    {
                        TaskImageBarrier const & task_image_barrier = batch.pre_batch_image_barriers[b];
                        cr.pipeline_image_barrier(ImageBarrierInfo{
                            .src_access = task_image_barrier.src_access,
                            .dst_access = task_image_barrier.dst_access,
                            .image_id = task_image_barrier.resource->id.image,
                            .layout_operation = task_image_barrier.layout_operation,
                        });
                    }

                    // DEBUG FULL BARRIER
                    cr.pipeline_barrier(BarrierInfo{
                        .src_access = {.stages = PipelineStageFlagBits::ALL_COMMANDS, .type = AccessTypeFlagBits::READ_WRITE},
                        .dst_access = {.stages = PipelineStageFlagBits::ALL_COMMANDS, .type = AccessTypeFlagBits::READ_WRITE},
                    });

                    // Record Tasks
                    for (u32 batch_task_i = 0; batch_task_i < batch.tasks.size(); ++batch_task_i)
                    {
                        /// =====================
                        /// ==== RECORD TASK ====
                        /// =====================

                        u32 const task_i = batch.tasks[batch_task_i].second;
                        ImplTask & task = impl.tasks[task_i];

                        DAXA_DBG_ASSERT_TRUE_M(&task == batch.tasks[batch_task_i].first, "IMPOSSIBLE CASE! THIS INDICATES UNINITIALIZED MEMORY OR DATA CORRUPTION!");

                        // Validate Shader Attachment Blob
                        {
                            for (u32 attach_i = 0; attach_i < task.attachments.size(); ++attach_i)
                            {
                                bool const is_null_attachment = task.attachment_resources[attach_i] == nullptr;
                                AttachmentShaderBlobSection asb_section = task.attachment_shader_blob_sections[attach_i];
                                if (task.attachments[attach_i].type == TaskAttachmentType::BUFFER && !task.attachments[attach_i].value.buffer.shader_as_address && task.attachments[attach_i].value.buffer.shader_array_size > 0)
                                {
                                    DeviceAddress const real_address = is_null_attachment ? DeviceAddress{} : device.device_address(task.attachment_resources[attach_i]->id.buffer).value();
                                    DeviceAddress const test_address = *asb_section.buffer_address;
                                    DAXA_DBG_ASSERT_TRUE_M(real_address == test_address, "IMPOSSIBLE CASE! INVALID ATTACHMENT SHADER BLOB DATA DETECTED!");
                                }
                                if (task.attachments[attach_i].type == TaskAttachmentType::BUFFER && !task.attachments[attach_i].value.buffer.shader_as_address && task.attachments[attach_i].value.buffer.shader_array_size > 0)
                                {
                                    BufferId const real_id = is_null_attachment ? BufferId{} : task.attachment_resources[attach_i]->id.buffer;
                                    BufferId const test_id = *asb_section.buffer_id;
                                    DAXA_DBG_ASSERT_TRUE_M(real_id == test_id && device.is_id_valid(test_id), "IMPOSSIBLE CASE! INVALID ATTACHMENT SHADER BLOB DATA DETECTED!");
                                }
                                if (task.attachments[attach_i].type == TaskAttachmentType::TLAS && !task.attachments[attach_i].value.tlas.shader_as_address && task.attachments[attach_i].value.tlas.shader_array_size > 0)
                                {
                                    DeviceAddress const real_address = is_null_attachment ? DeviceAddress{} : device.device_address(task.attachment_resources[attach_i]->id.tlas).value();
                                    DeviceAddress const test_address = *asb_section.tlas_address;
                                    DAXA_DBG_ASSERT_TRUE_M(real_address == test_address, "IMPOSSIBLE CASE! INVALID ATTACHMENT SHADER BLOB DATA DETECTED!");
                                }
                                if (task.attachments[attach_i].type == TaskAttachmentType::TLAS && !task.attachments[attach_i].value.tlas.shader_as_address && task.attachments[attach_i].value.buffer.shader_array_size > 0)
                                {
                                    TlasId const real_id = is_null_attachment ? TlasId{} : task.attachment_resources[attach_i]->id.tlas;
                                    TlasId const test_id = *asb_section.tlas_id;
                                    DAXA_DBG_ASSERT_TRUE_M(real_id == test_id && device.is_id_valid(test_id), "IMPOSSIBLE CASE! INVALID ATTACHMENT SHADER BLOB DATA DETECTED!");
                                }
                                if (task.attachments[attach_i].type == TaskAttachmentType::IMAGE && !task.attachments[attach_i].value.image.translated_view.is_null())
                                {
                                    if (task.attachments[attach_i].value.image.shader_as_index)
                                    {
                                        for (u32 i = 0; i < asb_section.image_view_indices.size(); ++i)
                                        {
                                            ImageViewId const real_id = is_null_attachment ? ImageViewId{} : task.attachment_image_views[attach_i][i];
                                            u32 const real_index = real_id.index;
                                            u32 const test_index = asb_section.image_view_indices[i].value;
                                            DAXA_DBG_ASSERT_TRUE_M(real_index == test_index, "IMPOSSIBLE CASE! INVALID ATTACHMENT SHADER BLOB DATA DETECTED!");
                                        }
                                    }
                                    else
                                    {
                                        for (u32 i = 0; i < asb_section.image_view_ids.size(); ++i)
                                        {
                                            ImageViewId const real_id = is_null_attachment ? ImageViewId{} : task.attachment_image_views[attach_i][i];
                                            ImageViewId const test_id = asb_section.image_view_ids[i];
                                            DAXA_DBG_ASSERT_TRUE_M(real_id == test_id, "IMPOSSIBLE CASE! INVALID ATTACHMENT SHADER BLOB DATA DETECTED!");
                                        }
                                    }
                                }
                            }
                        }

                        if (impl.info.enable_command_labels)
                        {
                            static std::string tag = {};
                            tag.clear();
                            std::format_to(std::back_inserter(tag), "batch {} task {} \"{}\"", batch_i, batch_task_i, task.name);
                            SmallString stag = SmallString{tag};
                            impl_runtime.recorder.begin_label({
                                .label_color = impl.info.task_label_color,
                                .name = stag,
                            });
                        }
                        auto interface = TaskInterface{
                            .device = impl.info.device,
                            .recorder = impl_runtime.recorder,
                            .attachment_infos = task.attachments,
                            .allocator = impl.staging_memory.has_value() ? &impl.staging_memory.value() : nullptr,
                            .attachment_shader_blob = task.attachment_shader_blob,
                            .task_name = task.name,
                            .task_index = task_i,
                            .queue = queue,
                        };
                        if (impl.info.pre_task_callback)
                        {
                            impl.info.pre_task_callback(interface);
                        }
                        task.task_callback(interface, task.task_callback_memory);
                        if (impl.info.post_task_callback)
                        {
                            impl.info.post_task_callback(interface);
                        }
                        if (impl.info.enable_command_labels)
                        {
                            impl_runtime.recorder.end_label();
                        }
                    }
                }

                /// =====================================================================
                /// ==== ACQUIRE, PRESENT AND LAYOUT TRANSITION SWAPCHAIN IMAGE SYNC ====
                /// =====================================================================

                // Swapchain acquire sync:
                bool const swapchain_image_used_in_graph = impl.swapchain_image != nullptr && impl.swapchain_image->access_timeline.size() > 0;
                if (swapchain_image_used_in_graph)
                {
                    Queue const first_swapchain_use_queue = impl.swapchain_image->access_timeline[0].tasks[0].task->queue;
                    u32 const first_swapchain_use_submit = impl.swapchain_image->access_timeline[0].tasks[0].task->submit_index;

                    DAXA_DBG_ASSERT_TRUE_M(std::popcount(impl.swapchain_image->access_timeline[0].queue_bits) == 1, "IMPOSSIBLE CASE! WE SHOULD ASSSERT IN COMPLETION IN MULTI QUEUE VALIDATION THAT THE FIRST USE IS NOT QUEUE CONCURRENT!");
                    DAXA_DBG_ASSERT_TRUE_M(impl.info.swapchain.has_value(), "IMPOSSIBLE CASE! IF WE USE A SWAPCHAIN IMAGE WE MUST HAVE THE SWAPCHAIN IN THE GRAPH INFO!");

                    bool const wait_on_swapchain_acquire = first_swapchain_use_queue == queue && first_swapchain_use_submit == submit_index;
                    if (wait_on_swapchain_acquire)
                    {
                        wait_semaphores[wait_semaphore_count++] = impl.info.swapchain->current_acquire_semaphore();
                    }
                }

                // Swapchain present and frame sync:
                if (impl.present.has_value())
                {
                    DAXA_DBG_ASSERT_TRUE_M(swapchain_image_used_in_graph, "IMPOSSIBLE CASE! WHEN PRESENTING THE SWAPCHAIN IMAGE MUST BE USED IN THE GRAPH!");
                    DAXA_DBG_ASSERT_TRUE_M(impl.swapchain_image->access_timeline.back().tasks[0].task->submit_index == impl.present->submit_index, "IMPOSSIBLE CASE! PRESENT SUBMIT MUST BE THE SAME AS THE LAST ACCESS TO THE SWAPCHAIN IMAGE!");

                    AccessGroup const & last_access = impl.swapchain_image->access_timeline.back();
                    Queue const last_swapchain_use_queue = last_access.tasks[0].task->queue;
                    u32 const last_swapchain_use_submit = last_access.tasks[0].task->submit_index;

                    bool const is_last_access_queue_submit = last_swapchain_use_queue == queue && last_swapchain_use_submit == submit_index;
                    if (is_last_access_queue_submit)
                    {
                        signal_semaphores[signal_semaphore_count++] = impl.info.swapchain->current_present_semaphore();
                        signal_timeline_semaphores[signal_timeline_semaphore_count++] = impl.info.swapchain->current_timeline_pair();
                        cr.pipeline_image_barrier(ImageBarrierInfo{
                            .src_access = Access{last_access.stages, to_access_type(last_access.type)},
                            .image_id = impl.swapchain_image->id.image,
                            .layout_operation = ImageLayoutOperation::TO_PRESENT_SRC,
                        });
                    }
                }

                /// ================
                /// ==== SUBMIT ====
                /// ================

                auto submit_info = CommandSubmitInfo{};
                auto exec_commands = std::array{cr.complete_current_commands()};
                submit_info.command_lists = exec_commands,
                submit_info.queue = queue;
                submit_info.signal_binary_semaphores = {signal_semaphores.data(), signal_semaphore_count};
                submit_info.signal_timeline_semaphores = {signal_timeline_semaphores.data(), signal_timeline_semaphore_count};
                submit_info.wait_binary_semaphores = {wait_semaphores.data(), wait_semaphore_count};
                submit_info.wait_queue_submit_indices = wait_queue_submit_indices;
                device.submit_commands(submit_info);
            }

            /// =================
            /// ==== PRESENT ====
            /// =================

            bool const present_after_submit = impl.present.has_value() && impl.present->submit_index == submit_index;
            if (present_after_submit)
            {
                device.present_frame({
                    .wait_binary_semaphores = std::array{impl.info.swapchain->current_present_semaphore()},
                    .swapchain = impl.info.swapchain.value(),
                    .queue = impl.present->queue,
                });
            }
        }

        if (impl.staging_memory.has_value())
        {
            impl.staging_memory->reuse_memory_after_pending_submits();
        }

        /// =================================================
        /// ==== UPDATE PRE GRAPH EXTERNAL RESOURCE INFO ====
        /// =================================================

        // This has to happen after all execution is finished.
        // We need the pre graph state while executing, it needs to not be modified until the execution is finished.

        for (u32 er = 0; er < impl.external_resources.size(); ++er)
        {
            auto [resource, resource_i] = impl.external_resources[er];
            ImplExternalResource * external = resource->external;

            if (resource == impl.swapchain_image && impl.present.has_value())
            {
                // Swapchain images that get presented to are in present src layout.
                external->pre_graph_is_general_layout = false;
                external->was_presented = true;
            }
            else
            {
                if (resource->access_timeline.size() > 0)
                {
                    external->pre_graph_is_general_layout = true;
                    external->pre_graph_queue_bits = resource->access_timeline.back().queue_bits;
                }
            }
        }
    }

    ImplTaskGraph::ImplTaskGraph(TaskGraphInfo a_info)
        : unique_index{ImplTaskGraph::exec_unique_next_index++}, info{std::move(a_info)},
          task_memory{"TaskGraph task memory pool", a_info.task_memory_pool_size}
    {
        info.name = task_memory.allocate_copy_string(info.name);
        tasks = ArenaDynamicArray8k<ImplTask>(&task_memory);
        resources = ArenaDynamicArray8k<ImplTaskResource>(&task_memory);
        submits = ArenaDynamicArray8k<TasksSubmit>(&task_memory);
        if (a_info.staging_memory_pool_size != 0)
        {
            this->staging_memory = TransferMemoryPool{TransferMemoryPoolInfo{.device = info.device, .capacity = info.staging_memory_pool_size, .name = "Transfer Memory Pool"}};
        }
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
                    if (this->info.device.is_id_valid(views[view_i]))
                    {
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
                    static_cast<ImplExternalResource *>(resource.external)->dec_refcnt(ImplExternalResource::zero_ref_callback, nullptr);
                }
                else
                {
                    static_cast<ImplExternalResource *>(resource.external)->dec_refcnt(ImplExternalResource::zero_ref_callback, nullptr);
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