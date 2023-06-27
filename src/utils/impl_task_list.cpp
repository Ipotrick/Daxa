#if DAXA_BUILT_WITH_UTILS_TASK_LIST

#include <algorithm>
#include <iostream>
#include <set>

#include <fstream>
#include <utility>

#include "impl_task_list.hpp"
#include "impl_task_list_debug.hpp"

namespace daxa
{
    auto to_string(TaskGPUResourceHandle const & id) -> std::string
    {
        return fmt::format("tlidx: {}, index: {}", id.task_list_index, id.index);
    }

    auto get_task_arg_shader_alignment(TaskResourceUseType type) -> u32
    {
        if (type == TaskResourceUseType::BUFFER)
        {
            return 8;
        }
        return 4;
    }

    auto get_task_arg_shader_offsets_size(std::span<GenericTaskResourceUse> args) -> std::pair<std::vector<u32>, u32>
    {
        std::vector<u32> ret = {};
        ret.reserve(args.size());
        u32 offset = 0;
        for (auto const & arg : args)
        {
            auto const align = get_task_arg_shader_alignment(arg.type);
            offset = (offset + align - 1) / align * align;
            ret.push_back(offset);
            offset += align;
        }
        // Final offset is equal to total size.
        return {ret, offset};
    }

    auto static constexpr access_to_usage(TaskImageAccess const & access) -> ImageUsageFlags
    {
        switch (access)
        {
        case TaskImageAccess::SHADER_READ: [[fallthrough]];
        case TaskImageAccess::VERTEX_SHADER_READ: [[fallthrough]];
        case TaskImageAccess::TESSELLATION_CONTROL_SHADER_READ: [[fallthrough]];
        case TaskImageAccess::TESSELLATION_EVALUATION_SHADER_READ: [[fallthrough]];
        case TaskImageAccess::GEOMETRY_SHADER_READ: [[fallthrough]];
        case TaskImageAccess::FRAGMENT_SHADER_READ: [[fallthrough]];
        case TaskImageAccess::COMPUTE_SHADER_READ:
            return ImageUsageFlagBits::SHADER_READ_ONLY;
        case TaskImageAccess::SHADER_WRITE: [[fallthrough]];
        case TaskImageAccess::VERTEX_SHADER_WRITE: [[fallthrough]];
        case TaskImageAccess::TESSELLATION_CONTROL_SHADER_WRITE: [[fallthrough]];
        case TaskImageAccess::TESSELLATION_EVALUATION_SHADER_WRITE: [[fallthrough]];
        case TaskImageAccess::GEOMETRY_SHADER_WRITE: [[fallthrough]];
        case TaskImageAccess::FRAGMENT_SHADER_WRITE: [[fallthrough]];
        case TaskImageAccess::COMPUTE_SHADER_WRITE: [[fallthrough]];
        case TaskImageAccess::SHADER_READ_WRITE: [[fallthrough]];
        case TaskImageAccess::TESSELLATION_CONTROL_SHADER_READ_WRITE: [[fallthrough]];
        case TaskImageAccess::TESSELLATION_EVALUATION_SHADER_READ_WRITE: [[fallthrough]];
        case TaskImageAccess::GEOMETRY_SHADER_READ_WRITE: [[fallthrough]];
        case TaskImageAccess::FRAGMENT_SHADER_READ_WRITE: [[fallthrough]];
        case TaskImageAccess::COMPUTE_SHADER_READ_WRITE:
            return ImageUsageFlagBits::SHADER_READ_WRITE;
        case TaskImageAccess::TRANSFER_READ:
            return ImageUsageFlagBits::TRANSFER_SRC;
        case TaskImageAccess::TRANSFER_WRITE:
            return ImageUsageFlagBits::TRANSFER_DST;
        // NOTE(msakmary) - not fully sure about the resolve being color attachment usage
        // this is the best I could deduce from vulkan docs
        case TaskImageAccess::RESOLVE_WRITE: [[fallthrough]];
        case TaskImageAccess::COLOR_ATTACHMENT:
            return ImageUsageFlagBits::COLOR_ATTACHMENT;
        case TaskImageAccess::DEPTH_ATTACHMENT: [[fallthrough]];
        case TaskImageAccess::STENCIL_ATTACHMENT: [[fallthrough]];
        case TaskImageAccess::DEPTH_STENCIL_ATTACHMENT:
            return ImageUsageFlagBits::DEPTH_STENCIL_ATTACHMENT;
        case TaskImageAccess::DEPTH_ATTACHMENT_READ: [[fallthrough]];
        case TaskImageAccess::STENCIL_ATTACHMENT_READ: [[fallthrough]];
        case TaskImageAccess::DEPTH_STENCIL_ATTACHMENT_READ:
            return ImageUsageFlagBits::DEPTH_STENCIL_ATTACHMENT;
        case TaskImageAccess::PRESENT: [[fallthrough]];
        case TaskImageAccess::NONE: [[fallthrough]];
        default:
            return ImageUsageFlagBits::NONE;
        }
    }

    auto task_image_access_to_layout_access(TaskImageAccess const & access) -> std::tuple<ImageLayout, Access>
    {
        switch (access)
        {
        case TaskImageAccess::NONE: return {ImageLayout::UNDEFINED, {PipelineStageFlagBits::NONE, AccessTypeFlagBits::NONE}};
        case TaskImageAccess::SHADER_READ: return {ImageLayout::READ_ONLY_OPTIMAL, {PipelineStageFlagBits::ALL_GRAPHICS | PipelineStageFlagBits::COMPUTE_SHADER, AccessTypeFlagBits::READ}};
        case TaskImageAccess::VERTEX_SHADER_READ: return {ImageLayout::READ_ONLY_OPTIMAL, {PipelineStageFlagBits::VERTEX_SHADER, AccessTypeFlagBits::READ}};
        case TaskImageAccess::TESSELLATION_CONTROL_SHADER_READ: return {ImageLayout::READ_ONLY_OPTIMAL, {PipelineStageFlagBits::TESSELLATION_CONTROL_SHADER, AccessTypeFlagBits::READ}};
        case TaskImageAccess::TESSELLATION_EVALUATION_SHADER_READ: return {ImageLayout::READ_ONLY_OPTIMAL, {PipelineStageFlagBits::TESSELLATION_EVALUATION_SHADER, AccessTypeFlagBits::READ}};
        case TaskImageAccess::GEOMETRY_SHADER_READ: return {ImageLayout::READ_ONLY_OPTIMAL, {PipelineStageFlagBits::GEOMETRY_SHADER, AccessTypeFlagBits::READ}};
        case TaskImageAccess::FRAGMENT_SHADER_READ: return {ImageLayout::READ_ONLY_OPTIMAL, {PipelineStageFlagBits::FRAGMENT_SHADER, AccessTypeFlagBits::READ}};
        case TaskImageAccess::COMPUTE_SHADER_READ: return {ImageLayout::READ_ONLY_OPTIMAL, {PipelineStageFlagBits::COMPUTE_SHADER, AccessTypeFlagBits::READ}};
        case TaskImageAccess::SHADER_WRITE: return {ImageLayout::GENERAL, {PipelineStageFlagBits::PipelineStageFlagBits::ALL_GRAPHICS | PipelineStageFlagBits::COMPUTE_SHADER, AccessTypeFlagBits::WRITE}};
        case TaskImageAccess::VERTEX_SHADER_WRITE: return {ImageLayout::GENERAL, {PipelineStageFlagBits::VERTEX_SHADER, AccessTypeFlagBits::WRITE}};
        case TaskImageAccess::TESSELLATION_CONTROL_SHADER_WRITE: return {ImageLayout::GENERAL, {PipelineStageFlagBits::TESSELLATION_CONTROL_SHADER, AccessTypeFlagBits::WRITE}};
        case TaskImageAccess::TESSELLATION_EVALUATION_SHADER_WRITE: return {ImageLayout::GENERAL, {PipelineStageFlagBits::TESSELLATION_EVALUATION_SHADER, AccessTypeFlagBits::WRITE}};
        case TaskImageAccess::GEOMETRY_SHADER_WRITE: return {ImageLayout::GENERAL, {PipelineStageFlagBits::GEOMETRY_SHADER, AccessTypeFlagBits::WRITE}};
        case TaskImageAccess::FRAGMENT_SHADER_WRITE: return {ImageLayout::GENERAL, {PipelineStageFlagBits::FRAGMENT_SHADER, AccessTypeFlagBits::WRITE}};
        case TaskImageAccess::COMPUTE_SHADER_WRITE: return {ImageLayout::GENERAL, {PipelineStageFlagBits::COMPUTE_SHADER, AccessTypeFlagBits::WRITE}};
        case TaskImageAccess::SHADER_READ_WRITE: return {ImageLayout::GENERAL, {PipelineStageFlagBits::PipelineStageFlagBits::ALL_GRAPHICS | PipelineStageFlagBits::COMPUTE_SHADER, AccessTypeFlagBits::READ_WRITE}};
        case TaskImageAccess::VERTEX_SHADER_READ_WRITE: return {ImageLayout::GENERAL, {PipelineStageFlagBits::VERTEX_SHADER, AccessTypeFlagBits::READ_WRITE}};
        case TaskImageAccess::TESSELLATION_CONTROL_SHADER_READ_WRITE: return {ImageLayout::GENERAL, {PipelineStageFlagBits::TESSELLATION_CONTROL_SHADER, AccessTypeFlagBits::READ_WRITE}};
        case TaskImageAccess::TESSELLATION_EVALUATION_SHADER_READ_WRITE: return {ImageLayout::GENERAL, {PipelineStageFlagBits::TESSELLATION_EVALUATION_SHADER, AccessTypeFlagBits::READ_WRITE}};
        case TaskImageAccess::GEOMETRY_SHADER_READ_WRITE: return {ImageLayout::GENERAL, {PipelineStageFlagBits::GEOMETRY_SHADER, AccessTypeFlagBits::READ_WRITE}};
        case TaskImageAccess::FRAGMENT_SHADER_READ_WRITE: return {ImageLayout::GENERAL, {PipelineStageFlagBits::FRAGMENT_SHADER, AccessTypeFlagBits::READ_WRITE}};
        case TaskImageAccess::COMPUTE_SHADER_READ_WRITE: return {ImageLayout::GENERAL, {PipelineStageFlagBits::COMPUTE_SHADER, AccessTypeFlagBits::READ_WRITE}};
        case TaskImageAccess::TRANSFER_READ: return {ImageLayout::TRANSFER_SRC_OPTIMAL, {PipelineStageFlagBits::TRANSFER, AccessTypeFlagBits::READ}};
        case TaskImageAccess::TRANSFER_WRITE: return {ImageLayout::TRANSFER_DST_OPTIMAL, {PipelineStageFlagBits::TRANSFER, AccessTypeFlagBits::WRITE}};
        case TaskImageAccess::COLOR_ATTACHMENT: return {ImageLayout::ATTACHMENT_OPTIMAL, {PipelineStageFlagBits::COLOR_ATTACHMENT_OUTPUT, AccessTypeFlagBits::READ_WRITE}};
        case TaskImageAccess::DEPTH_ATTACHMENT:
            [[fallthrough]];
        case TaskImageAccess::STENCIL_ATTACHMENT:
            [[fallthrough]];
        case TaskImageAccess::DEPTH_STENCIL_ATTACHMENT: return {ImageLayout::ATTACHMENT_OPTIMAL, {PipelineStageFlagBits::EARLY_FRAGMENT_TESTS | PipelineStageFlagBits::LATE_FRAGMENT_TESTS, AccessTypeFlagBits::READ_WRITE}};
        case TaskImageAccess::DEPTH_ATTACHMENT_READ:
            [[fallthrough]];
        case TaskImageAccess::STENCIL_ATTACHMENT_READ:
            [[fallthrough]];
        case TaskImageAccess::DEPTH_STENCIL_ATTACHMENT_READ: return {ImageLayout::READ_ONLY_OPTIMAL, {PipelineStageFlagBits::EARLY_FRAGMENT_TESTS | PipelineStageFlagBits::LATE_FRAGMENT_TESTS, AccessTypeFlagBits::READ}};
        case TaskImageAccess::RESOLVE_WRITE: return {ImageLayout::ATTACHMENT_OPTIMAL, {PipelineStageFlagBits::RESOLVE, AccessTypeFlagBits::WRITE}};
        case TaskImageAccess::PRESENT: return {ImageLayout::PRESENT_SRC, {PipelineStageFlagBits::ALL_COMMANDS, AccessTypeFlagBits::READ}};
        default: DAXA_DBG_ASSERT_TRUE_M(false, "unreachable");
        }
        return {};
    }

    auto task_buffer_access_to_access(TaskBufferAccess const & access) -> Access
    {
        switch (access)
        {
        case TaskBufferAccess::NONE: return {PipelineStageFlagBits::NONE, AccessTypeFlagBits::NONE};
        case TaskBufferAccess::SHADER_READ: return {PipelineStageFlagBits::ALL_GRAPHICS | PipelineStageFlagBits::COMPUTE_SHADER, AccessTypeFlagBits::READ};
        case TaskBufferAccess::VERTEX_SHADER_READ: return {PipelineStageFlagBits::VERTEX_SHADER, AccessTypeFlagBits::READ};
        case TaskBufferAccess::TESSELLATION_CONTROL_SHADER_READ: return {PipelineStageFlagBits::TESSELLATION_CONTROL_SHADER, AccessTypeFlagBits::READ};
        case TaskBufferAccess::TESSELLATION_EVALUATION_SHADER_READ: return {PipelineStageFlagBits::TESSELLATION_EVALUATION_SHADER, AccessTypeFlagBits::READ};
        case TaskBufferAccess::GEOMETRY_SHADER_READ: return {PipelineStageFlagBits::GEOMETRY_SHADER, AccessTypeFlagBits::READ};
        case TaskBufferAccess::FRAGMENT_SHADER_READ: return {PipelineStageFlagBits::FRAGMENT_SHADER, AccessTypeFlagBits::READ};
        case TaskBufferAccess::COMPUTE_SHADER_READ: return {PipelineStageFlagBits::COMPUTE_SHADER, AccessTypeFlagBits::READ};
        case TaskBufferAccess::SHADER_WRITE: return {PipelineStageFlagBits::PipelineStageFlagBits::ALL_GRAPHICS | PipelineStageFlagBits::COMPUTE_SHADER, AccessTypeFlagBits::WRITE};
        case TaskBufferAccess::VERTEX_SHADER_WRITE: return {PipelineStageFlagBits::VERTEX_SHADER, AccessTypeFlagBits::WRITE};
        case TaskBufferAccess::TESSELLATION_CONTROL_SHADER_WRITE: return {PipelineStageFlagBits::TESSELLATION_CONTROL_SHADER, AccessTypeFlagBits::WRITE};
        case TaskBufferAccess::TESSELLATION_EVALUATION_SHADER_WRITE: return {PipelineStageFlagBits::TESSELLATION_EVALUATION_SHADER, AccessTypeFlagBits::WRITE};
        case TaskBufferAccess::GEOMETRY_SHADER_WRITE: return {PipelineStageFlagBits::GEOMETRY_SHADER, AccessTypeFlagBits::WRITE};
        case TaskBufferAccess::FRAGMENT_SHADER_WRITE: return {PipelineStageFlagBits::FRAGMENT_SHADER, AccessTypeFlagBits::WRITE};
        case TaskBufferAccess::COMPUTE_SHADER_WRITE: return {PipelineStageFlagBits::COMPUTE_SHADER, AccessTypeFlagBits::WRITE};
        case TaskBufferAccess::SHADER_READ_WRITE: return {PipelineStageFlagBits::PipelineStageFlagBits::ALL_GRAPHICS | PipelineStageFlagBits::COMPUTE_SHADER, AccessTypeFlagBits::READ_WRITE};
        case TaskBufferAccess::VERTEX_SHADER_READ_WRITE: return {PipelineStageFlagBits::VERTEX_SHADER, AccessTypeFlagBits::READ_WRITE};
        case TaskBufferAccess::TESSELLATION_CONTROL_SHADER_READ_WRITE: return {PipelineStageFlagBits::TESSELLATION_CONTROL_SHADER, AccessTypeFlagBits::READ_WRITE};
        case TaskBufferAccess::TESSELLATION_EVALUATION_SHADER_READ_WRITE: return {PipelineStageFlagBits::TESSELLATION_EVALUATION_SHADER, AccessTypeFlagBits::READ_WRITE};
        case TaskBufferAccess::GEOMETRY_SHADER_READ_WRITE: return {PipelineStageFlagBits::GEOMETRY_SHADER, AccessTypeFlagBits::READ_WRITE};
        case TaskBufferAccess::FRAGMENT_SHADER_READ_WRITE: return {PipelineStageFlagBits::FRAGMENT_SHADER, AccessTypeFlagBits::READ_WRITE};
        case TaskBufferAccess::COMPUTE_SHADER_READ_WRITE: return {PipelineStageFlagBits::COMPUTE_SHADER, AccessTypeFlagBits::READ_WRITE};
        case TaskBufferAccess::TRANSFER_READ: return {PipelineStageFlagBits::TRANSFER, AccessTypeFlagBits::READ};
        case TaskBufferAccess::TRANSFER_WRITE: return {PipelineStageFlagBits::TRANSFER, AccessTypeFlagBits::WRITE};
        case TaskBufferAccess::HOST_TRANSFER_READ: return {PipelineStageFlagBits::HOST, AccessTypeFlagBits::READ};
        case TaskBufferAccess::HOST_TRANSFER_WRITE: return {PipelineStageFlagBits::HOST, AccessTypeFlagBits::WRITE};
        case TaskBufferAccess::INDEX_READ: return {PipelineStageFlagBits::INDEX_INPUT, AccessTypeFlagBits::READ};
        case TaskBufferAccess::DRAW_INDIRECT_INFO_READ: return {PipelineStageFlagBits::DRAW_INDIRECT, AccessTypeFlagBits::READ};
        default: DAXA_DBG_ASSERT_TRUE_M(false, "unreachable");
        }
        return {};
    }

    auto TaskGPUResourceHandle::is_empty() const -> bool
    {
        return index == 0 && task_list_index == 0;
    }

    auto TaskGPUResourceHandle::is_persistent() const -> bool
    {
        return task_list_index == std::numeric_limits<u32>::max();
    }

    auto to_string(TaskBufferAccess const & usage) -> std::string_view
    {
        switch (usage)
        {
        case TaskBufferAccess::SHADER_READ: return std::string_view{"SHADER_READ"};
        case TaskBufferAccess::VERTEX_SHADER_READ: return std::string_view{"VERTEX_SHADER_READ"};
        case TaskBufferAccess::TESSELLATION_CONTROL_SHADER_READ: return std::string_view{"TESSELLATION_CONTROL_SHADER_READ"};
        case TaskBufferAccess::TESSELLATION_EVALUATION_SHADER_READ: return std::string_view{"TESSELLATION_EVALUATION_SHADER_READ"};
        case TaskBufferAccess::GEOMETRY_SHADER_READ: return std::string_view{"GEOMETRY_SHADER_READ"};
        case TaskBufferAccess::FRAGMENT_SHADER_READ: return std::string_view{"FRAGMENT_SHADER_READ"};
        case TaskBufferAccess::COMPUTE_SHADER_READ: return std::string_view{"COMPUTE_SHADER_READ"};
        case TaskBufferAccess::SHADER_WRITE: return std::string_view{"SHADER_WRITE"};
        case TaskBufferAccess::VERTEX_SHADER_WRITE: return std::string_view{"VERTEX_SHADER_WRITE"};
        case TaskBufferAccess::TESSELLATION_CONTROL_SHADER_WRITE: return std::string_view{"TESSELLATION_CONTROL_SHADER_WRITE"};
        case TaskBufferAccess::TESSELLATION_EVALUATION_SHADER_WRITE: return std::string_view{"TESSELLATION_EVALUATION_SHADER_WRITE"};
        case TaskBufferAccess::GEOMETRY_SHADER_WRITE: return std::string_view{"GEOMETRY_SHADER_WRITE"};
        case TaskBufferAccess::FRAGMENT_SHADER_WRITE: return std::string_view{"FRAGMENT_SHADER_WRITE"};
        case TaskBufferAccess::COMPUTE_SHADER_WRITE: return std::string_view{"COMPUTE_SHADER_WRITE"};
        case TaskBufferAccess::SHADER_READ_WRITE: return std::string_view{"SHADER_READ_WRITE"};
        case TaskBufferAccess::VERTEX_SHADER_READ_WRITE: return std::string_view{"VERTEX_SHADER_READ_WRITE"};
        case TaskBufferAccess::TESSELLATION_CONTROL_SHADER_READ_WRITE: return std::string_view{"TESSELLATION_CONTROL_SHADER_READ_WRITE"};
        case TaskBufferAccess::TESSELLATION_EVALUATION_SHADER_READ_WRITE: return std::string_view{"TESSELLATION_EVALUATION_SHADER_READ_WRITE"};
        case TaskBufferAccess::GEOMETRY_SHADER_READ_WRITE: return std::string_view{"GEOMETRY_SHADER_READ_WRITE"};
        case TaskBufferAccess::FRAGMENT_SHADER_READ_WRITE: return std::string_view{"FRAGMENT_SHADER_READ_WRITE"};
        case TaskBufferAccess::COMPUTE_SHADER_READ_WRITE: return std::string_view{"COMPUTE_SHADER_READ_WRITE"};
        case TaskBufferAccess::TRANSFER_READ: return std::string_view{"TRANSFER_READ"};
        case TaskBufferAccess::TRANSFER_WRITE: return std::string_view{"TRANSFER_WRITE"};
        case TaskBufferAccess::HOST_TRANSFER_READ: return std::string_view{"HOST_TRANSFER_READ"};
        case TaskBufferAccess::HOST_TRANSFER_WRITE: return std::string_view{"HOST_TRANSFER_WRITE"};
        case TaskBufferAccess::INDEX_READ: return std::string_view{"INDEX_READ"};
        case TaskBufferAccess::DRAW_INDIRECT_INFO_READ: return std::string_view{"DRAW_INDIRECT_INFO_READ"};
        default: DAXA_DBG_ASSERT_TRUE_M(false, "unreachable");
        }
        return "invalid";
    }

    auto to_string(TaskImageAccess const & usage) -> std::string_view
    {
        switch (usage)
        {
        case TaskImageAccess::SHADER_READ: return std::string_view{"SHADER_READ"};
        case TaskImageAccess::VERTEX_SHADER_READ: return std::string_view{"VERTEX_SHADER_READ"};
        case TaskImageAccess::TESSELLATION_CONTROL_SHADER_READ: return std::string_view{"TESSELLATION_CONTROL_SHADER_READ"};
        case TaskImageAccess::TESSELLATION_EVALUATION_SHADER_READ: return std::string_view{"TESSELLATION_EVALUATION_SHADER_READ"};
        case TaskImageAccess::GEOMETRY_SHADER_READ: return std::string_view{"GEOMETRY_SHADER_READ"};
        case TaskImageAccess::FRAGMENT_SHADER_READ: return std::string_view{"FRAGMENT_SHADER_READ"};
        case TaskImageAccess::COMPUTE_SHADER_READ: return std::string_view{"COMPUTE_SHADER_READ"};
        case TaskImageAccess::SHADER_WRITE: return std::string_view{"SHADER_WRITE"};
        case TaskImageAccess::VERTEX_SHADER_WRITE: return std::string_view{"VERTEX_SHADER_WRITE"};
        case TaskImageAccess::TESSELLATION_CONTROL_SHADER_WRITE: return std::string_view{"TESSELLATION_CONTROL_SHADER_WRITE"};
        case TaskImageAccess::TESSELLATION_EVALUATION_SHADER_WRITE: return std::string_view{"TESSELLATION_EVALUATION_SHADER_WRITE"};
        case TaskImageAccess::GEOMETRY_SHADER_WRITE: return std::string_view{"GEOMETRY_SHADER_WRITE"};
        case TaskImageAccess::FRAGMENT_SHADER_WRITE: return std::string_view{"FRAGMENT_SHADER_WRITE"};
        case TaskImageAccess::COMPUTE_SHADER_WRITE: return std::string_view{"COMPUTE_SHADER_WRITE"};
        case TaskImageAccess::SHADER_READ_WRITE: return std::string_view{"SHADER_READ_WRITE"};
        case TaskImageAccess::VERTEX_SHADER_READ_WRITE: return std::string_view{"VERTEX_SHADER_READ_WRITE"};
        case TaskImageAccess::TESSELLATION_CONTROL_SHADER_READ_WRITE: return std::string_view{"TESSELLATION_CONTROL_SHADER_READ_WRITE"};
        case TaskImageAccess::TESSELLATION_EVALUATION_SHADER_READ_WRITE: return std::string_view{"TESSELLATION_EVALUATION_SHADER_READ_WRITE"};
        case TaskImageAccess::GEOMETRY_SHADER_READ_WRITE: return std::string_view{"GEOMETRY_SHADER_READ_WRITE"};
        case TaskImageAccess::FRAGMENT_SHADER_READ_WRITE: return std::string_view{"FRAGMENT_SHADER_READ_WRITE"};
        case TaskImageAccess::COMPUTE_SHADER_READ_WRITE: return std::string_view{"COMPUTE_SHADER_READ_WRITE"};
        case TaskImageAccess::TRANSFER_READ: return std::string_view{"TRANSFER_READ"};
        case TaskImageAccess::TRANSFER_WRITE: return std::string_view{"TRANSFER_WRITE"};
        case TaskImageAccess::COLOR_ATTACHMENT: return std::string_view{"COLOR_ATTACHMENT"};
        case TaskImageAccess::DEPTH_ATTACHMENT: return std::string_view{"DEPTH_ATTACHMENT"};
        case TaskImageAccess::STENCIL_ATTACHMENT: return std::string_view{"STENCIL_ATTACHMENT"};
        case TaskImageAccess::DEPTH_STENCIL_ATTACHMENT: return std::string_view{"DEPTH_STENCIL_ATTACHMENT"};
        case TaskImageAccess::DEPTH_ATTACHMENT_READ: return std::string_view{"DEPTH_ATTACHMENT_READ"};
        case TaskImageAccess::STENCIL_ATTACHMENT_READ: return std::string_view{"STENCIL_ATTACHMENT_READ"};
        case TaskImageAccess::DEPTH_STENCIL_ATTACHMENT_READ: return std::string_view{"DEPTH_STENCIL_ATTACHMENT_READ"};
        case TaskImageAccess::RESOLVE_WRITE: return std::string_view{"RESOLVE_WRITE"};
        case TaskImageAccess::PRESENT: return std::string_view{"PRESENT"};
        default: DAXA_DBG_ASSERT_TRUE_M(false, "unreachable");
        }
        return "invalid";
    }

    TaskInterfaceUses::TaskInterfaceUses(void * a_backend)
        : backend{a_backend}
    {
    }

    auto TaskInterfaceUses::operator[](TaskBufferHandle const & handle) const -> TaskBufferUse<> const &
    {
        auto & impl = *static_cast<ImplTaskRuntimeInterface *>(this->backend);
        auto const local_handle = impl.task_list.id_to_local_id(handle);
        auto iter = std::find_if(impl.current_task->base_task->get_generic_uses().begin(),
                                 impl.current_task->base_task->get_generic_uses().end(),
                                 [&](GenericTaskResourceUse const & input)
                                 {
                                     if (input.type == TaskResourceUseType::BUFFER)
                                     {
                                         auto const & buf_use = TaskBufferUse<>::from(input);
                                         return buf_use.handle == local_handle;
                                     }
                                     return false;
                                 });
        DAXA_DBG_ASSERT_TRUE_M(iter != impl.current_task->base_task->get_generic_uses().end(), "Detected invalid task buffer handle! Only handles, that are used in the task are in the list of uses!");
        usize const buffer_use_index = static_cast<usize>(std::distance(impl.current_task->base_task->get_generic_uses().begin(), iter));
        return TaskBufferUse<>::from(impl.current_task->base_task->get_generic_uses()[buffer_use_index]);
    }

    auto TaskInterfaceUses::operator[](TaskImageHandle const & handle) const -> TaskImageUse<> const &
    {
        auto & impl = *static_cast<ImplTaskRuntimeInterface *>(this->backend);
        auto const local_handle = impl.task_list.id_to_local_id(handle);
        auto iter = std::find_if(impl.current_task->base_task->get_generic_uses().begin(),
                                 impl.current_task->base_task->get_generic_uses().end(),
                                 [&](GenericTaskResourceUse const & input)
                                 {
                                     if (input.type == TaskResourceUseType::IMAGE)
                                     {
                                         auto const & img_use = TaskImageUse<>::from(input);
                                         return img_use.handle == local_handle;
                                     }
                                     return false;
                                 });
        DAXA_DBG_ASSERT_TRUE_M(iter != impl.current_task->base_task->get_generic_uses().end(), "Detected invalid task image handle! Only handles, that are used in the task are in the list of uses!");
        usize const image_use_index = static_cast<usize>(std::distance(impl.current_task->base_task->get_generic_uses().begin(), iter));
        return TaskImageUse<>::from(impl.current_task->base_task->get_generic_uses()[image_use_index]);
    }

    auto TaskInterfaceUses::get_uniform_buffer_info() const -> SetConstantBufferInfo
    {
        auto & impl = *static_cast<ImplTaskRuntimeInterface *>(this->backend);
        DAXA_DBG_ASSERT_TRUE_M(impl.set_uniform_buffer_info.has_value(), "task must have been created with a constant buffer slot in order to use task list provided constant buffer memory for uses.");
        return impl.set_uniform_buffer_info.value();
    }

    TaskInterface::TaskInterface(void * a_backend)
        : uses{a_backend}, backend{a_backend}
    {
    }

    auto TaskInterface::get_device() const -> Device &
    {
        auto & impl = *static_cast<ImplTaskRuntimeInterface *>(this->backend);
        return impl.task_list.info.device;
    }

    auto TaskInterface::get_command_list() const -> CommandList
    {
        auto & impl = *static_cast<ImplTaskRuntimeInterface *>(this->backend);
        if (impl.reuse_last_command_list)
        {
            impl.reuse_last_command_list = false;
            return impl.command_lists.back();
        }
        else
        {
            impl.command_lists.push_back({get_device().create_command_list({.name = std::string("Task Command List ") + std::to_string(impl.command_lists.size())})});
            return impl.command_lists.back();
        }
    }

    auto TaskInterface::get_allocator() const -> TransferMemoryPool &
    {
        auto & impl = *static_cast<ImplTaskRuntimeInterface *>(this->backend);
        return impl.task_list.staging_memory;
    }

    TaskBuffer::TaskBuffer(TaskBufferInfo const & info)
        : ManagedPtr{new ImplPersistentTaskBuffer(info)}
    {
    }
    ImplPersistentTaskBuffer::ImplPersistentTaskBuffer(TaskBufferInfo const & info)
        : info{info},
          actual_buffers{info.initial_buffers.buffers.begin(), info.initial_buffers.buffers.end()},
          latest_access{info.initial_buffers.latest_access},
          unique_index{ImplPersistentTaskBuffer::exec_unique_next_index++}
    {
    }
    ImplPersistentTaskBuffer::~ImplPersistentTaskBuffer() = default;

    auto TaskBuffer::handle() const -> TaskBufferHandle
    {
        auto & impl = *this->as<ImplPersistentTaskBuffer>();
        return TaskBufferHandle{{.task_list_index = std::numeric_limits<u32>::max(), .index = impl.unique_index}};
    }

    TaskBuffer::operator TaskBufferHandle() const
    {
        return handle();
    }

    auto TaskBuffer::info() const -> TaskBufferInfo const &
    {
        auto & impl = *this->as<ImplPersistentTaskBuffer>();
        return impl.info;
    }

    auto TaskBuffer::get_state() const -> TrackedBuffers
    {
        auto const & impl = *this->as<ImplPersistentTaskBuffer const>();
        return TrackedBuffers{
            .buffers = {impl.actual_buffers.data(), impl.actual_buffers.size()},
            .latest_access = impl.latest_access,
        };
    }

    void TaskBuffer::set_buffers(TrackedBuffers const & buffers)
    {
        auto & impl = *this->as<ImplPersistentTaskBuffer>();
        impl.actual_buffers.clear();
        impl.actual_buffers.insert(impl.actual_buffers.end(), buffers.buffers.begin(), buffers.buffers.end());
        impl.latest_access = buffers.latest_access;
    }

    void TaskBuffer::swap_buffers(TaskBuffer & other)
    {
        auto & impl = *this->as<ImplPersistentTaskBuffer>();
        auto & impl_other = *other.as<ImplPersistentTaskBuffer>();
        std::swap(impl.actual_buffers, impl_other.actual_buffers);
        std::swap(impl.latest_access, impl_other.latest_access);
    }

    TaskImage::TaskImage(TaskImageInfo const & a_info)
        : ManagedPtr{new ImplPersistentTaskImage(a_info)}
    {
    }

    ImplPersistentTaskImage::ImplPersistentTaskImage(TaskImageInfo const & a_info)
        : info{a_info},
          actual_images{a_info.initial_images.images.begin(), a_info.initial_images.images.end()},
          latest_slice_states{a_info.initial_images.latest_slice_states.begin(), a_info.initial_images.latest_slice_states.end()},
          unique_index{ImplPersistentTaskImage::exec_unique_next_index++}
    {
    }

    ImplPersistentTaskImage::~ImplPersistentTaskImage()
    {
    }

    TaskImage::operator TaskImageHandle() const
    {
        return handle();
    }

    auto TaskImage::handle() const -> TaskImageHandle
    {
        auto & impl = *this->as<ImplPersistentTaskImage>();
        return TaskImageHandle{{.task_list_index = std::numeric_limits<u32>::max(), .index = impl.unique_index}};
    }

    auto TaskImage::info() const -> TaskImageInfo const &
    {
        auto & impl = *this->as<ImplPersistentTaskImage>();
        return impl.info;
    }

    auto TaskImage::get_state() const -> TrackedImages
    {
        auto const & impl = *this->as<ImplPersistentTaskImage const>();
        return TrackedImages{
            .images = {impl.actual_images.data(), impl.actual_images.size()},
            .latest_slice_states = {impl.latest_slice_states.data(), impl.latest_slice_states.size()},
        };
    }

    void TaskImage::set_images(TrackedImages const & images)
    {
        auto & impl = *this->as<ImplPersistentTaskImage>();
        impl.actual_images.clear();
        impl.actual_images.insert(impl.actual_images.end(), images.images.begin(), images.images.end());
        impl.latest_slice_states.clear();
        impl.latest_slice_states.insert(impl.latest_slice_states.end(), images.latest_slice_states.begin(), images.latest_slice_states.end());
    }

    void TaskImage::swap_images(TaskImage & other)
    {
        auto & impl = *this->as<ImplPersistentTaskImage>();
        auto & impl_other = *other.as<ImplPersistentTaskImage>();
        std::swap(impl.actual_images, impl_other.actual_images);
        std::swap(impl.latest_slice_states, impl_other.latest_slice_states);
    }

    TaskList::TaskList(TaskListInfo const & info)
        : ManagedPtr{new ImplTaskList(info)}
    {
        auto & impl = *reinterpret_cast<ImplTaskList *>(this->object);
        impl.permutations.resize(usize{1} << info.permutation_condition_count);
        for (auto & permutation : impl.permutations)
        {
            permutation.batch_submit_scopes.push_back({});
        }
        impl.update_active_permutations();
    }
    TaskList::~TaskList() = default;

    void TaskList::use_persistent_buffer(TaskBuffer const & buffer)
    {
        auto & impl = *reinterpret_cast<ImplTaskList *>(this->object);
        DAXA_DBG_ASSERT_TRUE_M(!impl.compiled, "completed task lists can not record new tasks");
        DAXA_DBG_ASSERT_TRUE_M(!impl.buffer_name_to_id.contains(buffer.info().name), "task buffer names msut be unique");
        TaskBufferHandle task_buffer_id{{.task_list_index = impl.unique_index, .index = static_cast<u32>(impl.global_buffer_infos.size())}};

        for (auto & permutation : impl.permutations)
        {
            permutation.buffer_infos.push_back(PerPermTaskBuffer{
                .valid = false,
            });
        }

        impl.global_buffer_infos.emplace_back(PermIndepTaskBufferInfo{
            .task_buffer_data = PermIndepTaskBufferInfo::Persistent{
                .buffer = ManagedPtr{buffer.object}}});
        impl.persistent_buffer_index_to_local_index[buffer.handle().index] = task_buffer_id.index;
        impl.buffer_name_to_id[buffer.info().name] = task_buffer_id;
    }

    void TaskList::use_persistent_image(TaskImage const & image)
    {
        auto & impl = *reinterpret_cast<ImplTaskList *>(this->object);
        DAXA_DBG_ASSERT_TRUE_M(!impl.compiled, "completed task lists can not record new tasks");
        DAXA_DBG_ASSERT_TRUE_M(!impl.image_name_to_id.contains(image.info().name), "task image names must be unique");
        TaskImageHandle task_image_id{{.task_list_index = impl.unique_index, .index = static_cast<u32>(impl.global_image_infos.size())}};

        for (auto & permutation : impl.permutations)
        {
            // For non-persistent resources task list will synch on the initial to first use every execution.
            permutation.image_infos.emplace_back(PerPermTaskImage{
                .valid = false,
                .swapchain_semaphore_waited_upon = false,
                .last_slice_states = {},
                .first_slice_states = {},
            });
            if (image.info().swapchain_image)
            {
                DAXA_DBG_ASSERT_TRUE_M(permutation.swapchain_image.is_empty(), "can only register one swapchain image per task list permutation");
                permutation.swapchain_image = task_image_id;
            }
        }

        impl.global_image_infos.emplace_back(PermIndepTaskImageInfo{
            .task_image_data = PermIndepTaskImageInfo::Persistent{
                .image = ManagedPtr{image.object},
            }});
        impl.persistent_image_index_to_local_index[image.handle().index] = task_image_id.index;
        impl.image_name_to_id[image.info().name] = task_image_id;
    }

    auto TaskList::create_transient_buffer(TaskTransientBufferInfo const & info) -> TaskBufferHandle
    {
        auto & impl = *reinterpret_cast<ImplTaskList *>(this->object);
        DAXA_DBG_ASSERT_TRUE_M(!impl.compiled, "completed task lists can not record new tasks");
        DAXA_DBG_ASSERT_TRUE_M(!impl.buffer_name_to_id.contains(info.name), "task buffer names msut be unique");
        TaskBufferHandle task_buffer_id{{.task_list_index = impl.unique_index, .index = static_cast<u32>(impl.global_buffer_infos.size())}};

        for (auto & permutation : impl.permutations)
        {
            permutation.buffer_infos.push_back(PerPermTaskBuffer{
                .valid = permutation.active,
            });
        }
        auto info_copy = info; // NOTE: (HACK) we must do this because msvc designated init bugs causing it to not generate copy constructors.
        impl.global_buffer_infos.emplace_back(PermIndepTaskBufferInfo{
            .task_buffer_data = PermIndepTaskBufferInfo::Transient{.info = info_copy}});

        impl.buffer_name_to_id[info.name] = task_buffer_id;
        return task_buffer_id;
    }

    auto TaskList::create_transient_image(TaskTransientImageInfo const & info) -> TaskImageHandle
    {
        auto & impl = *reinterpret_cast<ImplTaskList *>(this->object);
        DAXA_DBG_ASSERT_TRUE_M(!impl.compiled, "completed task lists can not record new tasks");
        DAXA_DBG_ASSERT_TRUE_M(!impl.image_name_to_id.contains(info.name), "task image names must be unique");
        TaskImageHandle task_image_id{{.task_list_index = impl.unique_index, .index = static_cast<u32>(impl.global_image_infos.size())}};

        for (auto & permutation : impl.permutations)
        {
            permutation.image_infos.emplace_back(PerPermTaskImage{
                .valid = permutation.active,
                .swapchain_semaphore_waited_upon = false,
            });
        }

        auto info_copy = info; // NOTE: (HACK) we must do this because msvc designated init bugs causing it to not generate copy constructors.
        impl.global_image_infos.emplace_back(PermIndepTaskImageInfo{
            .task_image_data = PermIndepTaskImageInfo::Transient{
                .info = info_copy,
            }});
        impl.image_name_to_id[info.name] = task_image_id;
        return task_image_id;
    }

    auto ImplTaskList::get_actual_buffers(TaskBufferHandle id, TaskListPermutation const & perm) const -> std::span<BufferId const>
    {
        auto const & global_buffer = global_buffer_infos.at(id.index);
        if (global_buffer.is_persistent())
        {
            return {global_buffer.get_persistent().actual_buffers.data(),
                    global_buffer.get_persistent().actual_buffers.size()};
        }
        else
        {
            auto const & perm_buffer = perm.buffer_infos.at(id.index);
            DAXA_DBG_ASSERT_TRUE_M(perm_buffer.valid, "Can not get actual buffer - buffer is not valid in this permutation");
            return {&perm_buffer.actual_buffer, 1};
        }
    }

    auto ImplTaskList::get_actual_images(TaskImageHandle id, TaskListPermutation const & perm) const -> std::span<ImageId const>
    {
        auto const & global_image = global_image_infos.at(id.index);
        if (global_image.is_persistent())
        {
            return {global_image.get_persistent().actual_images.data(),
                    global_image.get_persistent().actual_images.size()};
        }
        else
        {
            auto const & perm_image = perm.image_infos.at(id.index);
            DAXA_DBG_ASSERT_TRUE_M(perm_image.valid, "Can not get actual image - image is not valid in this permutation");
            return {&perm_image.actual_image, 1};
        }
    }

    auto ImplTaskList::id_to_local_id(TaskBufferHandle id) const -> TaskBufferHandle
    {
        DAXA_DBG_ASSERT_TRUE_M(!id.is_empty(), "detected empty task buffer id. Please make sure to only use intialized task buffer ids.");
        if (id.is_persistent())
        {
            DAXA_DBG_ASSERT_TRUE_M(
                persistent_buffer_index_to_local_index.contains(id.index),
                fmt::format("detected invalid access of persistent task buffer id ({}) in task list \"{}\"; "
                            "please make sure to declare persistent resource use to each task list that uses this buffer with the function use_persistent_buffer!",
                            id.index, info.name));
            return TaskBufferHandle{{.task_list_index = this->unique_index, .index = persistent_buffer_index_to_local_index.at(id.index)}};
        }
        else
        {
            DAXA_DBG_ASSERT_TRUE_M(
                id.task_list_index == this->unique_index,
                fmt::format("detected invalid access of transient task buffer id ({}) in task list \"{}\"; "
                            "please make sure that you only use transient buffers within the list they are created in!",
                            id.index, info.name));
            return TaskBufferHandle{{.task_list_index = this->unique_index, .index = id.index}};
        }
    }

    auto ImplTaskList::id_to_local_id(TaskImageHandle id) const -> TaskImageHandle
    {
        DAXA_DBG_ASSERT_TRUE_M(!id.is_empty(), "detected empty task image id. Please make sure to only use initialized task image ids.");
        if (id.is_persistent())
        {
            DAXA_DBG_ASSERT_TRUE_MS(
                persistent_image_index_to_local_index.contains(id.index),
                << "detected invalid access of persistent task image id "
                << id.index
                << " in tasklist \""
                << info.name
                << "\". Please make sure to declare persistent resource use to each task list that uses this image with the function use_persistent_image!");
            return TaskImageHandle{{.task_list_index = this->unique_index, .index = persistent_image_index_to_local_index.at(id.index)}, id.slice};
        }
        else
        {
            DAXA_DBG_ASSERT_TRUE_MS(
                id.task_list_index == this->unique_index,
                << "detected invalid access of transient task image id "
                << (id.index)
                << " in tasklist \""
                << info.name
                << "\". Please make sure that you only use transient image within the list they are created in!");
            return TaskImageHandle{{.task_list_index = this->unique_index, .index = id.index}, id.slice};
        }
    }

    void ImplTaskList::update_active_permutations()
    {
        record_active_permutations.clear();

        for (u32 permutation_i = 0; permutation_i < permutations.size(); ++permutation_i)
        {
            bool const active = (record_active_conditional_scopes & permutation_i) == (record_active_conditional_scopes & record_conditional_states);
            permutations[permutation_i].active = active;
            if (active)
            {
                record_active_permutations.push_back(&permutations[permutation_i]);
            }
        }
    }

    void validate_runtime_image_slice(ImplTaskList & impl, TaskListPermutation const & perm, u32 use_index, u32 task_image_index, ImageMipArraySlice const & access_slice)
    {
        auto const actual_images = impl.get_actual_images(TaskImageHandle{{.task_list_index = impl.unique_index, .index = task_image_index}}, perm);
        std::string_view task_name = impl.global_image_infos[task_image_index].get_name();
        for (u32 index = 0; index < actual_images.size(); ++index)
        {
            ImageMipArraySlice const full_slice = impl.info.device.info_image_view(actual_images[index].default_view()).slice;
            std::string const & name = impl.info.device.info_image(actual_images[index]).name;
            bool const use_within_runtime_image_counts =
                (access_slice.base_mip_level + access_slice.level_count <= full_slice.base_mip_level + full_slice.level_count) &&
                (access_slice.base_array_layer + access_slice.layer_count <= full_slice.base_array_layer + full_slice.layer_count);
            [[maybe_unused]] std::string const error_message =
                fmt::format("task image argument (arg index: {}, task image: \"{}\", slice: {}) exceeds runtime image (index: {}, name: \"{}\") dimensions ({})!",
                            use_index, task_name, to_string(access_slice), index, name, to_string(full_slice));
            DAXA_DBG_ASSERT_TRUE_M(use_within_runtime_image_counts, error_message);
        }
    }

    void validate_image_uses(ImplTaskList & impl, TaskListPermutation const & perm, u32 use_index, u32 task_image_index, TaskImageAccess task_access, std::string_view task_name)
    {
        ImageUsageFlags use_flags = access_to_usage(task_access);
        auto const actual_images = impl.get_actual_images(TaskImageHandle{{.task_list_index = impl.unique_index, .index = task_image_index}}, perm);
        std::string_view task_image_name = impl.global_image_infos[task_image_index].get_name();
        for (u32 index = 0; index < actual_images.size(); ++index)
        {
            ImageId image = actual_images[index];
            bool const access_valid = (impl.info.device.info_image(image).usage & use_flags) != ImageUsageFlagBits::NONE;
            DAXA_DBG_ASSERT_TRUE_M(access_valid, fmt::format("detected invalid runtime image \"{}\" of task image \"{}\", in use {} of task \"{}\". "
                                                             "The given runtime image does NOT have the image use flag {} set, but the task use requires this use for all runtime images!",
                                                             impl.info.device.info_image(image).name, task_image_name, use_index, task_name, daxa::to_string(use_flags)));
        }
    }

    void ImplTaskList::update_image_view_cache(ImplTask & task, TaskListPermutation const & permutation)
    {
        for_each(
            task.base_task->get_generic_uses(),
            [](u32, TaskBufferUse<> &) {},
            [&](u32 task_image_use_index, TaskImageUse<> & image_use)
            {
                auto const slice = image_use.handle.slice;
                // The image id here is alreadt the task list local id.
                // The persistent ids are converted to local ids in the add_task function.
                auto const tid = image_use.handle;

                auto const actual_images = get_actual_images(tid, permutation);
                auto & view_cache = task.image_view_cache[task_image_use_index];

                bool cache_valid = actual_images.size() == view_cache.size();
                if (cache_valid)
                {
                    for (u32 index = 0; index < actual_images.size(); ++index)
                    {
                        cache_valid = cache_valid &&
                                      info.device.is_id_valid(view_cache[index]) &&
                                      info.device.info_image_view(view_cache[index]).image == actual_images[index];
                    }
                }
                if (!cache_valid)
                {
                    validate_runtime_image_slice(*this, permutation, task_image_use_index, tid.index, slice);
                    validate_image_uses(*this, permutation, task_image_use_index, tid.index, image_use.access(), task.base_task->get_name());
                    for (auto & view : view_cache)
                    {
                        if (info.device.is_id_valid(view))
                        {
                            ImageViewId const parent_image_default_view = info.device.info_image_view(view).image.default_view();
                            // Can not destroy the default view of an image!!!
                            if (parent_image_default_view != view)
                            {
                                info.device.destroy_image_view(view);
                            }
                        }
                    }
                    view_cache.clear();
                    for (u32 index = 0; index < actual_images.size(); ++index)
                    {
                        ImageId parent = actual_images[index];
                        ImageViewInfo view_info = info.device.info_image_view(parent.default_view());
                        ImageViewType use_view_type = (image_use.m_view_type != ImageViewType::MAX_ENUM) ? image_use.m_view_type : view_info.type;

                        // When the use image view parameters match the default view,
                        // then use the default view id and avoid creating a new id here.
                        bool const is_use_default_slice = view_info.slice == slice;
                        bool const is_use_default_view_type = use_view_type == view_info.type;
                        if (is_use_default_slice && is_use_default_view_type)
                        {
                            view_cache.push_back(parent.default_view());
                        }
                        else
                        {
                            view_info.type = use_view_type;
                            view_info.slice = slice;
                            view_cache.push_back(info.device.create_image_view(view_info));
                        }
                    }
                }
            });
    }

    void validate_runtime_resources(ImplTaskList const & impl, TaskListPermutation const & permutation)
    {
#if DAXA_VALIDATION
        constexpr std::string_view PERSISTENT_RESOURCE_MESSAGE = {
            "when executing a task list, all used persistent resources must be backed by at least one and exclusively "
            "valid runtime resources"};
        for (u32 local_buffer_i = 0; local_buffer_i < impl.global_buffer_infos.size(); ++local_buffer_i)
        {
            if (!permutation.buffer_infos[local_buffer_i].valid)
            {
                continue;
            }
            if (!impl.global_buffer_infos.at(local_buffer_i).is_persistent())
            {
                continue;
            }
            auto const & runtime_buffers = impl.global_buffer_infos.at(local_buffer_i).get_persistent().actual_buffers;
            DAXA_DBG_ASSERT_TRUE_M(
                runtime_buffers.size() > 0,
                fmt::format(
                    "detected persistent task buffer \"{}\" used in task list \"{}\" with 0 runtime buffers; {}",
                    impl.global_buffer_infos[local_buffer_i].get_name(),
                    impl.info.name,
                    PERSISTENT_RESOURCE_MESSAGE));
            for (usize buffer_index = 0; buffer_index < runtime_buffers.size(); ++buffer_index)
            {
                DAXA_DBG_ASSERT_TRUE_M(
                    impl.info.device.is_id_valid(runtime_buffers[buffer_index]),
                    fmt::format(
                        "detected persistent task buffer \"{}\" used in task list \"{}\" with invalid buffer id (runtime buffer index: {}); {}",
                        impl.global_buffer_infos[local_buffer_i].get_name(),
                        impl.info.name,
                        buffer_index,
                        PERSISTENT_RESOURCE_MESSAGE));
            }
        }
        for (u32 local_image_i = 0; local_image_i < impl.global_image_infos.size(); ++local_image_i)
        {
            if (!permutation.image_infos[local_image_i].valid)
            {
                continue;
            }
            if (!impl.global_image_infos.at(local_image_i).is_persistent())
            {
                continue;
            }
            auto const & runtime_images = impl.global_image_infos.at(local_image_i).get_persistent().actual_images;
            DAXA_DBG_ASSERT_TRUE_M(
                runtime_images.size() > 0,
                fmt::format(
                    "detected persistent task image \"{}\" used in task list \"{}\" with 0 runtime images; {}",
                    impl.global_image_infos[local_image_i].get_name(),
                    impl.info.name,
                    PERSISTENT_RESOURCE_MESSAGE));
            for (usize image_index = 0; image_index < runtime_images.size(); ++image_index)
            {
                DAXA_DBG_ASSERT_TRUE_M(
                    impl.info.device.is_id_valid(runtime_images[image_index]),
                    fmt::format(
                        "detected persistent task image \"{}\" used in task list \"{}\" with invalid image id (runtime image index: {}); {}",
                        impl.global_image_infos[local_image_i].get_name(),
                        impl.info.name,
                        image_index,
                        PERSISTENT_RESOURCE_MESSAGE));
            }
        }
#endif // #if DAXA_VALIDATION
    }

    void ImplTaskList::execute_task(ImplTaskRuntimeInterface & impl_runtime, TaskListPermutation & permutation, TaskBatchId in_batch_task_index, TaskId task_id)
    {
        // We always allow to reuse the last command list ONCE within the task callback.
        // When the get command list function is called in a task this is set to false.
        impl_runtime.reuse_last_command_list = true;
        ImplTask & task = tasks[task_id];
        update_image_view_cache(task, permutation);
        for_each(
            task.base_task->get_generic_uses(),
            [&](u32, TaskBufferUse<> & arg)
            {
                arg.buffers = this->get_actual_buffers(arg.handle, permutation);
            },
            [&](u32 input_index, TaskImageUse<> & arg)
            {
                arg.images = this->get_actual_images(arg.handle, permutation);
                arg.views = std::span{task.image_view_cache[input_index].data(), task.image_view_cache[input_index].size()};
            });
        bool const upload_args_to_constant_buffer = task.base_task->get_uses_constant_buffer_slot() != -1;
        if (upload_args_to_constant_buffer)
        {
            u32 const alignment = static_cast<u32>(info.device.properties().limits.min_uniform_buffer_offset_alignment);
            auto constant_buffer_alloc = staging_memory.allocate(task.constant_buffer_size, alignment).value();
            u8 * host_constant_buffer_ptr = reinterpret_cast<u8 *>(constant_buffer_alloc.host_address);
            auto d = task.base_task->get_generic_uses();
            for_each(
                task.base_task->get_generic_uses(),
                [&](u32 arg_i, TaskBufferUse<> & arg)
                {
                    auto adr = (host_constant_buffer_ptr + task.use_offsets[arg_i]);
                    *reinterpret_cast<types::BufferDeviceAddress *>(adr) = info.device.get_device_address(arg.buffers[0]);
                },
                [&](u32 arg_i, TaskImageUse<> & arg)
                {
                    auto adr = (host_constant_buffer_ptr + task.use_offsets[arg_i]);
                    auto ptr = reinterpret_cast<types::ImageViewId *>(adr);
                    *ptr = arg.views[0];
                    u32 tester = *reinterpret_cast<u32 *>(ptr);
                });
            impl_runtime.device_address = constant_buffer_alloc.device_address;
            impl_runtime.set_uniform_buffer_info = SetConstantBufferInfo{
                .slot = static_cast<u32>(task.base_task->get_uses_constant_buffer_slot()),
                .buffer = staging_memory.get_buffer(),
                .size = constant_buffer_alloc.size,
                .offset = constant_buffer_alloc.buffer_offset,
            };
        }
        impl_runtime.current_task = &task;
        impl_runtime.command_lists.back().begin_label({
            .label_name = std::string("task ") + std::to_string(in_batch_task_index) + std::string(" \"") + task.base_task->get_name() + std::string("\""),
            .label_color = info.task_label_color,
        });
        task.base_task->callback(TaskInterface{&impl_runtime});
        impl_runtime.command_lists.back().end_label();
    }

    void TaskList::conditional(TaskListConditionalInfo const & conditional_info)
    {
        auto & impl = *reinterpret_cast<ImplTaskList *>(this->object);
        DAXA_DBG_ASSERT_TRUE_M(!impl.compiled, "completed task lists can not record new tasks");
        [[maybe_unused]] bool const already_active = ((impl.record_active_conditional_scopes >> conditional_info.condition_index) & 1u) != 0;
        DAXA_DBG_ASSERT_TRUE_M(!already_active, "can not nest scopes of the same condition in itself.");
        DAXA_DBG_ASSERT_TRUE_M(conditional_info.condition_index < impl.info.permutation_condition_count,
                               fmt::format("detected invalid conditional index {}; conditional indices must all be smaller then the conditional count given in construction", conditional_info.condition_index));
        // Set conditional scope to active.
        impl.record_active_conditional_scopes |= 1u << conditional_info.condition_index;
        impl.update_active_permutations();
        // Set the conditional state to active.
        impl.record_conditional_states |= 1u << conditional_info.condition_index;
        impl.update_active_permutations();
        if (conditional_info.when_true)
        {
            conditional_info.when_true();
        }
        // Set the conditional state to false.
        impl.record_conditional_states &= ~(1u << conditional_info.condition_index);
        impl.update_active_permutations();
        if (conditional_info.when_false)
        {
            conditional_info.when_false();
        }
        // Set conditional scope to inactive.
        impl.record_active_conditional_scopes &= ~(1u << conditional_info.condition_index);
        impl.update_active_permutations();
    }

    void ImplTaskList::check_for_overlapping_use(BaseTask & task)
    {
        for_each(
            task.get_generic_uses(),
            [&](u32 index_a, TaskBufferUse<> const & a)
            {
                for_each(
                    task.get_generic_uses(),
                    [&](u32 index_b, TaskBufferUse<> const & b)
                    {
                        if (index_a == index_b)
                        {
                            return;
                        }
                        [[maybe_unused]] bool const overlapping = a.handle == b.handle;
                        DAXA_DBG_ASSERT_TRUE_M(
                            !overlapping,
                            fmt::format(
                                "detected overlapping uses (input index {} and {}) of buffer \"{}\" in task \"{}\"; all buffer task inputs must be disjoint!",
                                index_a, index_b,
                                global_buffer_infos[a.handle.index].get_name(),
                                task.get_name()));
                    },
                    [&](u32, TaskImageUse<> const &) {});
            },
            [&](u32 index_a, TaskImageUse<> const & a)
            {
                for_each(
                    task.get_generic_uses(),
                    [&](u32, TaskBufferUse<> const &) {},
                    [&](u32 index_b, TaskImageUse<> const & b)
                    {
                        if (index_a == index_b)
                        {
                            return;
                        }
                        [[maybe_unused]] auto const intersect = a.handle == b.handle && a.handle.slice.intersects(b.handle.slice);
                        [[maybe_unused]] auto const intersection = a.handle.slice.intersect(b.handle.slice);
                        DAXA_DBG_ASSERT_TRUE_M(
                            !intersect,
                            fmt::format(
                                "detected slice overlap between task image uses \n(use index: {}, slice: ({})) "
                                "and \n(use index: {}, slice: ({})), \n accessing task image \"{}\" witin task \"{}\", intersecting region of slices: ({}); all task image use slices must be disjoint within each task",
                                index_a, to_string(a.handle.slice),
                                index_b, to_string(b.handle.slice),
                                this->global_image_infos.at(b.handle.index).get_name(),
                                task.get_name(),
                                to_string(intersection)));
                    });
            });
    }

    auto shedule_task(
        ImplTaskList & impl,
        TaskListPermutation & perm,
        TaskBatchSubmitScope & current_submit_scope,
        usize const current_submit_scope_index,
        BaseTask & task)
        -> usize
    {
        usize first_possible_batch_index = 0;
        if (!impl.info.reorder_tasks)
        {
            first_possible_batch_index = std::max(current_submit_scope.task_batches.size(), 1ull) - 1ull;
        }

        for_each(
            task.get_generic_uses(),
            [&](u32, TaskBufferUse<> const & buffer_use)
            {
                PerPermTaskBuffer const & task_buffer = perm.buffer_infos[buffer_use.handle.index];
                // If the latest access is in a previous submit scope, the earliest batch we can insert into is
                // the current scopes first batch.
                if (task_buffer.latest_access_submit_scope_index < current_submit_scope_index)
                {
                    return;
                }

                Access const current_buffer_access = task_buffer_access_to_access(buffer_use.access());
                // Every other access (NONE, READ_WRITE, WRITE) are interpreted as writes in this context.
                bool const is_last_access_read = task_buffer.latest_access.type == AccessTypeFlagBits::READ;
                bool const is_last_access_none = task_buffer.latest_access.type == AccessTypeFlagBits::NONE;
                bool const is_current_access_read = current_buffer_access.type == AccessTypeFlagBits::READ;

                // TODO(msakmarry): improve sheduling here to reorder reads in front of each other, respecting the last to read barrier if present!
                // When a buffer has been read in a previous use AND the current task also reads the buffer,
                // we must insert the task at or after the last use batch.
                usize current_buffer_first_possible_batch_index = task_buffer.latest_access_batch_index;
                // So when not both, the last access and the current access, are reads, we need to insert AFTER the latest access.
                if (!(is_last_access_read && is_current_access_read) && !is_last_access_none)
                {
                    current_buffer_first_possible_batch_index += 1;
                }
                first_possible_batch_index = std::max(first_possible_batch_index, current_buffer_first_possible_batch_index);
            },
            [&](u32, TaskImageUse<> const & image_use)
            {
                PerPermTaskImage const & task_image = perm.image_infos[image_use.handle.index];
                PermIndepTaskImageInfo const & glob_task_image = impl.global_image_infos[image_use.handle.index];
                DAXA_DBG_ASSERT_TRUE_M(!task_image.swapchain_semaphore_waited_upon, "swapchain image is already presented!");
                if (glob_task_image.is_persistent() && glob_task_image.get_persistent().info.swapchain_image)
                {
                    if (perm.swapchain_image_first_use_submit_scope_index == std::numeric_limits<u64>::max())
                    {
                        perm.swapchain_image_first_use_submit_scope_index = current_submit_scope_index;
                        perm.swapchain_image_last_use_submit_scope_index = current_submit_scope_index;
                    }
                    else
                    {
                        perm.swapchain_image_first_use_submit_scope_index = std::min(current_submit_scope_index, perm.swapchain_image_first_use_submit_scope_index);
                        perm.swapchain_image_last_use_submit_scope_index = std::max(current_submit_scope_index, perm.swapchain_image_last_use_submit_scope_index);
                    }
                }

                auto [this_task_image_layout, this_task_image_access] = task_image_access_to_layout_access(image_use.access());
                // As image subresources can be in different layouts and also different synchronization scopes,
                // we need to track these image ranges individually.
                for (ExtendedImageSliceState const & tracked_slice : task_image.last_slice_states)
                {
                    // If the latest access is in a previous submit scope, the earliest batch we can insert into is
                    // the current scopes first batch.
                    // When the slices dont intersect, we dont need to do any sync or execution ordering between them.
                    if (
                        tracked_slice.latest_access_submit_scope_index < current_submit_scope_index ||
                        !tracked_slice.state.slice.intersects(image_use.handle.slice))
                    {
                        continue;
                    }
                    // Now that we found out that the new use and an old use intersect,
                    // we need to insert the task in the same or a later batch.
                    bool const is_last_access_read = tracked_slice.state.latest_access.type == AccessTypeFlagBits::READ;
                    bool const is_current_access_read = this_task_image_access.type == AccessTypeFlagBits::READ;
                    // When the image layouts differ, we must do a layout transition between reads.
                    // This forces us to place the task into a batch AFTER the tracked uses last batch.
                    bool const is_layout_identical = this_task_image_layout == tracked_slice.state.latest_layout;
                    usize current_image_first_possible_batch_index = tracked_slice.latest_access_batch_index;
                    // If either the image layouts differ, or not both accesses are reads, we must place the task in a later batch.
                    if (!(is_last_access_read && is_current_access_read && is_layout_identical))
                    {
                        current_image_first_possible_batch_index += 1;
                    }
                    first_possible_batch_index = std::max(first_possible_batch_index, current_image_first_possible_batch_index);
                }
            });
        // Make sure we have enough batches.
        if (first_possible_batch_index >= current_submit_scope.task_batches.size())
        {
            current_submit_scope.task_batches.resize(first_possible_batch_index + 1);
        }
        return first_possible_batch_index;
    }

    void translate_persistent_ids(ImplTaskList const & impl, BaseTask & task)
    {
        for_each(
            task.get_generic_uses(),
            [&](u32 index, TaskBufferUse<> & arg)
            {
                DAXA_DBG_ASSERT_TRUE_M(
                    !arg.handle.is_empty(),
                    fmt::format("detected empty task buffer handle in use (index: {}, access: {}) in task \"{}\"\n", index, to_string(arg.access()), task.get_name()));
                arg.handle = impl.id_to_local_id(arg.handle);
            },
            [&](u32 index, TaskImageUse<> & arg)
            {
                DAXA_DBG_ASSERT_TRUE_M(
                    !arg.handle.is_empty(),
                    fmt::format("detected empty task image handle in use (index: {}, access: {}) in task \"{}\"\n", index, to_string(arg.access()), task.get_name()));
                arg.handle = impl.id_to_local_id(arg.handle);
            });
    }

    void TaskList::add_task(std::unique_ptr<BaseTask> && base_task)
    {
        auto & impl = *reinterpret_cast<ImplTaskList *>(this->object);
        DAXA_DBG_ASSERT_TRUE_M(!impl.compiled, "completed task lists can not record new tasks");
        translate_persistent_ids(impl, *base_task);
        // Overlapping resource uses can be valid in the case of reads in the same layout for example.
        // But in order to make the task list implementation simpler,
        // daxa does not allow for overlapping use of a resource within a task, even when it is a read in the same layout.
        impl.check_for_overlapping_use(*base_task);

        TaskId const task_id = impl.tasks.size();

        for (auto * permutation : impl.record_active_permutations)
        {
            permutation->add_task(task_id, impl, *base_task);
        }

        std::vector<u32> constant_buffer_use_offsets = {};
        u32 constant_buffer_size = {};
        if (base_task->get_uses_constant_buffer_slot() != -1)
        {
            auto offsets_size = get_task_arg_shader_offsets_size(base_task->get_generic_uses());
            constant_buffer_use_offsets = std::move(offsets_size.first);
            constant_buffer_size = offsets_size.second;
        }

        std::vector<std::vector<ImageViewId>> view_cache = {};
        view_cache.resize(base_task->get_generic_uses().size(), {});
        impl.tasks.emplace_back(ImplTask{
            .base_task = std::move(base_task),
            .constant_buffer_size = constant_buffer_size,
            .use_offsets = std::move(constant_buffer_use_offsets),
            .image_view_cache = std::move(view_cache),
        });
    }

    thread_local std::vector<ImageMipArraySlice> tl_new_access_slices = {};
    void update_image_initial_access_slices(
        PerPermTaskImage & task_image,
        ExtendedImageSliceState new_access_slice)
    {
        // We need to test if a new use adds and or subtracts from initial uses.
        // To do that, we need to test if the new access slice is accessing a subresource BEFORE all other already stored initial access slices.
        // We compare all new use slices with already tracked first uses.
        // We intersect the earlier and later happening slices with the new slice and store the intersection rest or the respective later executing slice access.
        // This list will contain the remainder of the new access ranges after intersections.
        tl_new_access_slices.push_back(new_access_slice.state.slice);
        // Name shortening:
        auto & initial_accesses = task_image.first_slice_states;
        // Note(pahrens):
        // NEVER access new_access_slice.slice in this function past this point.
        // ALWAYS access the new access ImageMipArraySlice's from an iterator or via vector indexing.
        for (usize new_access_slice_i = 0; new_access_slice_i < tl_new_access_slices.size();)
        {
            bool broke_inner_loop = false;
            for (usize initial_access_i = 0; initial_access_i < task_image.first_slice_states.size();)
            {
                bool const slices_disjoint = !tl_new_access_slices[new_access_slice_i].intersects(initial_accesses[initial_access_i].state.slice);
                bool const same_batch =
                    new_access_slice.latest_access_submit_scope_index == initial_accesses[initial_access_i].latest_access_submit_scope_index &&
                    new_access_slice.latest_access_batch_index == initial_accesses[initial_access_i].latest_access_batch_index;
                // We check if the sets are disjoint.
                // If they are we do not need to do anything and advance to the next test.
                // When two accesses are in the same batch and scope, they can not overlap.
                // This is simply forbidden by task list rules!
                if (same_batch || slices_disjoint)
                {
                    ++initial_access_i;
                    continue;
                }
                // Now that we have this edge case out the way, we now need to test which tracked slice is executed earlier.
                bool const new_use_executes_earlier =
                    new_access_slice.latest_access_submit_scope_index < initial_accesses[initial_access_i].latest_access_submit_scope_index ||
                    (new_access_slice.latest_access_submit_scope_index == initial_accesses[initial_access_i].latest_access_submit_scope_index &&
                     new_access_slice.latest_access_batch_index < initial_accesses[initial_access_i].latest_access_batch_index);
                // When the new use is executing earlier, we subtract from the current initial access slice.
                // We then replace the current initial access slice with the resulting rest.
                if (new_use_executes_earlier)
                {
                    // When we intersect, we remove the old initial access slice and replace it with the rest of the subtraction.
                    // We need a copy of this, as we will erase this value from the vector first.
                    auto const initial_access_slice = initial_accesses[initial_access_i];
                    // Erase value from vector.
                    task_image.first_slice_states.erase(initial_accesses.begin() + isize(initial_access_i));
                    // Subtract ranges.
                    auto const [slice_rest, slice_rest_count] = initial_access_slice.state.slice.subtract(tl_new_access_slices[new_access_slice_i]);
                    // Now construct new sub-ranges from the rest of the subtraction.
                    // We advance the iterator each time.
                    for (usize rest_i = 0; rest_i < slice_rest_count; ++rest_i)
                    {
                        auto rest_tracked_slice = initial_access_slice;
                        rest_tracked_slice.state.slice = slice_rest[rest_i];
                        // We insert into the beginning, so we dont recheck these with the current new use slice.
                        // They are the result of a subtraction therefore disjoint.
                        task_image.first_slice_states.insert(initial_accesses.begin(), rest_tracked_slice);
                    }
                    // We erased, so we implicitly advanced by an element, as erase moves all elements one to the left past the iterator.
                    // But as we inserted into the front, we need to move the index accordingly to "stay in place".
                    initial_access_i += slice_rest_count;
                }
                // When the new use is executing AFTER the current inital access slice, we subtract the current initial access slice from the new slice.
                // We then replace the current new access slice with the resulting rest.
                else
                {
                    // We subtract the initial use from the new use and append the rest.
                    auto const [slice_rest, slice_rest_count] = tl_new_access_slices[new_access_slice_i].subtract(initial_accesses[initial_access_i].state.slice);
                    // We insert the rest of the subtraction into the new use list.
                    tl_new_access_slices.insert(tl_new_access_slices.end(), slice_rest.begin(), slice_rest.begin() + isize(slice_rest_count));
                    // We remove the current new use slice, as it intersects with an initial use slice and is later in the list.
                    tl_new_access_slices.erase(tl_new_access_slices.begin() + isize(new_access_slice_i));
                    // If we advance the new use index, we restart the inner loop over the initial accesses.
                    broke_inner_loop = true;
                    break;
                }
            }
            // When we broke out the inner loop we want to "restart" iteration of the outer loop at the current index.
            if (!broke_inner_loop)
            {
                ++new_access_slice_i;
            }
        }
        // Add the newly found initial access slices to the list of initial access slices.
        for (auto const & new_slice : tl_new_access_slices)
        {
            auto new_tracked_slice = new_access_slice;
            new_tracked_slice.state.slice = new_slice;
            initial_accesses.push_back(new_tracked_slice);
        }
        tl_new_access_slices.clear();
    }

    using ShaderUseIdOffsetTable = std::vector<std::variant<std::pair<TaskImageHandle, usize>, std::pair<TaskBufferHandle, usize>, std::monostate>>;

    void update_buffer_first_access(PerPermTaskBuffer & buffer, usize new_access_batch, usize new_access_submit, Access new_access)
    {
        if (buffer.first_access.type == AccessTypeFlagBits::NONE)
        {
            buffer.first_access = new_access;
            buffer.first_access_batch_index = new_access_batch;
            buffer.first_access_submit_scope_index = new_access_submit;
        }
        else if (buffer.first_access.type == AccessTypeFlagBits::READ && new_access.type == AccessTypeFlagBits::READ)
        {
            buffer.first_access = buffer.first_access | new_access;
            bool const new_is_earlier = new_access_submit < buffer.first_access_submit_scope_index ||
                                        (new_access_submit == buffer.first_access_submit_scope_index && new_access_batch < buffer.first_access_batch_index);
            if (new_is_earlier)
            {
                buffer.first_access_batch_index = new_access_batch;
                buffer.first_access_submit_scope_index = new_access_submit;
            }
        }
    }

    // I hate this function.
    thread_local std::vector<ExtendedImageSliceState> tl_tracked_slice_rests = {};
    thread_local std::vector<ImageMipArraySlice> tl_new_use_slices = {};
    void TaskListPermutation::add_task(
        TaskId task_id,
        ImplTaskList & task_list_impl,
        BaseTask & task)
    {
        // Set persistent task resources to be valid for the permutation.
        for_each(
            task.get_generic_uses(),
            [&](u32, TaskBufferUse<> & arg)
            {
                if (task_list_impl.global_buffer_infos[arg.handle.index].is_persistent())
                {
                    buffer_infos[arg.handle.index].valid = true;
                }
            },
            [&](u32, TaskImageUse<> & arg)
            {
                if (task_list_impl.global_image_infos[arg.handle.index].is_persistent())
                {
                    image_infos[arg.handle.index].valid = true;
                }
            });

        usize const current_submit_scope_index = this->batch_submit_scopes.size() - 1;
        TaskBatchSubmitScope & current_submit_scope = this->batch_submit_scopes[current_submit_scope_index];

        // All tasks are reordered while recording.
        // Tasks are grouped into "task batches" which are just a group of tasks,
        // that can execute together while overlapping without synchronization between them.
        // Task batches are further grouped into submit scopes.
        // A submit scopes contains a group of batches between two submits.
        // At first, we find the batch we need to insert the new task into.
        // To optimize for optimal overlap and minimal pipeline barriers, we try to insert the task as early as possible.
        const usize batch_index = shedule_task(
            task_list_impl,
            *this,
            current_submit_scope,
            current_submit_scope_index,
            task);
        TaskBatch & batch = current_submit_scope.task_batches[batch_index];
        // Add the task to the batch.
        batch.tasks.push_back(task_id);

        // Now that we know what batch we need to insert the task into, we need to add synchronization between batches.
        // As stated earlier batches are groups of tasks which can execute together without sync between them.
        // To simplify and optimize the sync placement daxa only synchronizes between batches.
        // This effectively means that all the resource uses, and their memory and execution dependencies in a batch
        // are combined into a single unit which is synchronized against other batches.
        for_each(
            task.get_generic_uses(),
            [&](u32, TaskBufferUse<> const & buffer_use)
            {
                PerPermTaskBuffer & task_buffer = this->buffer_infos[buffer_use.handle.index];
                Access const current_buffer_access = task_buffer_access_to_access(buffer_use.access());
                update_buffer_first_access(task_buffer, batch_index, current_submit_scope_index, current_buffer_access);
                // For transient buffers, we need to record first and last use so that we can later name their allocations.
                // TODO(msakmary, pahrens) We should think about how to combine this with update_buffer_first_access below since
                // they both overlap in what they are doing
                if (!task_list_impl.global_buffer_infos.at(buffer_use.handle.index).is_persistent())
                {
                    auto & buffer_first_use = task_buffer.lifetime.first_use;
                    auto & buffer_last_use = task_buffer.lifetime.last_use;
                    if (current_submit_scope_index < buffer_first_use.submit_scope_index)
                    {
                        buffer_first_use.submit_scope_index = current_submit_scope_index;
                        buffer_first_use.task_batch_index = batch_index;
                    }
                    else if (current_submit_scope_index == buffer_first_use.submit_scope_index)
                    {
                        buffer_first_use.task_batch_index = std::min(buffer_first_use.task_batch_index, batch_index);
                    }

                    if (current_submit_scope_index > buffer_last_use.submit_scope_index ||
                        buffer_last_use.submit_scope_index == std::numeric_limits<u32>::max())
                    {
                        buffer_last_use.submit_scope_index = current_submit_scope_index;
                        buffer_last_use.task_batch_index = batch_index;
                    }
                    else if (current_submit_scope_index == buffer_last_use.submit_scope_index)
                    {
                        buffer_last_use.task_batch_index = std::max(buffer_last_use.task_batch_index, batch_index);
                    }
                }
                // When the last use was a read AND the new use of the buffer is a read AND,
                // we need to add our stage flags to the existing barrier of the last use.
                bool const is_last_access_read = task_buffer.latest_access.type == AccessTypeFlagBits::READ;
                bool const is_current_access_read = current_buffer_access.type == AccessTypeFlagBits::READ;
                // We only need barriers between two accesses.
                // If the previous access is none, the current access is the first access.
                // Therefore we do not need to insert any synchronization if the previous access is none.
                // This is buffer specific. Images have a layout that needs to be set from undefined to the current accesses layout.
                // When the latest access  is a read that did not require a barrier before we also do not need a barrier now.
                // So skip, if the latest access is read and there is no latest_access_read_barrier_index present.
                bool const is_last_access_none = task_buffer.latest_access == AccessConsts::NONE;
                if (!is_last_access_none && !(std::holds_alternative<std::monostate>(task_buffer.latest_access_read_barrier_index) && is_last_access_read))
                {
                    if (is_last_access_read && is_current_access_read)
                    {
                        if (LastReadSplitBarrierIndex const * index0 = std::get_if<LastReadSplitBarrierIndex>(&task_buffer.latest_access_read_barrier_index))
                        {
                            auto & last_read_split_barrier = this->split_barriers[index0->index];
                            last_read_split_barrier.dst_access = last_read_split_barrier.dst_access | current_buffer_access;
                        }
                        else if (LastReadBarrierIndex const * index1 = std::get_if<LastReadBarrierIndex>(&task_buffer.latest_access_read_barrier_index))
                        {
                            auto & last_read_barrier = this->barriers[index1->index];
                            last_read_barrier.dst_access = last_read_barrier.dst_access | current_buffer_access;
                        }
                    }
                    else
                    {
                        // When the uses are incompatible (no read on read) we need to insert a new barrier.
                        // Host access needs to be handled in a specialized way.
                        bool const src_host_only_access = task_buffer.latest_access.stages == PipelineStageFlagBits::HOST;
                        bool const dst_host_only_access = current_buffer_access.stages == PipelineStageFlagBits::HOST;
                        DAXA_DBG_ASSERT_TRUE_M(!(src_host_only_access && dst_host_only_access), "direct sync between two host accesses on gpu are not allowed");
                        bool const is_host_barrier = src_host_only_access || dst_host_only_access;
                        // When the distance between src and dst batch is one, we can replace the split barrier with a normal barrier.
                        // We also need to make sure we do not use split barriers when the src or dst stage exclusively uses the host stage.
                        // This is because the host stage does not declare an execution dependency on the cpu but only a memory dependency.
                        bool const use_pipeline_barrier =
                            (task_buffer.latest_access_batch_index + 1 == batch_index &&
                             current_submit_scope_index == task_buffer.latest_access_submit_scope_index) ||
                            is_host_barrier;
                        if (use_pipeline_barrier)
                        {
                            usize const barrier_index = this->barriers.size();
                            this->barriers.push_back(TaskBarrier{
                                .image_id = {}, // {} signals that this is not an image barrier.
                                .src_access = task_buffer.latest_access,
                                .dst_access = current_buffer_access,
                                .src_batch = task_buffer.latest_access_batch_index,
                                .dst_batch = batch_index,
                            });
                            // And we insert the barrier index into the list of pipeline barriers of the current tasks batch.
                            batch.pipeline_barrier_indices.push_back(barrier_index);
                            if (current_buffer_access.type == AccessTypeFlagBits::READ)
                            {
                                // As the new access is a read we remember our barrier index,
                                // So that potential future reads after this can reuse this barrier.
                                task_buffer.latest_access_read_barrier_index = LastReadBarrierIndex{barrier_index};
                            }
                        }
                        else
                        {
                            usize const split_barrier_index = this->split_barriers.size();
                            this->split_barriers.push_back(TaskSplitBarrier{
                                {
                                    .image_id = {}, // {} signals that this is not an image barrier.
                                    .src_access = task_buffer.latest_access,
                                    .dst_access = current_buffer_access,
                                    .src_batch = task_buffer.latest_access_batch_index,
                                    .dst_batch = batch_index,
                                },
                                /* .split_barrier_state = */ task_list_impl.info.device.create_split_barrier({
                                    .name = std::string("TaskList \"") + task_list_impl.info.name + "\" SplitBarrier Nr. " + std::to_string(split_barrier_index),
                                }),
                            });
                            // Now we give the src batch the index of this barrier to signal.
                            TaskBatchSubmitScope & src_scope = this->batch_submit_scopes[task_buffer.latest_access_submit_scope_index];
                            TaskBatch & src_batch = src_scope.task_batches[task_buffer.latest_access_batch_index];
                            src_batch.signal_split_barrier_indices.push_back(split_barrier_index);
                            // And we also insert the split barrier index into the waits of the current tasks batch.
                            batch.wait_split_barrier_indices.push_back(split_barrier_index);
                            if (current_buffer_access.type == AccessTypeFlagBits::READ)
                            {
                                // As the new access is a read we remember our barrier index,
                                // So that potential future reads after this can reuse this barrier.
                                task_buffer.latest_access_read_barrier_index = LastReadSplitBarrierIndex{split_barrier_index};
                            }
                            else
                            {
                                task_buffer.latest_access_read_barrier_index = {};
                            }
                        }
                    }
                }
                // Now that we inserted/updated the synchronization, we update the latest access.
                task_buffer.latest_access = current_buffer_access;
                task_buffer.latest_access_batch_index = batch_index;
                task_buffer.latest_access_submit_scope_index = current_submit_scope_index;
            },
            [&](u32, TaskImageUse<> const & image_use)
            {
                auto const & used_image_t_id = image_use.handle;
                auto const & used_image_t_access = image_use.access();
                auto const & initial_used_image_slice = image_use.handle.slice;
                PerPermTaskImage & task_image = this->image_infos[used_image_t_id.index];
                // For transient images we need to record first and last use so that we can later name their allocations
                // TODO(msakmary, pahrens) We should think about how to combine this with update_image_inital_slices below since
                // they both overlap in what they are doing
                if (!task_list_impl.global_image_infos.at(used_image_t_id.index).is_persistent())
                {
                    auto & image_first_use = task_image.lifetime.first_use;
                    auto & image_last_use = task_image.lifetime.last_use;
                    if (current_submit_scope_index < image_first_use.submit_scope_index)
                    {
                        image_first_use.submit_scope_index = current_submit_scope_index;
                        image_first_use.task_batch_index = batch_index;
                    }
                    else if (current_submit_scope_index == image_first_use.submit_scope_index)
                    {
                        image_first_use.task_batch_index = std::min(image_first_use.task_batch_index, batch_index);
                    }

                    if (current_submit_scope_index > image_last_use.submit_scope_index ||
                        image_last_use.submit_scope_index == std::numeric_limits<u32>::max())
                    {
                        image_last_use.submit_scope_index = current_submit_scope_index;
                        image_last_use.task_batch_index = batch_index;
                    }
                    else if (current_submit_scope_index == image_last_use.submit_scope_index)
                    {
                        image_last_use.task_batch_index = std::max(image_last_use.task_batch_index, batch_index);
                    }
                }
                task_image.usage |= access_to_usage(used_image_t_access);
                auto [current_image_layout, current_image_access] = task_image_access_to_layout_access(used_image_t_access);
                // Now this seems strange, why would be need multiple current use slices, as we only have one here.
                // This is because when we intersect this slice with the tracked slices, we get an intersection and a rest.
                // We need to then test the rest against all the remaining tracked uses,
                // as the intersected part is already beeing handled in the following code.
                tl_new_use_slices.push_back(initial_used_image_slice);
                // This is the tracked slice we will insert after we finished analyzing the current used image.
                ExtendedImageSliceState ret_new_use_tracked_slice{
                    .state = {
                        .latest_access = current_image_access,
                        .latest_layout = current_image_layout,
                        .slice = initial_used_image_slice,
                    },
                    .latest_access_batch_index = batch_index,
                    .latest_access_submit_scope_index = current_submit_scope_index,
                    .latest_access_read_barrier_index = {}, // This is a dummy value (either set later or ignored entirely).
                };
                // We update the initial access slices.
                update_image_initial_access_slices(task_image, ret_new_use_tracked_slice);
                // As image subresources can be in different layouts and also different synchronization scopes,
                // we need to track these image ranges individually.
                for (
                    auto tracked_slice_iter = task_image.last_slice_states.begin();
                    tracked_slice_iter != task_image.last_slice_states.end();)
                {
                    bool advanced_tracked_slice_iterator = false;
                    for (
                        auto used_image_slice_iter = tl_new_use_slices.begin();
                        used_image_slice_iter != tl_new_use_slices.end();)
                    {
                        // We make a local copy of both slices here.
                        // We can not rely on dereferencing the iterators, as we modify them in this function.
                        // For this inner loop we want to remember the information about these slices,
                        // even after they are removed from their respective vector.
                        ImageMipArraySlice const used_image_slice = *used_image_slice_iter;
                        ExtendedImageSliceState const tracked_slice = *tracked_slice_iter;
                        // We are only interested in intersecting ranges, as use of non intersecting ranges does not need synchronization.
                        if (!used_image_slice.intersects(tracked_slice.state.slice))
                        {
                            // We only need to advance the iterator manually here.
                            // After this if statement there is an unconditional erase that advances the iterator if this is not hit.
                            ++used_image_slice_iter;
                            continue;
                        }
                        // As we found an intersection, part of the old tracked slice must be altered.
                        // Instead of altering it we remove it and add the rest slices back in later.
                        used_image_slice_iter = tl_new_use_slices.erase(used_image_slice_iter);
                        // Now we know that the new use intersects slices with a previous use.
                        // This means that we need to find the intersecting part,
                        // sync the intersecting part from the previous use to the current use
                        // and remove the overlapping part from the tracked slice.
                        // Now we need to split the uses into three groups:
                        // * the slice that is the intersection of tracked image slice and current new use (intersection)
                        // * the part of the tracked image slice that does not intersect with the current new use (tracked_slice_rest)
                        // * the part of the current new use slice that does not intersect with the tracked image (new_use_slice_rest)
                        auto intersection = tracked_slice.state.slice.intersect(used_image_slice);
                        auto [tracked_slice_rest, tracked_slice_rest_count] = tracked_slice.state.slice.subtract(intersection);
                        auto [new_use_slice_rest, new_use_slice_rest_count] = used_image_slice.subtract(intersection);
                        // We now remove the old tracked slice from the list of tracked slices, as we just split it.
                        // We need to know if the iterator was advanced. This erase advances the iterator.
                        // If the iterator got not advanced by this we need to advance it ourself manually later.
                        advanced_tracked_slice_iterator = true;
                        tracked_slice_iter = task_image.last_slice_states.erase(tracked_slice_iter);
                        // Now we remember the left over slice from the original tracked slice.
                        for (usize rest_i = 0; rest_i < tracked_slice_rest_count; ++rest_i)
                        {
                            // The rest tracked slices are the same as the original tracked slice,
                            // except for the slice itself, which is the remainder of the subtraction of the intersection.
                            ExtendedImageSliceState current_rest_tracked_slice = tracked_slice;
                            current_rest_tracked_slice.state.slice = tracked_slice_rest[rest_i];
                            tl_tracked_slice_rests.push_back(current_rest_tracked_slice);
                        }
                        // Now we remember the left over slice from our current used slice.
                        for (usize rest_i = 0; rest_i < new_use_slice_rest_count; ++rest_i)
                        {
                            // We reassign the iterator here as it is getting invalidated by the insert.
                            // The new iterator points to the newly inserted element.
                            // When the next iteration of the outer for loop starts, all these elements get skipped.
                            // This is good as these elements do NOT intersect with the currently inspected tracked slice.
                            used_image_slice_iter = tl_new_use_slices.insert(
                                tl_new_use_slices.end(),
                                new_use_slice_rest[rest_i]);
                        }
                        // Every other access (NONE, READ_WRITE, WRITE) are interpreted as writes in this context.
                        // When the last use was a read AND the new use of the buffer is a read AND,
                        // we need to add our stage flags to the existing barrier of the last use.
                        // To be able to do this the layout of the image slice must also match.
                        // If they differ we need to insert an execution barrier with a layout transition.
                        bool const is_last_access_read = tracked_slice.state.latest_access.type == AccessTypeFlagBits::READ;
                        bool const is_current_access_read = current_image_access.type == AccessTypeFlagBits::READ;
                        bool const are_layouts_identical = tracked_slice.state.latest_layout == current_image_layout;
                        if (is_last_access_read && is_current_access_read && are_layouts_identical)
                        {
                            if (LastReadSplitBarrierIndex const * index0 = std::get_if<LastReadSplitBarrierIndex>(&tracked_slice.latest_access_read_barrier_index))
                            {
                                auto & last_read_split_barrier = this->split_barriers[index0->index];
                                last_read_split_barrier.dst_access = last_read_split_barrier.dst_access | tracked_slice.state.latest_access;
                            }
                            else if (LastReadBarrierIndex const * index1 = std::get_if<LastReadBarrierIndex>(&tracked_slice.latest_access_read_barrier_index))
                            {
                                auto & last_read_barrier = this->barriers[index1->index];
                                last_read_barrier.dst_access = last_read_barrier.dst_access | tracked_slice.state.latest_access;
                            }
                        }
                        else
                        {
                            // When the uses are incompatible (no read on read, or no identical layout) we need to insert a new barrier.
                            // Host access needs to be handled in a specialized way.
                            bool const src_host_only_access = tracked_slice.state.latest_access.stages == PipelineStageFlagBits::HOST;
                            bool const dst_host_only_access = current_image_access.stages == PipelineStageFlagBits::HOST;
                            DAXA_DBG_ASSERT_TRUE_M(!(src_host_only_access && dst_host_only_access), "direct sync between two host accesses on gpu is not allowed");
                            bool const is_host_barrier = src_host_only_access || dst_host_only_access;
                            // When the distance between src and dst batch is one, we can replace the split barrier with a normal barrier.
                            // We also need to make sure we do not use split barriers when the src or dst stage exclusively uses the host stage.
                            // This is because the host stage does not declare an execution dependency on the cpu but only a memory dependency.
                            bool const use_pipeline_barrier =
                                (tracked_slice.latest_access_batch_index + 1 == batch_index &&
                                 current_submit_scope_index == tracked_slice.latest_access_submit_scope_index) ||
                                is_host_barrier;
                            if (use_pipeline_barrier)
                            {
                                usize const barrier_index = this->barriers.size();
                                this->barriers.push_back(TaskBarrier{
                                    .image_id = used_image_t_id,
                                    .slice = intersection,
                                    .layout_before = tracked_slice.state.latest_layout,
                                    .layout_after = current_image_layout,
                                    .src_access = tracked_slice.state.latest_access,
                                    .dst_access = current_image_access,
                                    .src_batch = tracked_slice.latest_access_batch_index,
                                    .dst_batch = batch_index,
                                });
                                // And we insert the barrier index into the list of pipeline barriers of the current tasks batch.
                                batch.pipeline_barrier_indices.push_back(barrier_index);
                                if (current_image_access.type == AccessTypeFlagBits::READ)
                                {
                                    // As the new access is a read we remember our barrier index,
                                    // So that potential future reads after this can reuse this barrier.
                                    ret_new_use_tracked_slice.latest_access_read_barrier_index = LastReadBarrierIndex{barrier_index};
                                }
                            }
                            else
                            {
                                usize const split_barrier_index = this->split_barriers.size();
                                this->split_barriers.push_back(TaskSplitBarrier{
                                    {
                                        .image_id = used_image_t_id,
                                        .slice = intersection,
                                        .layout_before = tracked_slice.state.latest_layout,
                                        .layout_after = current_image_layout,
                                        .src_access = tracked_slice.state.latest_access,
                                        .dst_access = current_image_access,
                                        .src_batch = tracked_slice.latest_access_batch_index,
                                        .dst_batch = batch_index,
                                    },
                                    /* .split_barrier_state = */ task_list_impl.info.device.create_split_barrier({
                                        .name = std::string("TaskList \"") + task_list_impl.info.name + "\" SplitBarrier (Image) Nr. " + std::to_string(split_barrier_index),
                                    }),
                                });
                                // Now we give the src batch the index of this barrier to signal.
                                TaskBatchSubmitScope & src_scope = this->batch_submit_scopes[tracked_slice.latest_access_submit_scope_index];
                                TaskBatch & src_batch = src_scope.task_batches[tracked_slice.latest_access_batch_index];
                                src_batch.signal_split_barrier_indices.push_back(split_barrier_index);
                                // And we also insert the split barrier index into the waits of the current tasks batch.
                                batch.wait_split_barrier_indices.push_back(split_barrier_index);
                                if (current_image_access.type == AccessTypeFlagBits::READ)
                                {
                                    // As the new access is a read we remember our barrier index,
                                    // So that potential future reads after this can reuse this barrier.
                                    ret_new_use_tracked_slice.latest_access_read_barrier_index = LastReadSplitBarrierIndex{split_barrier_index};
                                }
                                else
                                {
                                    ret_new_use_tracked_slice.latest_access_read_barrier_index = {};
                                }
                            }
                        }
                        // Make sure we have any tracked slices to intersect with left.
                        if (tracked_slice_iter == task_image.last_slice_states.end())
                        {
                            break;
                        }
                    }
                    if (!advanced_tracked_slice_iterator)
                    {
                        // If we didn't find any intersections, we dont remove the tracked slice.
                        // Erasing a tracked slice "advances" iterator. As we did not remove,
                        // we need to advance it manually.
                        ++tracked_slice_iter;
                    }
                }
                tl_new_use_slices.clear();
                // Now we need to add the latest use and tracked range of our current access:
                task_image.last_slice_states.push_back(ret_new_use_tracked_slice);
                // The remainder tracked slices we remembered from earlier are now inserted back into the list of tracked slices.
                // We deferred this step as we dont want to check these in the loop above, as we found them to not intersect with the new use.
                task_image.last_slice_states.insert(
                    task_image.last_slice_states.end(),
                    tl_tracked_slice_rests.begin(),
                    tl_tracked_slice_rests.end());
                tl_tracked_slice_rests.clear();
            });
    }

    void TaskList::submit(TaskSubmitInfo const & info)
    {
        auto & impl = *as<ImplTaskList>();
        DAXA_DBG_ASSERT_TRUE_M(!impl.compiled, "completed task lists can not record new tasks");

        for (auto & permutation : impl.record_active_permutations)
        {
            permutation->submit(info);
        }
    }

    void TaskListPermutation::submit(TaskSubmitInfo const & info)
    {
        TaskBatchSubmitScope & submit_scope = this->batch_submit_scopes.back();
        submit_scope.submit_info = {};
        // We provide the user submit info to the submit batch.
        submit_scope.user_submit_info = info;
        // Start a new batch.
        this->batch_submit_scopes.push_back({
            .present_info = {},
        });
    }

    void TaskList::present(TaskPresentInfo const & info)
    {
        auto & impl = *as<ImplTaskList>();
        DAXA_DBG_ASSERT_TRUE_M(!impl.compiled, "completed task lists can not record new tasks");
        DAXA_DBG_ASSERT_TRUE_M(impl.info.swapchain.has_value(), "can only present, when a swapchain was provided in creation");

        for (auto & permutation : impl.record_active_permutations)
        {
            permutation->present(info);
        }
    }

    void TaskListPermutation::present(TaskPresentInfo const & info)
    {
        DAXA_DBG_ASSERT_TRUE_M(this->batch_submit_scopes.size() > 1, "can only present if at least one submit was issued before");
        DAXA_DBG_ASSERT_TRUE_M(!this->swapchain_image.is_empty(), "can only present when an image was annotated as swapchain image");
        DAXA_DBG_ASSERT_TRUE_M(!this->image_infos[this->swapchain_image.index].swapchain_semaphore_waited_upon, "Can only present once");
        this->image_infos[this->swapchain_image.index].swapchain_semaphore_waited_upon = true;

        ExtendedImageSliceState default_slice;
        ExtendedImageSliceState const * tracked_slice = {};
        if (this->image_infos[this->swapchain_image.index].last_slice_states.empty())
        {
            tracked_slice = &default_slice;
        }
        else
        {
            tracked_slice = &this->image_infos[this->swapchain_image.index].last_slice_states.back();
        }
        usize const submit_scope_index = tracked_slice->latest_access_submit_scope_index;
        DAXA_DBG_ASSERT_TRUE_M(submit_scope_index < this->batch_submit_scopes.size() - 1, "the last swapchain image use MUST be before the last submit when presenting");
        TaskBatchSubmitScope & submit_scope = this->batch_submit_scopes[submit_scope_index];
        usize const batch_index = tracked_slice->latest_access_batch_index;
        // We need to insert a pipeline barrier to transition the swapchain image layout to present src optimal.
        usize const barrier_index = this->barriers.size();
        this->barriers.push_back(TaskBarrier{
            .image_id = this->swapchain_image,
            .slice = tracked_slice->state.slice,
            .layout_before = tracked_slice->state.latest_layout,
            .layout_after = ImageLayout::PRESENT_SRC,
            .src_access = tracked_slice->state.latest_access,
            .dst_access = {.stages = PipelineStageFlagBits::BOTTOM_OF_PIPE},
            .src_batch = batch_index,
            .dst_batch = batch_index + 1,
        });
        submit_scope.last_minute_barrier_indices.push_back(barrier_index);
        // Now we need to insert the binary semaphore between submit and present.
        submit_scope.present_info = ImplPresentInfo{
            .additional_binary_semaphores = info.additional_binary_semaphores,
        };
    }

    void ImplTaskList::create_transient_runtime_buffers(TaskListPermutation & permutation)
    {
        for (u32 buffer_info_idx = 0; buffer_info_idx < u32(global_buffer_infos.size()); buffer_info_idx++)
        {
            auto const & glob_buffer = global_buffer_infos.at(buffer_info_idx);
            auto & perm_buffer = permutation.buffer_infos.at(buffer_info_idx);

            if (!glob_buffer.is_persistent() && perm_buffer.valid)
            {
                auto const & transient_info = std::get<PermIndepTaskBufferInfo::Transient>(glob_buffer.task_buffer_data);

                perm_buffer.actual_buffer = info.device.create_buffer(BufferInfo{
                    .size = transient_info.info.size,
                    .allocate_info = ManualAllocInfo{
                        .memory_block = transient_data_memory_block,
                        .offset = perm_buffer.allocation_offset},
                    .name = transient_info.info.name});
            }
        }
    }

    void ImplTaskList::create_transient_runtime_images(TaskListPermutation & permutation)
    {
        for (u32 image_info_idx = 0; image_info_idx < u32(global_image_infos.size()); image_info_idx++)
        {
            auto const & glob_image = global_image_infos.at(image_info_idx);
            auto & perm_image = permutation.image_infos.at(image_info_idx);

            if (!glob_image.is_persistent() && perm_image.valid)
            {
                DAXA_DBG_ASSERT_TRUE_M(perm_image.usage != ImageUsageFlagBits::NONE,
                                       std::string("Transient image is not used in this permutation but marked as valid either: ") +
                                           std::string("\t- it was used as PRESENT which is not allowed for transient images") +
                                           std::string("\t- it was used as NONE which makes no sense - just don't mark it as used in the task"));
                auto const & transient_image_info = std::get<PermIndepTaskImageInfo::Transient>(glob_image.task_image_data).info;
                perm_image.actual_image = info.device.create_image(ImageInfo{
                    .dimensions = transient_image_info.dimensions,
                    .format = transient_image_info.format,
                    .aspect = transient_image_info.aspect,
                    .size = transient_image_info.size,
                    .mip_level_count = transient_image_info.mip_level_count,
                    .array_layer_count = transient_image_info.array_layer_count,
                    .sample_count = transient_image_info.sample_count,
                    .usage = perm_image.usage,
                    .allocate_info = ManualAllocInfo{
                        .memory_block = transient_data_memory_block,
                        .offset = perm_image.allocation_offset},
                    .name = transient_image_info.name,
                });
            }
        }
    }

    void ImplTaskList::allocate_transient_resources()
    {
        // figure out transient resource sizes
        usize max_alignment_requirement = 0;
        for (auto & global_image : global_image_infos)
        {
            if (!global_image.is_persistent())
            {
                auto & transient_image = std::get<PermIndepTaskImageInfo::Transient>(global_image.task_image_data);
                ImageInfo const image_info = {
                    .dimensions = transient_image.info.dimensions,
                    .format = transient_image.info.format,
                    .aspect = transient_image.info.aspect,
                    .size = transient_image.info.size,
                    .mip_level_count = transient_image.info.mip_level_count,
                    .array_layer_count = transient_image.info.array_layer_count,
                    .sample_count = transient_image.info.sample_count,
                    .usage = ImageUsageFlagBits::SHADER_READ_WRITE,
                    .allocate_info = MemoryFlagBits::DEDICATED_MEMORY,
                    .name = "Dummy to figure mem requirements",
                };
                transient_image.memory_requirements = info.device.get_memory_requirements({image_info});
                max_alignment_requirement = std::max(transient_image.memory_requirements.alignment, max_alignment_requirement);
            }
        }
        for (auto & global_buffer : global_buffer_infos)
        {
            if (!global_buffer.is_persistent())
            {
                auto & transient_buffer = std::get<PermIndepTaskBufferInfo::Transient>(global_buffer.task_buffer_data);
                BufferInfo const buffer_info = {
                    .size = transient_buffer.info.size,
                    .allocate_info = MemoryFlagBits::DEDICATED_MEMORY,
                    .name = "Dummy to figure mem requirements"};
                transient_buffer.memory_requirements = info.device.get_memory_requirements({buffer_info});
                max_alignment_requirement = std::max(transient_buffer.memory_requirements.alignment, max_alignment_requirement);
            }
        }

        // for each permutation figure out the max memory requirements
        for (auto & permutation : permutations)
        {
            usize batches = 0;
            std::vector<usize> submit_batch_offsets(permutation.batch_submit_scopes.size());
            for (u32 submit_scope_idx = 0; submit_scope_idx < permutation.batch_submit_scopes.size(); submit_scope_idx++)
            {
                submit_batch_offsets.at(submit_scope_idx) = batches;
                batches += permutation.batch_submit_scopes.at(submit_scope_idx).task_batches.size();
            }

            struct LifetimeLengthResource
            {
                usize start_batch;
                usize end_batch;
                usize lifetime_lenght;
                bool is_image;
                u32 resource_idx;
            };

            std::vector<LifetimeLengthResource> lifetime_length_sorted_resources;

            for (u32 perm_image_idx = 0; perm_image_idx < permutation.image_infos.size(); perm_image_idx++)
            {
                if (global_image_infos.at(perm_image_idx).is_persistent() || !permutation.image_infos.at(perm_image_idx).valid)
                {
                    continue;
                }

                auto const & perm_task_image = permutation.image_infos.at(perm_image_idx);

                if (perm_task_image.lifetime.first_use.submit_scope_index == std::numeric_limits<u32>::max() ||
                    perm_task_image.lifetime.last_use.submit_scope_index == std::numeric_limits<u32>::max())
                {
                    permutation.image_infos.at(perm_image_idx).valid = false;
                    continue;
                }

                usize start_idx = submit_batch_offsets.at(perm_task_image.lifetime.first_use.submit_scope_index) +
                                  perm_task_image.lifetime.first_use.task_batch_index;
                usize end_idx = submit_batch_offsets.at(perm_task_image.lifetime.last_use.submit_scope_index) +
                                perm_task_image.lifetime.last_use.task_batch_index;

                lifetime_length_sorted_resources.emplace_back(LifetimeLengthResource{
                    .start_batch = start_idx,
                    .end_batch = end_idx,
                    .lifetime_lenght = end_idx - start_idx + 1,
                    .is_image = true,
                    .resource_idx = perm_image_idx,
                });
            }

            for (u32 perm_buffer_idx = 0; perm_buffer_idx < permutation.buffer_infos.size(); perm_buffer_idx++)
            {
                if (global_buffer_infos.at(perm_buffer_idx).is_persistent())
                {
                    continue;
                }

                auto const & perm_task_buffer = permutation.buffer_infos.at(perm_buffer_idx);
                usize start_idx = submit_batch_offsets.at(perm_task_buffer.lifetime.first_use.submit_scope_index) +
                                  perm_task_buffer.lifetime.first_use.task_batch_index;
                usize end_idx = submit_batch_offsets.at(perm_task_buffer.lifetime.last_use.submit_scope_index) +
                                perm_task_buffer.lifetime.last_use.task_batch_index;

                DAXA_DBG_ASSERT_TRUE_M(start_idx != std::numeric_limits<u32>::max() ||
                                           end_idx != std::numeric_limits<u32>::max(),
                                       "Detected transient resource created but never used");
                lifetime_length_sorted_resources.emplace_back(LifetimeLengthResource{
                    .start_batch = start_idx,
                    .end_batch = end_idx,
                    .lifetime_lenght = end_idx - start_idx + 1,
                    .is_image = false,
                    .resource_idx = perm_buffer_idx,
                });
            }

            std::sort(lifetime_length_sorted_resources.begin(), lifetime_length_sorted_resources.end(),
                      [](LifetimeLengthResource const & first, LifetimeLengthResource const & second) -> bool
                      {
                          return first.lifetime_lenght > second.lifetime_lenght;
                      });

            struct Allocation
            {
                usize offset = {};
                usize size = {};
                usize start_batch = {};
                usize end_batch = {};
                bool is_image = {};
                u32 owning_resource_idx = {};
                u32 memory_type_bits = {};
                ImageMipArraySlice intersection_object = {};
            };
            // Sort allocations in the set in the following way
            //      1) sort by offsets into the memory block
            //  if equal:
            //      2) sort by start batch of the allocation
            //  if equal:
            //      3) sort by owning image index
            struct AllocCompare
            {
                constexpr auto operator()(Allocation const & first, Allocation const & second) const -> bool
                {
                    if (first.offset < second.offset)
                    {
                        return true;
                    }
                    if (first.offset == second.offset)
                    {
                        if (first.start_batch < second.start_batch)
                        {
                            return true;
                        }
                        if (first.start_batch == second.start_batch)
                        {
                            return first.owning_resource_idx < second.owning_resource_idx;
                        }
                        // first.offset == second.offset && first.start_batch > second.start_batch
                        return false;
                    }
                    // first.offset > second.offset
                    return false;
                };
            };
            std::set<Allocation, AllocCompare> allocations = {};
            // Figure out where to allocate each resource
            for (auto const & resource_lifetime : lifetime_length_sorted_resources)
            {
                MemoryRequirements mem_requirements;
                if (resource_lifetime.is_image)
                {
                    mem_requirements = std::get<PermIndepTaskImageInfo::Transient>(
                                           global_image_infos.at(resource_lifetime.resource_idx).task_image_data)
                                           .memory_requirements;
                }
                else
                {
                    mem_requirements = std::get<PermIndepTaskBufferInfo::Transient>(
                                           global_buffer_infos.at(resource_lifetime.resource_idx).task_buffer_data)
                                           .memory_requirements;
                }
                // Go through all memory block states in which this resource is alive and try to find a spot for it
                const u8 resource_lifetime_duration = resource_lifetime.end_batch - resource_lifetime.start_batch + 1;
                Allocation new_allocation = Allocation{
                    .offset = 0,
                    .size = mem_requirements.size,
                    .start_batch = resource_lifetime.start_batch,
                    .end_batch = resource_lifetime.end_batch,
                    .is_image = resource_lifetime.is_image,
                    .owning_resource_idx = resource_lifetime.resource_idx,
                    .memory_type_bits = mem_requirements.memory_type_bits,
                    .intersection_object = {
                        .base_mip_level = static_cast<u8>(resource_lifetime.start_batch),
                        .level_count = resource_lifetime_duration,
                        .base_array_layer = static_cast<u16>(0),
                        .layer_count = static_cast<u32>(mem_requirements.size),
                    }};

                // TODO(msakmary) Fix the intersect functionality so that it is general and does not do hacky stuff like constructing
                // a mip array slice
                // Find space in memory and time the new allocation fits into.
                for (auto const & allocation : allocations)
                {
                    if (new_allocation.intersection_object.intersects(allocation.intersection_object))
                    {
                        // assign new offset into the memory block - we need to guarantee correct allignment
                        usize curr_offset = allocation.offset + allocation.size;
                        usize const align = std::max(mem_requirements.alignment, 1ull);
                        usize const aligned_curr_offset = (curr_offset + align - 1) / align * align;
                        new_allocation.offset = aligned_curr_offset;
                        new_allocation.intersection_object.base_array_layer = static_cast<u32>(new_allocation.offset);
                    }
                }
                allocations.insert(new_allocation);
            }
            // Once we are done with finding space for all the allocations go through all permutation images and copy over the allocation information
            for (auto const & allocation : allocations)
            {
                if (allocation.is_image)
                {
                    permutation.image_infos.at(allocation.owning_resource_idx).allocation_offset = allocation.offset;
                }
                else
                {
                    permutation.buffer_infos.at(allocation.owning_resource_idx).allocation_offset = allocation.offset;
                }
                // find the amount of memory this permutation requires
                memory_block_size = std::max(memory_block_size, allocation.offset + allocation.size);
                memory_type_bits = memory_type_bits | allocation.memory_type_bits;
            }
        }
        transient_data_memory_block = info.device.create_memory({
            .requirements = {
                .size = memory_block_size,
                .alignment = max_alignment_requirement,
                .memory_type_bits = memory_type_bits,
            },
            .flags = MemoryFlagBits::DEDICATED_MEMORY,
        });
    }

    void TaskList::complete(TaskCompleteInfo const &)
    {
        auto & impl = *as<ImplTaskList>();
        DAXA_DBG_ASSERT_TRUE_M(!impl.compiled, "task lists can only be completed once");
        impl.compiled = true;

        impl.allocate_transient_resources();
        // Insert static barriers initializing image layouts.
        for (auto & permutation : impl.permutations)
        {
            impl.create_transient_runtime_buffers(permutation);
            impl.create_transient_runtime_images(permutation);
            auto & first_batch = permutation.batch_submit_scopes[0].task_batches[0];
            // Insert static initialization barriers for non persistent resources:
            // Buffers never need layout initialization, only images.
            for (u32 task_image_index = 0; task_image_index < permutation.image_infos.size(); ++task_image_index)
            {
                TaskImageHandle const task_image_id = {impl.unique_index, task_image_index};
                auto & task_image = permutation.image_infos[task_image_index];
                PermIndepTaskImageInfo const & glob_task_image = impl.global_image_infos[task_image_index];
                if (task_image.valid && !glob_task_image.is_persistent())
                {
                    // Insert barriers, initializing all the initially accesses subresource ranges to the correct layout.
                    for (auto & first_access : task_image.first_slice_states)
                    {
                        usize const new_barrier_index = permutation.barriers.size();
                        permutation.barriers.push_back(TaskBarrier{
                            .image_id = task_image_id,
                            .slice = first_access.state.slice,
                            .layout_before = {},
                            .layout_after = first_access.state.latest_layout,
                            .src_access = {},
                            .dst_access = first_access.state.latest_access,
                            .src_batch = {},
                            .dst_batch = first_access.latest_access_batch_index,
                        });
                        first_batch.pipeline_barrier_indices.push_back(new_barrier_index);
                    }
                }
            }
        }
    }

    auto TaskList::get_command_lists() -> std::vector<CommandList>
    {
        auto & impl = *as<ImplTaskList>();
        DAXA_DBG_ASSERT_TRUE_M(impl.compiled, "Can only get command lists of a finished task list");
        DAXA_DBG_ASSERT_TRUE_M(!impl.executed_once, "Can only get command lists of a task list that has been executed");
        auto command_lists = std::move(impl.left_over_command_lists);
        impl.left_over_command_lists = {};
        return command_lists;
    }

    auto TaskList::get_debug_string() -> std::string
    {
        auto & impl = *as<ImplTaskList>();
        DAXA_DBG_ASSERT_TRUE_M(impl.info.record_debug_information,
                               "in order to have debug string you need to set record_debug_information flag to true on task list creation");
        DAXA_DBG_ASSERT_TRUE_M(impl.executed_once,
                               "in order to have debug string you need to execute the task list at least once");
        std::string ret = impl.debug_string_stream.str();
        impl.debug_string_stream.str("");
        return ret;
    }

    thread_local std::vector<SplitBarrierWaitInfo> tl_split_barrier_wait_infos = {};
    thread_local std::vector<ImageBarrierInfo> tl_image_barrier_infos = {};
    thread_local std::vector<MemoryBarrierInfo> tl_memory_barrier_infos = {};
    void insert_pipeline_barrier(ImplTaskList const & impl, TaskListPermutation & perm, CommandList & command_list, TaskBarrier & barrier)
    {
        // Check if barrier is image barrier or normal barrier (see TaskBarrier struct comments).
        if (barrier.image_id.is_empty())
        {
            command_list.pipeline_barrier({
                .src_access = barrier.src_access,
                .dst_access = barrier.dst_access,
            });
        }
        else
        {
            auto const actual_images = impl.get_actual_images(barrier.image_id, perm);
            for (usize index = 0; index < actual_images.size(); ++index)
            {
                auto & image = actual_images[index];
                DAXA_DBG_ASSERT_TRUE_M(
                    impl.info.device.is_id_valid(image),
                    std::string("detected invalid runtime image id while inserting barriers: the runtime image id at index ") +
                        std::to_string(index) +
                        std::string(" of task image \"") +
                        std::string(impl.global_image_infos[barrier.image_id.index].get_name()) +
                        std::string("\" is invalid"));
                command_list.pipeline_barrier_image_transition({
                    .src_access = barrier.src_access,
                    .dst_access = barrier.dst_access,
                    .src_layout = barrier.layout_before,
                    .dst_layout = barrier.layout_after,
                    .image_slice = barrier.slice,
                    .image_id = image,
                });
            }
        }
    }

    void generate_persistent_resource_synch(
        ImplTaskList & impl,
        TaskListPermutation & permutation,
        CommandList & cmd_list)
    {
        // Persistent resources need just in time synch between executions,
        // as pre generating the transitions between all permutations is not manageable.
        std::string out = "";
        std::string indent = "";
        if (impl.info.record_debug_information)
        {
            fmt::format_to(std::back_inserter(out), "{}runtime sync memory barriers:\n", indent);
            begin_indent(out, indent, true);
        }
        for (usize task_buffer_index = 0; task_buffer_index < permutation.buffer_infos.size(); ++task_buffer_index)
        {
            auto & task_buffer = permutation.buffer_infos[task_buffer_index];
            auto & glob_buffer_info = impl.global_buffer_infos[task_buffer_index];
            if (task_buffer.valid && glob_buffer_info.is_persistent())
            {
                auto & persistent_data = glob_buffer_info.get_persistent();
                bool const no_prev_access = persistent_data.latest_access == AccessConsts::NONE;
                bool const read_on_read_same_access =
                    persistent_data.latest_access == permutation.buffer_infos[task_buffer_index].first_access &&
                    persistent_data.latest_access.type == AccessTypeFlagBits::READ;
                // TODO(pahrens,msakmary): read on read should only be set to true whenever the access the same and the STAGES are also the same
                // otherwise we need to generate barrier
                //      AS WE CANT MODIFY BARRIES FROM PREVIOUSLY ALREADY EXECUTED TASK LISTS, WE MUST OVERSYNC ON THE LAST WRITE TO READ
                //      WE CAN ONLY SKIP ON THE SAME ACCESS READ ON READ
                //      WE MUST REMEMBER THE LAST WRITE

                // bool const read_on_read =
                //     persistent_data.latest_access.type == AccessTypeFlagBits::READ &&
                //     permutation.buffer_infos[task_buffer_index].first_access.type == AccessTypeFlagBits::READ;
                // For now just oversync on reads.
                if (no_prev_access || read_on_read_same_access)
                {
                    // Skip buffers that have no previous access, as there is nothing to sync on.
                    continue;
                }

                MemoryBarrierInfo mem_barrier_info{
                    .src_access = persistent_data.latest_access,
                    .dst_access = permutation.buffer_infos[task_buffer_index].first_access,
                };
                cmd_list.pipeline_barrier(mem_barrier_info);
                if (impl.info.record_debug_information)
                {
                    fmt::format_to(std::back_inserter(out), "{}{}\n", indent, to_string(mem_barrier_info));
                    print_seperator_to(out, indent);
                }
                persistent_data.latest_access = {};
            }
        }
        if (impl.info.record_debug_information)
        {
            end_indent(out, indent);
            fmt::format_to(std::back_inserter(out), "{}runtime sync image memory barriers:\n", indent);
            begin_indent(out, indent, true);
        }
        // If parts of the first use slices to not intersect with any previous use,
        // we must synchronize on undefined layout!
        std::vector<ExtendedImageSliceState> remaining_first_accesses = {};
        for (u32 task_image_index = 0; task_image_index < permutation.image_infos.size(); ++task_image_index)
        {
            auto & task_image = permutation.image_infos[task_image_index];
            auto & exec_image = impl.global_image_infos[task_image_index];
            remaining_first_accesses = task_image.first_slice_states;
            // Iterate over all persistent images.
            // Find all intersections between tracked slices of first use and previous use.
            // Synch on the intersection and delete the intersected part from the tracked slice of the previous use.
            if (task_image.valid && exec_image.is_persistent())
            {
                if (impl.info.record_debug_information)
                {
                    fmt::format_to(std::back_inserter(out), "{}sync from previous uses:\n", indent);
                    begin_indent(out, indent, true);
                }
                auto & previous_access_slices = exec_image.get_persistent().latest_slice_states;
                for (u32 previous_access_slice_index = 0; previous_access_slice_index < previous_access_slices.size();)
                {
                    bool broke_inner_loop = false;
                    for (u32 first_access_slice_index = 0; first_access_slice_index < remaining_first_accesses.size(); ++first_access_slice_index)
                    {
                        // Dont sync on disjoint subresource uses.
                        if (!remaining_first_accesses[first_access_slice_index].state.slice.intersects(previous_access_slices[previous_access_slice_index].slice))
                        {
                            // Disjoint subresources or read on read with same layout.
                            continue;
                        }
                        // Intersect previous use and initial use.
                        // Record synchronization for the intersecting part.
                        auto intersection = previous_access_slices[previous_access_slice_index].slice.intersect(remaining_first_accesses[first_access_slice_index].state.slice);
                        // Dont sync on same accesses following each other.
                        bool const both_accesses_read =
                            remaining_first_accesses[first_access_slice_index].state.latest_access.type == AccessTypeFlagBits::READ &&
                            previous_access_slices[previous_access_slice_index].latest_access.type == AccessTypeFlagBits::READ;
                        bool const both_layouts_same =
                            remaining_first_accesses[first_access_slice_index].state.latest_layout ==
                            previous_access_slices[previous_access_slice_index].latest_layout;
                        if (!(both_accesses_read && both_layouts_same))
                        {
                            for (auto execution_image_id : impl.get_actual_images(TaskImageHandle{{.task_list_index = impl.unique_index, .index = task_image_index}}, permutation))
                            {
                                ImageBarrierInfo img_barrier_info{
                                    .src_access = previous_access_slices[previous_access_slice_index].latest_access,
                                    .dst_access = remaining_first_accesses[first_access_slice_index].state.latest_access,
                                    .src_layout = previous_access_slices[previous_access_slice_index].latest_layout,
                                    .dst_layout = remaining_first_accesses[first_access_slice_index].state.latest_layout,
                                    .image_slice = intersection,
                                    .image_id = execution_image_id,
                                };
                                cmd_list.pipeline_barrier_image_transition(img_barrier_info);
                                if (impl.info.record_debug_information)
                                {
                                    fmt::format_to(std::back_inserter(out), "{}{}\n", indent, to_string(img_barrier_info));
                                    print_seperator_to(out, indent);
                                }
                            }
                        }
                        // Put back the non intersecting rest into the previous use list.
                        auto [previous_use_slice_rest, previous_use_slice_rest_count] = previous_access_slices[previous_access_slice_index].slice.subtract(intersection);
                        auto [first_use_slice_rest, first_use_slice_rest_count] = remaining_first_accesses[first_access_slice_index].state.slice.subtract(intersection);
                        for (usize rest_slice_index = 0; rest_slice_index < previous_use_slice_rest_count; ++rest_slice_index)
                        {
                            auto rest_previous_slice = previous_access_slices[previous_access_slice_index];
                            rest_previous_slice.slice = previous_use_slice_rest[rest_slice_index];
                            previous_access_slices.push_back(rest_previous_slice);
                        }
                        // Append the new rest first uses.nd
                        for (usize rest_slice_index = 0; rest_slice_index < first_use_slice_rest_count; ++rest_slice_index)
                        {
                            auto rest_first_slice = remaining_first_accesses[first_access_slice_index];
                            rest_first_slice.state.slice = first_use_slice_rest[rest_slice_index];
                            remaining_first_accesses.push_back(rest_first_slice);
                        }
                        // Remove the previous use from the list, it is synchronized now.
                        previous_access_slices.erase(std::next(previous_access_slices.begin(), static_cast<ptrdiff_t>(previous_access_slice_index)));
                        // Remove the first use from the remaining first uses, as it was now synchronized from.
                        remaining_first_accesses.erase(std::next(remaining_first_accesses.begin(), static_cast<ptrdiff_t>(first_access_slice_index)));
                        // As we removed an element in this place,
                        // we dont need to advance the iterator as in its place there will be a new element already that we do not want to skip.
                        broke_inner_loop = true;
                        break;
                    }
                    if (!broke_inner_loop)
                    {
                        // We break the loop when we erase the current element.
                        // Erasing moved all elements past the current one one to the left.
                        // This means that the current element is already a new one,
                        // we do not want to skip it so we wont increment the index here.
                        ++previous_access_slice_index;
                    }
                }
                if (impl.info.record_debug_information)
                {
                    end_indent(out, indent);
                    fmt::format_to(std::back_inserter(out), "{}sync from undefined:\n", indent);
                    begin_indent(out, indent, true);
                }
                // For all first uses that did NOT intersect with and previous use,
                // we need to synchronize from an undefined state to initialize the layout of the image.
                for (usize remaining_first_uses_index = 0; remaining_first_uses_index < remaining_first_accesses.size(); ++remaining_first_uses_index)
                {
                    for (auto execution_image_id : impl.get_actual_images(TaskImageHandle{{.task_list_index = impl.unique_index, .index = task_image_index}}, permutation))
                    {
                        ImageBarrierInfo img_barrier_info{
                            .src_access = AccessConsts::NONE,
                            .dst_access = remaining_first_accesses[remaining_first_uses_index].state.latest_access,
                            .src_layout = ImageLayout::UNDEFINED,
                            .dst_layout = remaining_first_accesses[remaining_first_uses_index].state.latest_layout,
                            .image_slice = remaining_first_accesses[remaining_first_uses_index].state.slice,
                            .image_id = execution_image_id,
                        };
                        cmd_list.pipeline_barrier_image_transition(img_barrier_info);
                        if (impl.info.record_debug_information)
                        {
                            fmt::format_to(std::back_inserter(out), "{}{}\n", indent, to_string(img_barrier_info));
                            print_seperator_to(out, indent);
                        }
                    }
                }
                if (impl.info.record_debug_information)
                {
                    end_indent(out, indent);
                }
            }
        }
        if (impl.info.record_debug_information)
        {
            end_indent(out, indent);
            impl.debug_string_stream << out;
        }
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
    ///     2.2 check if submit scope submits work, either submit or collect cmd lists and sync primitives for querry
    ///     2.3 check if submit scope presents, present if true.
    void TaskList::execute(ExecutionInfo const & info)
    {
        auto & impl = *as<ImplTaskList>();
        DAXA_DBG_ASSERT_TRUE_M(info.permutation_condition_values.size() >= impl.info.permutation_condition_count, "detected invalid permutation condition count");
        DAXA_DBG_ASSERT_TRUE_M(impl.compiled, "task lists must be completed before execution");

        u32 permutation_index = {};
        for (u32 index = 0; index < std::min(usize(32), info.permutation_condition_values.size()); ++index)
        {
            permutation_index |= info.permutation_condition_values[index] ? (1u << index) : 0;
        }
        impl.chosen_permutation_last_execution = permutation_index;
        TaskListPermutation & permutation = impl.permutations[permutation_index];

        ImplTaskRuntimeInterface impl_runtime{.task_list = impl, .permutation = permutation};
        impl_runtime.command_lists.push_back(impl.info.device.create_command_list({.name = std::string("Task Command List ") + std::to_string(impl_runtime.command_lists.size())}));

        validate_runtime_resources(impl, permutation);
        // Generate and insert synchronization for persistent resources:
        generate_persistent_resource_synch(impl, permutation, impl_runtime.command_lists.back());

        usize submit_scope_index = 0;
        for (auto & submit_scope : permutation.batch_submit_scopes)
        {
            if (impl.info.enable_command_labels)
            {
                impl_runtime.command_lists.back().begin_label({
                    .label_name = impl.info.name + std::string(", submit ") + std::to_string(submit_scope_index),
                    .label_color = impl.info.task_list_label_color,
                });
            }
            usize batch_index = 0;
            for (auto & task_batch : submit_scope.task_batches)
            {
                if (impl.info.enable_command_labels)
                {
                    impl_runtime.command_lists.back().begin_label({
                        .label_name = impl.info.name + std::string(", submit ") + std::to_string(submit_scope_index) + std::string(", batch ") + std::to_string(batch_index),
                        .label_color = impl.info.task_batch_label_color,
                    });
                }
                batch_index += 1;
                // Wait on pipeline barriers before batch execution.
                for (auto barrier_index : task_batch.pipeline_barrier_indices)
                {
                    TaskBarrier & barrier = permutation.barriers[barrier_index];
                    insert_pipeline_barrier(impl, permutation, impl_runtime.command_lists.back(), barrier);
                }
                // Wait on split barriers before batch execution.
                if (!impl.info.use_split_barriers)
                {
                    for (auto barrier_index : task_batch.wait_split_barrier_indices)
                    {
                        TaskSplitBarrier const & split_barrier = permutation.split_barriers[barrier_index];
                        // Convert split barrier to normal barrier.
                        TaskBarrier barrier = split_barrier;
                        insert_pipeline_barrier(impl, permutation, impl_runtime.command_lists.back(), barrier);
                    }
                }
                else
                {
                    usize needed_image_barriers = 0;
                    for (auto barrier_index : task_batch.wait_split_barrier_indices)
                    {
                        TaskSplitBarrier const & split_barrier = permutation.split_barriers[barrier_index];
                        if (!split_barrier.image_id.is_empty())
                        {
                            needed_image_barriers += impl.get_actual_images(split_barrier.image_id, permutation).size();
                        }
                    }
                    tl_split_barrier_wait_infos.reserve(task_batch.wait_split_barrier_indices.size());
                    tl_memory_barrier_infos.reserve(task_batch.wait_split_barrier_indices.size());
                    tl_image_barrier_infos.reserve(needed_image_barriers);
                    for (auto barrier_index : task_batch.wait_split_barrier_indices)
                    {
                        TaskSplitBarrier & split_barrier = permutation.split_barriers[barrier_index];
                        if (split_barrier.image_id.is_empty())
                        {
                            tl_memory_barrier_infos.push_back(MemoryBarrierInfo{
                                .src_access = split_barrier.src_access,
                                .dst_access = split_barrier.dst_access,
                            });
                            tl_split_barrier_wait_infos.push_back(SplitBarrierWaitInfo{
                                .memory_barriers = std::span{&tl_memory_barrier_infos.back(), 1},
                                .split_barrier = split_barrier.split_barrier_state,
                            });
                        }
                        else
                        {
                            usize const img_bar_vec_start_size = tl_image_barrier_infos.size();
                            for (auto image : impl.get_actual_images(split_barrier.image_id, permutation))
                            {
                                tl_image_barrier_infos.push_back(ImageBarrierInfo{
                                    .src_access = split_barrier.src_access,
                                    .dst_access = split_barrier.dst_access,
                                    .src_layout = split_barrier.layout_before,
                                    .dst_layout = split_barrier.layout_after,
                                    .image_slice = split_barrier.slice,
                                    .image_id = image,
                                });
                            }
                            usize const img_bar_vec_end_size = tl_image_barrier_infos.size();
                            usize const img_bar_count = img_bar_vec_end_size - img_bar_vec_start_size;
                            tl_split_barrier_wait_infos.push_back(SplitBarrierWaitInfo{
                                .image_barriers = std::span{tl_image_barrier_infos.data() + img_bar_vec_start_size, img_bar_count},
                                .split_barrier = split_barrier.split_barrier_state,
                            });
                        }
                    }
                    if (!tl_split_barrier_wait_infos.empty())
                    {
                        impl_runtime.command_lists.back().wait_split_barriers(tl_split_barrier_wait_infos);
                    }
                    tl_split_barrier_wait_infos.clear();
                    tl_image_barrier_infos.clear();
                    tl_memory_barrier_infos.clear();
                }
                // Execute all tasks in the batch.
                usize task_index = 0;
                for (TaskId const task_id : task_batch.tasks)
                {
                    impl.execute_task(impl_runtime, permutation, task_index, task_id);
                    task_index += 1;
                }
                if (impl.info.use_split_barriers)
                {
                    // Reset all waited upon split barriers here.
                    for (auto barrier_index : task_batch.wait_split_barrier_indices)
                    {
                        // We wait on the stages, that waited on our split barrier earlier.
                        // This way, we make sure, that the stages that wait on the split barrier
                        // executed and saw the split barrier signaled, before we reset them.
                        impl_runtime.command_lists.back().reset_split_barrier({
                            .barrier = permutation.split_barriers[barrier_index].split_barrier_state,
                            .stage_masks = permutation.split_barriers[barrier_index].dst_access.stages,
                        });
                    }
                    // Signal all signal split barriers after batch execution.
                    for (usize const barrier_index : task_batch.signal_split_barrier_indices)
                    {
                        TaskSplitBarrier & task_split_barrier = permutation.split_barriers[barrier_index];
                        if (task_split_barrier.image_id.is_empty())
                        {
                            MemoryBarrierInfo memory_barrier{
                                .src_access = task_split_barrier.src_access,
                                .dst_access = task_split_barrier.dst_access,
                            };
                            impl_runtime.command_lists.back().signal_split_barrier({
                                .memory_barriers = std::span{&memory_barrier, 1},
                                .split_barrier = task_split_barrier.split_barrier_state,
                            });
                        }
                        else
                        {
                            for (auto image : impl.get_actual_images(task_split_barrier.image_id, permutation))
                            {
                                tl_image_barrier_infos.push_back({
                                    .src_access = task_split_barrier.src_access,
                                    .dst_access = task_split_barrier.dst_access,
                                    .src_layout = task_split_barrier.layout_before,
                                    .dst_layout = task_split_barrier.layout_after,
                                    .image_slice = task_split_barrier.slice,
                                    .image_id = image,
                                });
                            }
                            impl_runtime.command_lists.back().signal_split_barrier({
                                .image_barriers = tl_image_barrier_infos,
                                .split_barrier = task_split_barrier.split_barrier_state,
                            });
                            tl_image_barrier_infos.clear();
                        }
                    }
                }
                if (impl.info.enable_command_labels)
                {
                    impl_runtime.command_lists.back().end_label();
                }
            }
            for (usize const barrier_index : submit_scope.last_minute_barrier_indices)
            {
                TaskBarrier & barrier = permutation.barriers[barrier_index];
                insert_pipeline_barrier(impl, permutation, impl_runtime.command_lists.back(), barrier);
            }
            if (impl.info.enable_command_labels)
            {
                impl_runtime.command_lists.back().end_label();
            }
            for (auto & command_list : impl_runtime.command_lists)
            {
                DAXA_DBG_ASSERT_TRUE_M(
                    !command_list.is_complete(),
                    "it is illegal to complete command lists in tasks that are obtained by the runtime!");
                command_list.complete();
            }

            if (&submit_scope != &permutation.batch_submit_scopes.back())
            {
                auto submit_info = submit_scope.submit_info;
                submit_info.command_lists.insert(submit_info.command_lists.end(), impl_runtime.command_lists.begin(), impl_runtime.command_lists.end());
                if (impl.info.swapchain.has_value())
                {
                    Swapchain const & swapchain = impl.info.swapchain.value();
                    if (submit_scope_index == permutation.swapchain_image_first_use_submit_scope_index)
                    {
                        submit_info.wait_binary_semaphores.push_back(swapchain.get_acquire_semaphore());
                    }
                    if (submit_scope_index == permutation.swapchain_image_last_use_submit_scope_index)
                    {
                        submit_info.signal_binary_semaphores.push_back(swapchain.get_present_semaphore());
                        submit_info.signal_timeline_semaphores.emplace_back(
                            swapchain.get_gpu_timeline_semaphore(),
                            swapchain.get_cpu_timeline_value());
                    }
                }
                if (submit_scope.user_submit_info.additional_command_lists != nullptr)
                {
                    submit_info.command_lists.insert(submit_info.command_lists.end(), submit_scope.user_submit_info.additional_command_lists->begin(), submit_scope.user_submit_info.additional_command_lists->end());
                }
                if (submit_scope.user_submit_info.additional_wait_binary_semaphores != nullptr)
                {
                    submit_info.wait_binary_semaphores.insert(submit_info.wait_binary_semaphores.end(), submit_scope.user_submit_info.additional_wait_binary_semaphores->begin(), submit_scope.user_submit_info.additional_wait_binary_semaphores->end());
                }
                if (submit_scope.user_submit_info.additional_signal_binary_semaphores != nullptr)
                {
                    submit_info.signal_binary_semaphores.insert(submit_info.signal_binary_semaphores.end(), submit_scope.user_submit_info.additional_signal_binary_semaphores->begin(), submit_scope.user_submit_info.additional_signal_binary_semaphores->end());
                }
                if (submit_scope.user_submit_info.additional_wait_timeline_semaphores != nullptr)
                {
                    submit_info.wait_timeline_semaphores.insert(submit_info.wait_timeline_semaphores.end(), submit_scope.user_submit_info.additional_wait_timeline_semaphores->begin(), submit_scope.user_submit_info.additional_wait_timeline_semaphores->end());
                }
                if (submit_scope.user_submit_info.additional_signal_timeline_semaphores != nullptr)
                {
                    submit_info.signal_timeline_semaphores.insert(submit_info.signal_timeline_semaphores.end(), submit_scope.user_submit_info.additional_signal_timeline_semaphores->begin(), submit_scope.user_submit_info.additional_signal_timeline_semaphores->end());
                }
                if (impl.staging_memory.timeline_value() > impl.last_execution_staging_timeline_value)
                {
                    submit_info.signal_timeline_semaphores.push_back({impl.staging_memory.get_timeline_semaphore(), impl.staging_memory.timeline_value()});
                }
                impl.info.device.submit_commands(submit_info);

                if (submit_scope.present_info.has_value())
                {
                    ImplPresentInfo & impl_present_info = submit_scope.present_info.value();
                    std::vector<BinarySemaphore> present_wait_semaphores = impl_present_info.binary_semaphores;
                    present_wait_semaphores.push_back(impl.info.swapchain.value().get_present_semaphore());
                    if (impl_present_info.additional_binary_semaphores != nullptr)
                    {
                        present_wait_semaphores.insert(
                            present_wait_semaphores.end(),
                            impl_present_info.additional_binary_semaphores->begin(),
                            impl_present_info.additional_binary_semaphores->end());
                    }
                    impl.info.device.present_frame(PresentInfo{
                        .wait_binary_semaphores = present_wait_semaphores,
                        .swapchain = impl.info.swapchain.value(),
                    });
                }
                // We need to clear all completed command lists that have been submitted.
                impl_runtime.command_lists.clear();
                impl_runtime.command_lists.push_back(impl.info.device.create_command_list({.name = std::string("Task Command List ") + std::to_string(impl_runtime.command_lists.size())}));
            }
            ++submit_scope_index;
        }

        // Insert pervious uses into execution info for tje next executions synch.
        for (usize task_buffer_index = 0; task_buffer_index < permutation.buffer_infos.size(); ++task_buffer_index)
        {
            bool const is_persistent = std::holds_alternative<PermIndepTaskBufferInfo::Persistent>(impl.global_buffer_infos[task_buffer_index].task_buffer_data);
            if (permutation.buffer_infos[task_buffer_index].valid && is_persistent)
            {
                std::get<PermIndepTaskBufferInfo::Persistent>(impl.global_buffer_infos[task_buffer_index].task_buffer_data).get().latest_access = permutation.buffer_infos[task_buffer_index].latest_access;
            }
        }
        for (usize task_image_index = 0; task_image_index < permutation.image_infos.size(); ++task_image_index)
        {
            if (
                permutation.image_infos[task_image_index].valid &&
                impl.global_image_infos[task_image_index].is_persistent())
            {
                auto & persistent_image = impl.global_image_infos[task_image_index].get_persistent();
                for (auto const & extended_state : permutation.image_infos[task_image_index].last_slice_states)
                {
                    persistent_image.latest_slice_states.push_back(extended_state.state);
                }
            }
        }

        impl.left_over_command_lists = std::move(impl_runtime.command_lists);
        impl.executed_once = true;
        impl.prev_frame_permutation_index = permutation_index;
        impl.last_execution_staging_timeline_value = impl.staging_memory.timeline_value();

        if (impl.info.record_debug_information)
        {
            impl.debug_print();
        }
    }

    ImplTaskList::ImplTaskList(TaskListInfo a_info)
        : unique_index{ImplTaskList::exec_unique_next_index++}, info{std::move(a_info)}
    {
    }

    ImplTaskList::~ImplTaskList()
    {
        for (auto & permutation : permutations)
        {
            // because transient buffers are owned by tasklist we need to destroy them
            for (u32 buffer_info_idx = 0; buffer_info_idx < static_cast<u32>(global_buffer_infos.size()); buffer_info_idx++)
            {
                auto const & global_buffer = global_buffer_infos.at(buffer_info_idx);
                auto const & perm_buffer = permutation.buffer_infos.at(buffer_info_idx);
                if (!global_buffer.is_persistent() && perm_buffer.valid)
                {
                    info.device.destroy_buffer(get_actual_buffers(TaskBufferHandle{{.task_list_index = unique_index, .index = buffer_info_idx}}, permutation)[0]);
                }
            }
            // because transient images are owned by tasklist we need to destroy them
            for (u32 image_info_idx = 0; image_info_idx < static_cast<u32>(global_image_infos.size()); image_info_idx++)
            {
                auto const & global_image = global_image_infos.at(image_info_idx);
                auto const & perm_image = permutation.image_infos.at(image_info_idx);
                if (!global_image.is_persistent() && perm_image.valid)
                {
                    info.device.destroy_image(get_actual_images(TaskImageHandle{{.task_list_index = unique_index, .index = image_info_idx}}, permutation)[0]);
                }
            }
        }
        for (auto & task : tasks)
        {
            for (auto & view_cache : task.image_view_cache)
            {
                for (auto & view : view_cache)
                {
                    if (info.device.is_id_valid(view))
                    {
                        ImageId const parent = info.device.info_image_view(view).image;
                        bool const is_default_view = parent.default_view() == view;
                        if (!is_default_view)
                        {
                            info.device.destroy_image_view(view);
                        }
                    }
                }
            }
        }
    }

    void ImplTaskList::print_task_image_to(std::string & out, std::string indent, TaskListPermutation const & permutation, TaskImageHandle local_id)
    {
        auto const & glob_image = global_image_infos[local_id.index];
        std::string persistent_info = "";
        if (global_image_infos[local_id.index].is_persistent())
        {
            u32 const persistent_index = global_image_infos[local_id.index].get_persistent().unique_index;
            persistent_info = fmt::format(", persistent index: {}", persistent_index);
        }
        fmt::format_to(std::back_inserter(out), "{}task image name: \"{}\", id: ({}){}\n", indent, glob_image.get_name(), to_string(local_id), persistent_info);
        fmt::format_to(std::back_inserter(out), "{}runtime images:\n", indent);
        {
            [[maybe_unused]] FormatIndent d1{out, indent, true};
            for (u32 child_i = 0; child_i < get_actual_images(local_id, permutation).size(); ++child_i)
            {
                auto const child_id = get_actual_images(local_id, permutation)[child_i];
                auto const & child_info = info.device.info_image(child_id);
                fmt::format_to(std::back_inserter(out), "{}name: \"{}\", id: ({})\n", indent, child_info.name, to_string(child_id));
            }
            print_seperator_to(out, indent);
        }
    }

    void ImplTaskList::print_task_buffer_to(std::string & out, std::string indent, TaskListPermutation const & permutation, TaskBufferHandle local_id)
    {
        auto const & glob_buffer = global_buffer_infos[local_id.index];
        std::string persistent_info = "";
        if (global_buffer_infos[local_id.index].is_persistent())
        {
            u32 const persistent_index = global_buffer_infos[local_id.index].get_persistent().unique_index;
            persistent_info = fmt::format(", persistent index: {}", persistent_index);
        }
        fmt::format_to(std::back_inserter(out), "{}task buffer name: \"{}\", id: ({}){}\n", indent, glob_buffer.get_name(), to_string(local_id), persistent_info);
        fmt::format_to(std::back_inserter(out), "{}runtime buffers:\n", indent);
        {
            [[maybe_unused]] FormatIndent d2{out, indent, true};
            for (u32 child_i = 0; child_i < get_actual_buffers(local_id, permutation).size(); ++child_i)
            {
                auto const child_id = get_actual_buffers(local_id, permutation)[child_i];
                auto const & child_info = info.device.info_buffer(child_id);
                fmt::format_to(std::back_inserter(out), "{}name: \"{}\", id: ({})\n", indent, child_info.name, to_string(child_id));
            }
            print_seperator_to(out, indent);
        }
    }

    void ImplTaskList::print_task_barrier_to(std::string & out, std::string & indent, TaskListPermutation const & permutation, usize index, bool const split_barrier)
    {
        TaskBarrier const & barrier = split_barrier ? permutation.split_barriers[index] : permutation.barriers[index];
        if (barrier.image_id.is_empty())
        {
            MemoryBarrierInfo mem_barrier{
                .src_access = barrier.src_access,
                .dst_access = barrier.dst_access,
            };
            out.append(indent).append(to_string(mem_barrier)).push_back('\n');
        }
        else
        {
            fmt::format_to(std::back_inserter(out), "{}slice: ({})\n", indent, to_string(barrier.slice));
            fmt::format_to(std::back_inserter(out), "{}{}\n", indent, to_string(MemoryBarrierInfo{.src_access = barrier.src_access, .dst_access = barrier.dst_access}));
            fmt::format_to(std::back_inserter(out), "{}layout: ({}) -> ({})\n", indent, to_string(barrier.layout_before), to_string(barrier.layout_after));
            print_task_image_to(out, indent, permutation, barrier.image_id);
        }
    }

    void ImplTaskList::print_task_to(std::string & out, std::string & indent, TaskListPermutation const & permutation, usize task_id)
    {
        ImplTask const & task = tasks[task_id];
        fmt::format_to(std::back_inserter(out), "{}task name: \"{}\", id: {}\n", indent, task.base_task->get_name(), task_id);
        fmt::format_to(std::back_inserter(out), "{}task arguments:\n", indent);
        [[maybe_unused]] FormatIndent d0{out, indent, true};
        for_each(
            task.base_task->get_generic_uses(),
            [&](u32, TaskBufferUse<> const & buf)
            {
                auto access = task_buffer_access_to_access(buf.m_access);
                fmt::format_to(std::back_inserter(out), "{}buffer argument:\n", indent);
                fmt::format_to(std::back_inserter(out), "{}access: ({})\n", indent, to_string(access));
                print_task_buffer_to(out, indent, permutation, buf.handle);
                print_seperator_to(out, indent);
            },
            [&](u32, TaskImageUse<> const & img)
            {
                auto [layout, access] = task_image_access_to_layout_access(img.m_access);
                fmt::format_to(std::back_inserter(out), "{}image argument:\n", indent);
                fmt::format_to(std::back_inserter(out), "{}access: ({})\n", indent, to_string(access));
                fmt::format_to(std::back_inserter(out), "{}layout: {}\n", indent, to_string(layout));
                fmt::format_to(std::back_inserter(out), "{}slice: {}\n", indent, to_string(img.handle.slice));
                print_task_image_to(out, indent, permutation, img.handle);
                print_seperator_to(out, indent);
            });
    }

    void ImplTaskList::print_permutation_aliasing_to(std::string & out, std::string indent, TaskListPermutation const & permutation)
    {
        usize batches = 0;
        std::vector<usize> submit_batch_offsets(permutation.batch_submit_scopes.size());
        for (u32 submit_scope_idx = 0; submit_scope_idx < permutation.batch_submit_scopes.size(); submit_scope_idx++)
        {
            submit_batch_offsets.at(submit_scope_idx) = batches;
            batches += permutation.batch_submit_scopes.at(submit_scope_idx).task_batches.size();
        }
        auto print_lifetime = [&](usize start_idx, usize end_idx)
        {
            for (usize i = 0; i < batches; i++)
            {
                if (i >= start_idx && i < end_idx)
                {
                    fmt::format_to(std::back_inserter(out), "{}===", i);
                }
                else if (i == end_idx && end_idx != batches - 1)
                {
                    fmt::format_to(std::back_inserter(out), "{}---", i);
                }
                else if (i != batches - 1)
                {
                    fmt::format_to(std::back_inserter(out), "----", i);
                }
                else
                {
                    if (end_idx == batches - 1)
                    {
                        fmt::format_to(std::back_inserter(out), "{}", i);
                    }
                    else
                    {
                        fmt::format_to(std::back_inserter(out), "-");
                    }
                }
            }
        };
        fmt::format_to(std::back_inserter(out), "{}Resource lifetimes and aliasing:\n", indent);
        for (u32 perm_image_idx = 0; perm_image_idx < permutation.image_infos.size(); perm_image_idx++)
        {
            if (global_image_infos.at(perm_image_idx).is_persistent() || !permutation.image_infos.at(perm_image_idx).valid)
            {
                continue;
            }

            auto const & perm_task_image = permutation.image_infos.at(perm_image_idx);
            usize start_idx = submit_batch_offsets.at(perm_task_image.lifetime.first_use.submit_scope_index) +
                              perm_task_image.lifetime.first_use.task_batch_index;
            usize end_idx = submit_batch_offsets.at(perm_task_image.lifetime.last_use.submit_scope_index) +
                            perm_task_image.lifetime.last_use.task_batch_index;
            fmt::format_to(std::back_inserter(out), "{}", indent);
            print_lifetime(start_idx, end_idx);
            fmt::format_to(std::back_inserter(out), "  allocation offset: {} allocation size: {} task resource name: {}\n",
                           perm_task_image.allocation_offset,
                           std::get<PermIndepTaskImageInfo::Transient>(global_image_infos.at(perm_image_idx).task_image_data).memory_requirements.size,
                           global_image_infos.at(perm_image_idx).get_name());
        }
        for (u32 perm_buffer_idx = 0; perm_buffer_idx < permutation.buffer_infos.size(); perm_buffer_idx++)
        {
            if (global_buffer_infos.at(perm_buffer_idx).is_persistent())
            {
                continue;
            }

            auto const & perm_task_buffer = permutation.buffer_infos.at(perm_buffer_idx);
            usize start_idx = submit_batch_offsets.at(perm_task_buffer.lifetime.first_use.submit_scope_index) +
                              perm_task_buffer.lifetime.first_use.task_batch_index;
            usize end_idx = submit_batch_offsets.at(perm_task_buffer.lifetime.last_use.submit_scope_index) +
                            perm_task_buffer.lifetime.last_use.task_batch_index;
            fmt::format_to(std::back_inserter(out), "{}", indent);
            print_lifetime(start_idx, end_idx);
            fmt::format_to(std::back_inserter(out), "  allocation offset: {} allocation size: {} task resource name: {}\n",
                           perm_task_buffer.allocation_offset,
                           std::get<PermIndepTaskBufferInfo::Transient>(global_buffer_infos.at(perm_buffer_idx).task_buffer_data).memory_requirements.size,
                           global_buffer_infos.at(perm_buffer_idx).get_name());
        }
    }

    void ImplTaskList::debug_print()
    {
        std::string out = {};
        std::string indent = {};
        fmt::format_to(std::back_inserter(out), "task list name: {}, id: {}:\n", info.name, unique_index);
        fmt::format_to(std::back_inserter(out), "device: {}\n", info.device.info().name);
        fmt::format_to(std::back_inserter(out), "swapchain: {}\n", (this->info.swapchain.has_value() ? this->info.swapchain.value().info().name : "-"));
        fmt::format_to(std::back_inserter(out), "reorder tasks: {}\n", info.reorder_tasks);
        fmt::format_to(std::back_inserter(out), "use split barriers: {}\n", info.use_split_barriers);
        fmt::format_to(std::back_inserter(out), "permutation_condition_count: {}\n", info.permutation_condition_count);
        fmt::format_to(std::back_inserter(out), "enable_command_labels: {}\n", info.enable_command_labels);
        fmt::format_to(std::back_inserter(out), "task_list_label_color: ({},{},{},{})\n",
                       info.task_list_label_color[0],
                       info.task_list_label_color[1],
                       info.task_list_label_color[2],
                       info.task_list_label_color[3]);
        fmt::format_to(std::back_inserter(out), "task_batch_label_color: ({},{},{},{})\n",
                       info.task_batch_label_color[0],
                       info.task_batch_label_color[1],
                       info.task_batch_label_color[2],
                       info.task_batch_label_color[3]);
        fmt::format_to(std::back_inserter(out), "task_label_color: ({},{},{},{})\n",
                       info.task_label_color[0],
                       info.task_label_color[1],
                       info.task_label_color[2],
                       info.task_label_color[3]);
        fmt::format_to(std::back_inserter(out), "record_debug_information: {}\n", info.record_debug_information);
        fmt::format_to(std::back_inserter(out), "staging_memory_pool_size: {}\n", info.staging_memory_pool_size);
        fmt::format_to(std::back_inserter(out), "executed permutation: {}\n", chosen_permutation_last_execution);
        usize permutation_index = this->chosen_permutation_last_execution;
        auto & permutation = this->permutations[permutation_index];
        {
            this->print_permutation_aliasing_to(out, indent, permutation);
            permutation_index += 1;
            fmt::format_to(std::back_inserter(out), "permutations split barriers: {}\n", info.use_split_barriers);
            [[maybe_unused]] FormatIndent d0{out, indent, true};
            usize submit_scope_index = 0;
            for (auto & submit_scope : permutation.batch_submit_scopes)
            {
                fmt::format_to(std::back_inserter(out), "{}submit scope: {}\n", indent, submit_scope_index);
                [[maybe_unused]] FormatIndent d1{out, indent, true};
                usize batch_index = 0;
                for (auto & task_batch : submit_scope.task_batches)
                {
                    fmt::format_to(std::back_inserter(out), "{}batch: {}\n", indent, batch_index);
                    batch_index += 1;
                    fmt::format_to(std::back_inserter(out), "{}inserted pipeline barriers:\n", indent);
                    {
                        [[maybe_unused]] FormatIndent d2{out, indent, true};
                        for (auto barrier_index : task_batch.pipeline_barrier_indices)
                        {
                            this->print_task_barrier_to(out, indent, permutation, barrier_index, false);
                            print_seperator_to(out, indent);
                        }
                    }
                    if (!this->info.use_split_barriers)
                    {
                        fmt::format_to(std::back_inserter(out), "{}inserted pipeline barriers (converted from split barrier):\n", indent);
                        [[maybe_unused]] FormatIndent d2{out, indent, true};
                        for (auto barrier_index : task_batch.wait_split_barrier_indices)
                        {
                            this->print_task_barrier_to(out, indent, permutation, barrier_index, true);
                            print_seperator_to(out, indent);
                        }
                    }
                    else
                    {
                        fmt::format_to(std::back_inserter(out), "{}inserted split pipeline barrier waits:\n", indent);
                        [[maybe_unused]] FormatIndent d2{out, indent, true};
                        print_seperator_to(out, indent);
                        for (auto barrier_index : task_batch.wait_split_barrier_indices)
                        {
                            this->print_task_barrier_to(out, indent, permutation, barrier_index, true);
                            print_seperator_to(out, indent);
                        }
                    }
                    fmt::format_to(std::back_inserter(out), "{}tasks:\n", indent);
                    {
                        [[maybe_unused]] FormatIndent d2{out, indent, true};
                        for (TaskId const task_id : task_batch.tasks)
                        {
                            this->print_task_to(out, indent, permutation, task_id);
                            print_seperator_to(out, indent);
                        }
                    }
                    if (this->info.use_split_barriers)
                    {
                        fmt::format_to(std::back_inserter(out), "{}inserted split barrier signals:\n", indent);
                        [[maybe_unused]] FormatIndent d2{out, indent, true};
                        for (usize const barrier_index : task_batch.signal_split_barrier_indices)
                        {
                            this->print_task_barrier_to(out, indent, permutation, barrier_index, true);
                            print_seperator_to(out, indent);
                        }
                    }
                    print_seperator_to(out, indent);
                }
                if (!submit_scope.last_minute_barrier_indices.empty())
                {
                    fmt::format_to(std::back_inserter(out), "{}inserted last minute pipeline barriers:\n", indent);
                    [[maybe_unused]] FormatIndent d2{out, indent, true};
                    for (usize const barrier_index : submit_scope.last_minute_barrier_indices)
                    {
                        this->print_task_barrier_to(out, indent, permutation, barrier_index, false);
                        print_seperator_to(out, indent);
                    }
                }
                if (&submit_scope != &permutation.batch_submit_scopes.back())
                {
                    fmt::format_to(std::back_inserter(out), "{} -- inserted submit -- \n", indent);
                    if (submit_scope.present_info.has_value())
                    {
                        fmt::format_to(std::back_inserter(out), "{} -- inserted present -- \n", indent);
                    }
                }
                print_seperator_to(out, indent);
            }
            print_seperator_to(out, indent);
        }
        this->debug_string_stream << out;
    }
} // namespace daxa

#endif
