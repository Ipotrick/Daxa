#if DAXA_BUILT_WITH_UTILS_TASK_LIST

#include "impl_task_list.hpp"
#include <iostream>

#include <fstream>
#include <utility>

namespace daxa
{
    auto task_image_access_to_layout_access(TaskImageAccess const & access) -> std::tuple<ImageLayout, Access>
    {
        switch (access)
        {
        case TaskImageAccess::NONE: return {ImageLayout::UNDEFINED, {PipelineStageFlagBits::NONE, AccessTypeFlagBits::NONE}};
        case TaskImageAccess::SHADER_READ_ONLY: return {ImageLayout::READ_ONLY_OPTIMAL, {PipelineStageFlagBits::ALL_GRAPHICS | PipelineStageFlagBits::COMPUTE_SHADER, AccessTypeFlagBits::READ}};
        case TaskImageAccess::VERTEX_SHADER_READ_ONLY: return {ImageLayout::READ_ONLY_OPTIMAL, {PipelineStageFlagBits::VERTEX_SHADER, AccessTypeFlagBits::READ}};
        case TaskImageAccess::TESSELLATION_CONTROL_SHADER_READ_ONLY: return {ImageLayout::READ_ONLY_OPTIMAL, {PipelineStageFlagBits::TESSELLATION_CONTROL_SHADER, AccessTypeFlagBits::READ}};
        case TaskImageAccess::TESSELLATION_EVALUATION_SHADER_READ_ONLY: return {ImageLayout::READ_ONLY_OPTIMAL, {PipelineStageFlagBits::TESSELLATION_EVALUATION_SHADER, AccessTypeFlagBits::READ}};
        case TaskImageAccess::GEOMETRY_SHADER_READ_ONLY: return {ImageLayout::READ_ONLY_OPTIMAL, {PipelineStageFlagBits::GEOMETRY_SHADER, AccessTypeFlagBits::READ}};
        case TaskImageAccess::FRAGMENT_SHADER_READ_ONLY: return {ImageLayout::READ_ONLY_OPTIMAL, {PipelineStageFlagBits::FRAGMENT_SHADER, AccessTypeFlagBits::READ}};
        case TaskImageAccess::COMPUTE_SHADER_READ_ONLY: return {ImageLayout::READ_ONLY_OPTIMAL, {PipelineStageFlagBits::COMPUTE_SHADER, AccessTypeFlagBits::READ}};
        case TaskImageAccess::SHADER_WRITE_ONLY: return {ImageLayout::GENERAL, {PipelineStageFlagBits::PipelineStageFlagBits::ALL_GRAPHICS | PipelineStageFlagBits::COMPUTE_SHADER, AccessTypeFlagBits::WRITE}};
        case TaskImageAccess::VERTEX_SHADER_WRITE_ONLY: return {ImageLayout::GENERAL, {PipelineStageFlagBits::VERTEX_SHADER, AccessTypeFlagBits::WRITE}};
        case TaskImageAccess::TESSELLATION_CONTROL_SHADER_WRITE_ONLY: return {ImageLayout::GENERAL, {PipelineStageFlagBits::TESSELLATION_CONTROL_SHADER, AccessTypeFlagBits::WRITE}};
        case TaskImageAccess::TESSELLATION_EVALUATION_SHADER_WRITE_ONLY: return {ImageLayout::GENERAL, {PipelineStageFlagBits::TESSELLATION_EVALUATION_SHADER, AccessTypeFlagBits::WRITE}};
        case TaskImageAccess::GEOMETRY_SHADER_WRITE_ONLY: return {ImageLayout::GENERAL, {PipelineStageFlagBits::GEOMETRY_SHADER, AccessTypeFlagBits::WRITE}};
        case TaskImageAccess::FRAGMENT_SHADER_WRITE_ONLY: return {ImageLayout::GENERAL, {PipelineStageFlagBits::FRAGMENT_SHADER, AccessTypeFlagBits::WRITE}};
        case TaskImageAccess::COMPUTE_SHADER_WRITE_ONLY: return {ImageLayout::GENERAL, {PipelineStageFlagBits::COMPUTE_SHADER, AccessTypeFlagBits::WRITE}};
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
        case TaskImageAccess::DEPTH_ATTACHMENT_READ_ONLY:
            [[fallthrough]];
        case TaskImageAccess::STENCIL_ATTACHMENT_READ_ONLY:
            [[fallthrough]];
        case TaskImageAccess::DEPTH_STENCIL_ATTACHMENT_READ_ONLY: return {ImageLayout::READ_ONLY_OPTIMAL, {PipelineStageFlagBits::EARLY_FRAGMENT_TESTS | PipelineStageFlagBits::LATE_FRAGMENT_TESTS, AccessTypeFlagBits::READ}};
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
        case TaskBufferAccess::SHADER_READ_ONLY: return {PipelineStageFlagBits::ALL_GRAPHICS | PipelineStageFlagBits::COMPUTE_SHADER, AccessTypeFlagBits::READ};
        case TaskBufferAccess::VERTEX_SHADER_READ_ONLY: return {PipelineStageFlagBits::VERTEX_SHADER, AccessTypeFlagBits::READ};
        case TaskBufferAccess::TESSELLATION_CONTROL_SHADER_READ_ONLY: return {PipelineStageFlagBits::TESSELLATION_CONTROL_SHADER, AccessTypeFlagBits::READ};
        case TaskBufferAccess::TESSELLATION_EVALUATION_SHADER_READ_ONLY: return {PipelineStageFlagBits::TESSELLATION_EVALUATION_SHADER, AccessTypeFlagBits::READ};
        case TaskBufferAccess::GEOMETRY_SHADER_READ_ONLY: return {PipelineStageFlagBits::GEOMETRY_SHADER, AccessTypeFlagBits::READ};
        case TaskBufferAccess::FRAGMENT_SHADER_READ_ONLY: return {PipelineStageFlagBits::FRAGMENT_SHADER, AccessTypeFlagBits::READ};
        case TaskBufferAccess::COMPUTE_SHADER_READ_ONLY: return {PipelineStageFlagBits::COMPUTE_SHADER, AccessTypeFlagBits::READ};
        case TaskBufferAccess::SHADER_WRITE_ONLY: return {PipelineStageFlagBits::PipelineStageFlagBits::ALL_GRAPHICS | PipelineStageFlagBits::COMPUTE_SHADER, AccessTypeFlagBits::WRITE};
        case TaskBufferAccess::VERTEX_SHADER_WRITE_ONLY: return {PipelineStageFlagBits::VERTEX_SHADER, AccessTypeFlagBits::WRITE};
        case TaskBufferAccess::TESSELLATION_CONTROL_SHADER_WRITE_ONLY: return {PipelineStageFlagBits::TESSELLATION_CONTROL_SHADER, AccessTypeFlagBits::WRITE};
        case TaskBufferAccess::TESSELLATION_EVALUATION_SHADER_WRITE_ONLY: return {PipelineStageFlagBits::TESSELLATION_EVALUATION_SHADER, AccessTypeFlagBits::WRITE};
        case TaskBufferAccess::GEOMETRY_SHADER_WRITE_ONLY: return {PipelineStageFlagBits::GEOMETRY_SHADER, AccessTypeFlagBits::WRITE};
        case TaskBufferAccess::FRAGMENT_SHADER_WRITE_ONLY: return {PipelineStageFlagBits::FRAGMENT_SHADER, AccessTypeFlagBits::WRITE};
        case TaskBufferAccess::COMPUTE_SHADER_WRITE_ONLY: return {PipelineStageFlagBits::COMPUTE_SHADER, AccessTypeFlagBits::WRITE};
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
        default: DAXA_DBG_ASSERT_TRUE_M(false, "unreachable");
        }
        return {};
    }

    auto TaskGPUResourceId::is_empty() const -> bool
    {
        return index == std::numeric_limits<u32>::max();
    }

    auto to_string(TaskBufferAccess const & usage) -> std::string_view
    {
        switch (usage)
        {
        case TaskBufferAccess::SHADER_READ_ONLY: return std::string_view{"SHADER_READ_ONLY"};
        case TaskBufferAccess::VERTEX_SHADER_READ_ONLY: return std::string_view{"VERTEX_SHADER_READ_ONLY"};
        case TaskBufferAccess::TESSELLATION_CONTROL_SHADER_READ_ONLY: return std::string_view{"TESSELLATION_CONTROL_SHADER_READ_ONLY"};
        case TaskBufferAccess::TESSELLATION_EVALUATION_SHADER_READ_ONLY: return std::string_view{"TESSELLATION_EVALUATION_SHADER_READ_ONLY"};
        case TaskBufferAccess::GEOMETRY_SHADER_READ_ONLY: return std::string_view{"GEOMETRY_SHADER_READ_ONLY"};
        case TaskBufferAccess::FRAGMENT_SHADER_READ_ONLY: return std::string_view{"FRAGMENT_SHADER_READ_ONLY"};
        case TaskBufferAccess::COMPUTE_SHADER_READ_ONLY: return std::string_view{"COMPUTE_SHADER_READ_ONLY"};
        case TaskBufferAccess::SHADER_WRITE_ONLY: return std::string_view{"SHADER_WRITE_ONLY"};
        case TaskBufferAccess::VERTEX_SHADER_WRITE_ONLY: return std::string_view{"VERTEX_SHADER_WRITE_ONLY"};
        case TaskBufferAccess::TESSELLATION_CONTROL_SHADER_WRITE_ONLY: return std::string_view{"TESSELLATION_CONTROL_SHADER_WRITE_ONLY"};
        case TaskBufferAccess::TESSELLATION_EVALUATION_SHADER_WRITE_ONLY: return std::string_view{"TESSELLATION_EVALUATION_SHADER_WRITE_ONLY"};
        case TaskBufferAccess::GEOMETRY_SHADER_WRITE_ONLY: return std::string_view{"GEOMETRY_SHADER_WRITE_ONLY"};
        case TaskBufferAccess::FRAGMENT_SHADER_WRITE_ONLY: return std::string_view{"FRAGMENT_SHADER_WRITE_ONLY"};
        case TaskBufferAccess::COMPUTE_SHADER_WRITE_ONLY: return std::string_view{"COMPUTE_SHADER_WRITE_ONLY"};
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
        default: DAXA_DBG_ASSERT_TRUE_M(false, "unreachable");
        }
        return "invalid";
    }

    auto to_string(TaskImageAccess const & usage) -> std::string_view
    {
        switch (usage)
        {
        case TaskImageAccess::SHADER_READ_ONLY: return std::string_view{"SHADER_READ_ONLY"};
        case TaskImageAccess::VERTEX_SHADER_READ_ONLY: return std::string_view{"VERTEX_SHADER_READ_ONLY"};
        case TaskImageAccess::TESSELLATION_CONTROL_SHADER_READ_ONLY: return std::string_view{"TESSELLATION_CONTROL_SHADER_READ_ONLY"};
        case TaskImageAccess::TESSELLATION_EVALUATION_SHADER_READ_ONLY: return std::string_view{"TESSELLATION_EVALUATION_SHADER_READ_ONLY"};
        case TaskImageAccess::GEOMETRY_SHADER_READ_ONLY: return std::string_view{"GEOMETRY_SHADER_READ_ONLY"};
        case TaskImageAccess::FRAGMENT_SHADER_READ_ONLY: return std::string_view{"FRAGMENT_SHADER_READ_ONLY"};
        case TaskImageAccess::COMPUTE_SHADER_READ_ONLY: return std::string_view{"COMPUTE_SHADER_READ_ONLY"};
        case TaskImageAccess::SHADER_WRITE_ONLY: return std::string_view{"SHADER_WRITE_ONLY"};
        case TaskImageAccess::VERTEX_SHADER_WRITE_ONLY: return std::string_view{"VERTEX_SHADER_WRITE_ONLY"};
        case TaskImageAccess::TESSELLATION_CONTROL_SHADER_WRITE_ONLY: return std::string_view{"TESSELLATION_CONTROL_SHADER_WRITE_ONLY"};
        case TaskImageAccess::TESSELLATION_EVALUATION_SHADER_WRITE_ONLY: return std::string_view{"TESSELLATION_EVALUATION_SHADER_WRITE_ONLY"};
        case TaskImageAccess::GEOMETRY_SHADER_WRITE_ONLY: return std::string_view{"GEOMETRY_SHADER_WRITE_ONLY"};
        case TaskImageAccess::FRAGMENT_SHADER_WRITE_ONLY: return std::string_view{"FRAGMENT_SHADER_WRITE_ONLY"};
        case TaskImageAccess::COMPUTE_SHADER_WRITE_ONLY: return std::string_view{"COMPUTE_SHADER_WRITE_ONLY"};
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
        case TaskImageAccess::DEPTH_ATTACHMENT_READ_ONLY: return std::string_view{"DEPTH_ATTACHMENT_READ_ONLY"};
        case TaskImageAccess::STENCIL_ATTACHMENT_READ_ONLY: return std::string_view{"STENCIL_ATTACHMENT_READ_ONLY"};
        case TaskImageAccess::DEPTH_STENCIL_ATTACHMENT_READ_ONLY: return std::string_view{"DEPTH_STENCIL_ATTACHMENT_READ_ONLY"};
        case TaskImageAccess::RESOLVE_WRITE: return std::string_view{"RESOLVE_WRITE"};
        case TaskImageAccess::PRESENT: return std::string_view{"PRESENT"};
        default: DAXA_DBG_ASSERT_TRUE_M(false, "unreachable");
        }
        return "invalid";
    }

    TaskRuntime::TaskRuntime(void * a_backend)
        : backend{a_backend}
    {
    }

    auto TaskRuntime::get_device() const -> Device &
    {
        auto & impl = *static_cast<ImplTaskRuntime *>(this->backend);
        return impl.task_list.info.device;
    }

    auto TaskRuntime::get_command_list() const -> CommandList
    {
        auto & impl = *static_cast<ImplTaskRuntime *>(this->backend);
        if (impl.reuse_last_command_list)
        {
            impl.reuse_last_command_list = false;
            return impl.command_lists.back();
        }
        else
        {
            impl.command_lists.push_back({get_device().create_command_list({.debug_name = std::string("Task Command List ") + std::to_string(impl.command_lists.size())})});
            return impl.command_lists.back();
        }
    }

    auto TaskRuntime::get_used_task_buffers() const -> UsedTaskBuffers const &
    {
        auto & impl = *static_cast<ImplTaskRuntime *>(this->backend);
        return impl.current_task->info.used_buffers;
    }

    auto TaskRuntime::get_used_task_images() const -> UsedTaskImages const &
    {
        auto & impl = *static_cast<ImplTaskRuntime *>(this->backend);
        return impl.current_task->info.used_images;
    }

    auto TaskRuntime::get_buffers(TaskBufferId const & task_resource_id) const -> std::span<BufferId>
    {
        auto & impl = *static_cast<ImplTaskRuntime *>(this->backend);
        return impl.task_list.impl_task_buffers[task_resource_id.index].buffers;
    }

    auto TaskRuntime::get_images(TaskImageId const & task_resource_id) const -> std::span<ImageId>
    {
        auto & impl = *static_cast<ImplTaskRuntime *>(this->backend);
        return impl.task_list.impl_task_images[task_resource_id.index].images;
    }

    TaskList::TaskList(TaskListInfo const & info)
        : ManagedPtr{new ImplTaskList(info)}
    {
        auto & impl = *reinterpret_cast<ImplTaskList *>(this->object);
        impl.batch_submit_scopes.push_back({
            .present_info = {},
        });
    }

    TaskList::~TaskList() = default;

    auto TaskList::create_task_buffer(TaskBufferInfo const & info) -> TaskBufferId
    {
        auto & impl = *reinterpret_cast<ImplTaskList *>(this->object);
        DAXA_DBG_ASSERT_TRUE_M(!impl.compiled, "can not record to completed command list");
        TaskBufferId task_buffer_id{{.index = static_cast<u32>(impl.impl_task_buffers.size())}};
        impl.impl_task_buffers.push_back(ImplTaskBuffer{
            .info = info,
        });
        return task_buffer_id;
    }

    auto TaskList::create_task_image(TaskImageInfo const & info) -> TaskImageId
    {
        auto & impl = *reinterpret_cast<ImplTaskList *>(this->object);
        DAXA_DBG_ASSERT_TRUE_M(!impl.compiled, "can not record to completed command list");
        TaskImageId task_image_id{{.index = static_cast<u32>(impl.impl_task_images.size())}};
        impl.impl_task_images.push_back(ImplTaskImage{
            .info = info,
            .swapchain_semaphore_waited_upon = false,
            .slices_last_uses = {},
        });
        if (info.swapchain_image)
        {
            impl.swapchain_image = task_image_id;
        }
        return task_image_id;
    }

    void TaskList::add_runtime_buffer(TaskBufferId tid, BufferId id)
    {
        auto & impl = *reinterpret_cast<ImplTaskList *>(this->object);
        impl.add_runtime_buffer(tid, id);
    }
    void TaskList::add_runtime_image(TaskImageId tid, ImageId id)
    {
        auto & impl = *reinterpret_cast<ImplTaskList *>(this->object);
        impl.add_runtime_image(tid, id);
    }
    void TaskList::remove_runtime_buffer(TaskBufferId tid, BufferId id)
    {
        auto & impl = *reinterpret_cast<ImplTaskList *>(this->object);
        impl.remove_runtime_buffer(tid, id);
    }
    void TaskList::remove_runtime_image(TaskImageId tid, ImageId id)
    {
        auto & impl = *reinterpret_cast<ImplTaskList *>(this->object);
        impl.remove_runtime_image(tid, id);
    }
    void TaskList::clear_runtime_buffers(TaskBufferId tid)
    {
        auto & impl = *reinterpret_cast<ImplTaskList *>(this->object);
        impl.clear_runtime_buffers(tid);
    }
    void TaskList::clear_runtime_images(TaskImageId tid)
    {
        auto & impl = *reinterpret_cast<ImplTaskList *>(this->object);
        impl.clear_runtime_images(tid);
    }

    void TaskRuntime::add_runtime_buffer(TaskBufferId tid, BufferId id)
    {
        auto & impl = *static_cast<ImplTaskRuntime *>(this->backend);
        impl.task_list.add_runtime_buffer(tid, id);
    }
    void TaskRuntime::add_runtime_image(TaskImageId tid, ImageId id)
    {
        auto & impl = *static_cast<ImplTaskRuntime *>(this->backend);
        impl.task_list.add_runtime_image(tid, id);
    }
    void TaskRuntime::remove_runtime_buffer(TaskBufferId tid, BufferId id)
    {
        auto & impl = *static_cast<ImplTaskRuntime *>(this->backend);
        impl.task_list.remove_runtime_buffer(tid, id);
    }
    void TaskRuntime::remove_runtime_image(TaskImageId tid, ImageId id)
    {
        auto & impl = *static_cast<ImplTaskRuntime *>(this->backend);
        impl.task_list.remove_runtime_image(tid, id);
    }
    void TaskRuntime::clear_runtime_buffers(TaskBufferId tid)
    {
        auto & impl = *static_cast<ImplTaskRuntime *>(this->backend);
        impl.task_list.clear_runtime_buffers(tid);
    }
    void TaskRuntime::clear_runtime_images(TaskImageId tid)
    {
        auto & impl = *static_cast<ImplTaskRuntime *>(this->backend);
        impl.task_list.clear_runtime_images(tid);
    }

    void ImplTaskList::add_runtime_buffer(TaskBufferId tid, BufferId id)
    {
        impl_task_buffers.at(tid.index).buffers.push_back(id);
    }
    void ImplTaskList::add_runtime_image(TaskImageId tid, ImageId id)
    {
        impl_task_images.at(tid.index).images.push_back(id);
    }
    void ImplTaskList::remove_runtime_buffer(TaskBufferId tid, BufferId id)
    {
        impl_task_buffers[tid.index].buffers.erase(std::remove(impl_task_buffers[tid.index].buffers.begin(), impl_task_buffers[tid.index].buffers.end(), id), impl_task_buffers[tid.index].buffers.end());
    }
    void ImplTaskList::remove_runtime_image(TaskImageId tid, ImageId id)
    {
        impl_task_images[tid.index].images.erase(std::remove(impl_task_images[tid.index].images.begin(), impl_task_images[tid.index].images.end(), id), impl_task_images[tid.index].images.end());
    }
    void ImplTaskList::clear_runtime_buffers(TaskBufferId tid)
    {
        impl_task_buffers.at(tid.index).buffers.clear();
    }
    void ImplTaskList::clear_runtime_images(TaskImageId tid)
    {
        impl_task_images.at(tid.index).images.clear();
    }

    void check_for_overlapping_use(TaskInfo const & info)
    {
        for (usize current_i = 0; current_i < info.used_buffers.size(); ++current_i)
        {
            for (usize other_i = 0; other_i < info.used_buffers.size(); ++other_i)
            {
                if (current_i == other_i)
                {
                    continue;
                }
                DAXA_DBG_ASSERT_TRUE_M(
                    info.used_buffers[current_i].id.index != info.used_buffers[other_i].id.index,
                    "illegal to specify multiple uses for one buffer for one task. please combine all uses into one for each buffer.");
            }
        }
        for (usize current_i = 0; current_i < info.used_images.size(); ++current_i)
        {
            for (usize other_i = 0; other_i < info.used_images.size(); ++other_i)
            {
                if (current_i == other_i)
                {
                    continue;
                }
                auto const current_image_use = info.used_images[current_i];
                auto const other_image_use = info.used_images[other_i];
                // We only check for overlapping use of the same image.
                if (current_image_use.id != other_image_use.id)
                {
                    continue;
                }
                DAXA_DBG_ASSERT_TRUE_M(
                    !current_image_use.slice.intersects(other_image_use.slice),
                    "illegal to specify multiple uses for one image with overlapping slices for one task.");
            }
        }
    }

    auto find_first_possible_batch_index(
        ImplTaskList & impl,
        TaskBatchSubmitScope & current_submit_scope,
        usize const current_submit_scope_index,
        TaskInfo const & info) -> usize
    {
        usize first_possible_batch_index = 0;
        if (!impl.info.reorder_tasks)
        {
            first_possible_batch_index = current_submit_scope.task_batches.size() - 1;
        }

        for (auto const & [used_buffer_t_id, used_buffer_t_access] : info.used_buffers)
        {
            ImplTaskBuffer const & impl_task_buffer = impl.impl_task_buffers[used_buffer_t_id.index];
            // If the latest access is in a previous submit scope, the earliest batch we can insert into is
            // the current scopes first batch.
            if (impl_task_buffer.latest_access_submit_scope_index < current_submit_scope_index)
            {
                continue;
            }

            Access const current_buffer_access = task_buffer_access_to_access(used_buffer_t_access);
            // Every other access (NONE, READ_WRITE, WRITE) are interpreted as writes in this context.
            bool const is_last_access_read = impl_task_buffer.latest_access.type == AccessTypeFlagBits::READ;
            bool const is_current_access_read = current_buffer_access.type == AccessTypeFlagBits::READ;

            // When a buffer has been read in a previous use AND the current task also reads the buffer,
            // we must insert the task at or after the last use batch.
            usize current_buffer_first_possible_batch_index = impl_task_buffer.latest_access_batch_index;
            // So when not both, the last access and the current access, are reads, we need to insert AFTER the latest access.
            if (!(is_last_access_read && is_current_access_read))
            {
                current_buffer_first_possible_batch_index += 1;
            }
            first_possible_batch_index = std::max(first_possible_batch_index, current_buffer_first_possible_batch_index);
        }
        for (auto const & [used_image_t_id, used_image_t_access, used_image_slice] : info.used_images)
        {
            ImplTaskImage const & impl_task_image = impl.impl_task_images[used_image_t_id.index];
            DAXA_DBG_ASSERT_TRUE_M(!impl_task_image.swapchain_semaphore_waited_upon, "swapchain image is already presented!");
            if (impl_task_image.info.swapchain_image)
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

            auto [this_task_image_layout, this_task_image_access] = task_image_access_to_layout_access(used_image_t_access);
            // As image subresources can be in different layouts and also different synchronization scopes,
            // we need to track these image ranges individually.
            for (TaskImageTrackedSlice const & tracked_slice : impl_task_image.slices_last_uses)
            {
                // If the latest access is in a previous submit scope, the earliest batch we can insert into is
                // the current scopes first batch.
                // When the slices dont intersect, we dont need to do any sync or execution ordering between them.
                if (
                    tracked_slice.latest_access_submit_scope_index < current_submit_scope_index ||
                    !tracked_slice.slice.intersects(used_image_slice))
                {
                    continue;
                }
                // Now that we found out that the new use and an old use intersect,
                // we need to insert the task in the same or a later batch.
                bool const is_last_access_read = tracked_slice.latest_access.type == AccessTypeFlagBits::READ;
                bool const is_current_access_read = this_task_image_access.type == AccessTypeFlagBits::READ;
                // When the image layouts differ, we must do a layout transition between reads.
                // This forces us to place the task into a batch AFTER the tracked uses last batch.
                bool const is_layout_identical = this_task_image_layout == tracked_slice.latest_layout;
                usize current_image_first_possible_batch_index = tracked_slice.latest_access_batch_index;
                // If either the image layouts differ, or not both accesses are reads, we must place the task in a later batch.
                if (!(is_last_access_read && is_current_access_read && is_layout_identical))
                {
                    current_image_first_possible_batch_index += 1;
                }
                first_possible_batch_index = std::max(first_possible_batch_index, current_image_first_possible_batch_index);
            }
        }
        // Make sure we have enough batches.
        if (first_possible_batch_index >= current_submit_scope.task_batches.size())
        {
            current_submit_scope.task_batches.resize(first_possible_batch_index + 1);
        }
        return first_possible_batch_index;
    }

    thread_local std::vector<TaskImageTrackedSlice> tl_tracked_slice_rests = {};
    thread_local std::vector<ImageMipArraySlice> tl_new_use_slices = {};
    void TaskList::add_task(TaskInfo const & info)
    {
        auto & impl = *reinterpret_cast<ImplTaskList *>(this->object);
        DAXA_DBG_ASSERT_TRUE_M(!impl.compiled, "can not record to completed command list");

        TaskId const task_id = impl.tasks.size();
        impl.tasks.emplace_back(Task{
            .info = info,
        });

        usize const current_submit_scope_index = impl.batch_submit_scopes.size() - 1;
        TaskBatchSubmitScope & current_submit_scope = impl.batch_submit_scopes[current_submit_scope_index];

        // All tasks are reordered while recording.
        // Tasks are grouped into "task batches" which are just a group of tasks,
        // that can execute together while overlapping without synchronization between them.
        // Task batches are further grouped into submit scopes.
        // A submit scopes contains a group of batches between two submits.

#if DAXA_VALIDATION
        // Overlapping resource uses can be valid in the case of reads in the same layout for example.
        // But in order to make the task list implementation simpler,
        // daxa does not allow for overlapping use of a resource within a task, even when it is a read in the same layout.
        check_for_overlapping_use(info);
#endif // #if DAXA_VALIDATION

        // At first, we find the batch we need to insert the new task into.
        // To optimize for optimal overlap and minimal pipeline barriers, we try to insert the task as early as possible.
        const usize batch_index = find_first_possible_batch_index(
            impl,
            current_submit_scope,
            current_submit_scope_index,
            info);
        TaskBatch & batch = current_submit_scope.task_batches[batch_index];
        // Add the task to the batch.
        batch.tasks.push_back(task_id);

        // Now that we know what batch we need to insert the task into, we need to add synchronization between batches.
        // As stated earlier batches are groups of tasks which can execute together without sync between them.
        // To simplify and optimize the sync placement daxa only synchronizes between batches.
        // This effectively means that all the resource uses, and their memory and execution dependencies in a batch
        // are combined into a single unit which is synchronized against other batches.
        for (auto const & [used_buffer_t_id, used_buffer_t_access] : info.used_buffers)
        {
            ImplTaskBuffer & impl_task_buffer = impl.impl_task_buffers[used_buffer_t_id.index];
            Access const current_buffer_access = task_buffer_access_to_access(used_buffer_t_access);
            // When the last use was a read AND the new use of the buffer is a read AND,
            // we need to add our stage flags to the existing barrier of the last use.
            bool const is_last_access_read = impl_task_buffer.latest_access.type == AccessTypeFlagBits::READ;
            bool const is_current_access_read = current_buffer_access.type == AccessTypeFlagBits::READ;
            // We only need barriers between two accesses.
            // If the previous access is none, the current access is the first access.
            // Therefore we do not need to insert any synchronization if the previous access is none.
            // This is buffer specific. Images have a layout that needs to be set from undefined to the current accesses layout.
            // When the latest access  is a read that did not require a barrier before we also do not need a barrier now.
            // So skip, if the latest access is read and there is no latest_access_read_barrier_index present.
            bool const is_last_access_none = impl_task_buffer.latest_access == AccessConsts::NONE;
            if (!is_last_access_none && !(std::holds_alternative<std::monostate>(impl_task_buffer.latest_access_read_barrier_index) && is_last_access_read))
            {
                if (is_last_access_read && is_current_access_read)
                {
                    if (LastReadSplitBarrierIndex const * index0 = std::get_if<LastReadSplitBarrierIndex>(&impl_task_buffer.latest_access_read_barrier_index))
                    {
                        auto & last_read_split_barrier = impl.split_barriers[index0->index];
                        last_read_split_barrier.dst_access = last_read_split_barrier.dst_access | current_buffer_access;
                    }
                    else if (LastReadBarrierIndex const * index1 = std::get_if<LastReadBarrierIndex>(&impl_task_buffer.latest_access_read_barrier_index))
                    {
                        auto & last_read_barrier = impl.barriers[index1->index];
                        last_read_barrier.dst_access = last_read_barrier.dst_access | current_buffer_access;
                    }
                }
                else
                {
                    // When the uses are incompatible (no read on read) we need to insert a new barrier.
                    // Host access needs to be handeled in a specialized way.
                    bool const src_host_only_access = impl_task_buffer.latest_access.stages == PipelineStageFlagBits::HOST;
                    bool const dst_host_only_access = current_buffer_access.stages == PipelineStageFlagBits::HOST;
                    DAXA_DBG_ASSERT_TRUE_M(!(src_host_only_access && dst_host_only_access), "direct sync between two host accesses on gpu are not allowed");
                    bool const is_host_barrier = src_host_only_access || dst_host_only_access;
                    // When the distance between src and dst batch is one, we can replace the split barrier with a normal barrier.
                    // We also need to make sure we do not use split barriers when the src or dst stage exclusively uses the host stage.
                    // This is because the host stage does not declare an execution dependency on the cpu but only a memory dependency.
                    bool const use_pipeline_barrier =
                        (impl_task_buffer.latest_access_batch_index + 1 == batch_index &&
                         current_submit_scope_index == impl_task_buffer.latest_access_submit_scope_index) ||
                        is_host_barrier;
                    if (use_pipeline_barrier)
                    {
                        usize const barrier_index = impl.barriers.size();
                        impl.barriers.push_back(TaskBarrier{
                            .image_id = {}, // {} signals that this is not an image barrier.
                            .src_access = impl_task_buffer.latest_access,
                            .dst_access = current_buffer_access,
                            .src_batch = impl_task_buffer.latest_access_batch_index,
                            .dst_batch = batch_index,
                        });
                        // And we insert the barrier index into the list of pipeline barriers of the current tasks batch.
                        batch.pipeline_barrier_indices.push_back(barrier_index);
                        if (current_buffer_access.type == AccessTypeFlagBits::READ)
                        {
                            // As the new access is a read we remember our barrier index,
                            // So that potential future reads after this can reuse this barrier.
                            impl_task_buffer.latest_access_read_barrier_index = LastReadBarrierIndex{barrier_index};
                        }
                    }
                    else
                    {
                        usize const split_barrier_index = impl.split_barriers.size();
                        impl.split_barriers.push_back(TaskSplitBarrier{
                            {
                                .image_id = {}, // {} signals that this is not an image barrier.
                                .src_access = impl_task_buffer.latest_access,
                                .dst_access = current_buffer_access,
                                .src_batch = impl_task_buffer.latest_access_batch_index,
                                .dst_batch = batch_index,
                            },
                            /* .split_barrier_state = */ impl.info.device.create_split_barrier({
                                .debug_name = std::string("TaskList \"") + impl.info.debug_name + "\" SplitBarrier Nr. " + std::to_string(split_barrier_index),
                            }),
                        });
                        // Now we give the src batch the index of this barrier to signal.
                        TaskBatchSubmitScope & src_scope = impl.batch_submit_scopes[impl_task_buffer.latest_access_submit_scope_index];
                        TaskBatch & src_batch = src_scope.task_batches[impl_task_buffer.latest_access_batch_index];
                        src_batch.signal_split_barrier_indices.push_back(split_barrier_index);
                        // And we also insert the split barrier index into the waits of the current tasks batch.
                        batch.wait_split_barrier_indices.push_back(split_barrier_index);
                        if (current_buffer_access.type == AccessTypeFlagBits::READ)
                        {
                            // As the new access is a read we remember our barrier index,
                            // So that potential future reads after this can reuse this barrier.
                            impl_task_buffer.latest_access_read_barrier_index = LastReadSplitBarrierIndex{split_barrier_index};
                        }
                        else
                        {
                            impl_task_buffer.latest_access_read_barrier_index = {};
                        }
                    }
                }
            }
            // Now that we inserted/updated the synchronization, we update the latest access.
            impl_task_buffer.latest_access = current_buffer_access;
            impl_task_buffer.latest_access_batch_index = batch_index;
            impl_task_buffer.latest_access_submit_scope_index = current_submit_scope_index;
        }
        // Now we insert image dependent sync
        for (auto const & [used_image_t_id, used_image_t_access, initial_used_image_slice] : info.used_images)
        {
            ImplTaskImage & impl_task_image = impl.impl_task_images[used_image_t_id.index];
            auto [current_image_layout, current_image_access] = task_image_access_to_layout_access(used_image_t_access);
            // Now this seems strange, why would be need multiple current use slices, as we only have one here.
            // This is because when we intersect this slice with the tracked slices, we get an intersection and a rest.
            // We need to then test the rest against all the remaining tracked uses,
            // as the intersected part is already beeing handled in the following code.
            tl_new_use_slices.push_back(initial_used_image_slice);
            // This is the tracked slice we will insert after we finished analyzing the current used image.
            TaskImageTrackedSlice ret_new_use_tracked_slice{
                .latest_access = current_image_access,
                .latest_layout = current_image_layout,
                .latest_access_batch_index = batch_index,
                .latest_access_submit_scope_index = current_submit_scope_index,
                .latest_access_read_barrier_index = {}, // This is a dummy value (either set later or ignored entirely).
                .slice = initial_used_image_slice,
            };
            // As image subresources can be in different layouts and also different synchronization scopes,
            // we need to track these image ranges individually.
            for (
                auto tracked_slice_iter = impl_task_image.slices_last_uses.begin();
                tracked_slice_iter != impl_task_image.slices_last_uses.end();)
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
                    TaskImageTrackedSlice const tracked_slice = *tracked_slice_iter;
                    // We are only interested in intersecting ranges, as use of non intersecting ranges does not need synchronization.
                    if (!used_image_slice.intersects(tracked_slice.slice))
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
                    auto intersection = tracked_slice.slice.intersect(used_image_slice);
                    auto [tracked_slice_rest, tracked_slice_rest_count] = tracked_slice.slice.subtract(intersection);
                    auto [new_use_slice_rest, new_use_slice_rest_count] = used_image_slice.subtract(intersection);
                    // We now remove the old tracked slice from the list of tracked slices, as we just split it.
                    // We need to know if the iterator was advanced. This erase advances the iterator.
                    // If the iterator got not advanced by this we need to advance it ourself manually later.
                    advanced_tracked_slice_iterator = true;
                    tracked_slice_iter = impl_task_image.slices_last_uses.erase(tracked_slice_iter);
                    // Now we remember the left over slice from the original tracked slice.
                    for (usize rest_i = 0; rest_i < tracked_slice_rest_count; ++rest_i)
                    {
                        // The rest tracked slices are the same as the original tracked slice,
                        // except for the slice itself, which is the remainder of the subtraction of the intersection.
                        TaskImageTrackedSlice current_rest_tracked_slice = tracked_slice;
                        current_rest_tracked_slice.slice = tracked_slice_rest[rest_i];
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
                    bool const is_last_access_read = tracked_slice.latest_access.type == AccessTypeFlagBits::READ;
                    bool const is_current_access_read = current_image_access.type == AccessTypeFlagBits::READ;
                    bool const are_layouts_identical = tracked_slice.latest_layout == current_image_layout;
                    if (is_last_access_read && is_current_access_read && are_layouts_identical)
                    {
                        if (LastReadSplitBarrierIndex const * index0 = std::get_if<LastReadSplitBarrierIndex>(&tracked_slice.latest_access_read_barrier_index))
                        {
                            auto & last_read_split_barrier = impl.split_barriers[index0->index];
                            last_read_split_barrier.dst_access = last_read_split_barrier.dst_access | tracked_slice.latest_access;
                        }
                        else if (LastReadBarrierIndex const * index1 = std::get_if<LastReadBarrierIndex>(&tracked_slice.latest_access_read_barrier_index))
                        {
                            auto & last_read_barrier = impl.barriers[index1->index];
                            last_read_barrier.dst_access = last_read_barrier.dst_access | tracked_slice.latest_access;
                        }
                        else
                        {
                            // TODO(pahrens): THIS IS ACTUALLY REACHABLE WHEN THERE ARE READS FOLLOWING EACH OTHER WITH NO PRIOR BARRIER ENQUEUED FOR THIS RESOURCE!
                            DAXA_DBG_ASSERT_TRUE_M(false, "NOT IMPLEMENTED, PING Ipotrick TO FIX IT!");
                        }
                    }
                    else
                    {
                        // When the uses are incompatible (no read on read, or no identical layout) we need to insert a new barrier.
                        // Host access needs to be handeled in a specialized way.
                        bool const src_host_only_access = tracked_slice.latest_access.stages == PipelineStageFlagBits::HOST;
                        bool const dst_host_only_access = current_image_access.stages == PipelineStageFlagBits::HOST;
                        DAXA_DBG_ASSERT_TRUE_M(!(src_host_only_access && dst_host_only_access), "direct sync between two host accesses on gpu are not allowed");
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
                            usize const barrier_index = impl.barriers.size();
                            impl.barriers.push_back(TaskBarrier{
                                .image_id = used_image_t_id,
                                .slice = intersection,
                                .layout_before = tracked_slice.latest_layout,
                                .layout_after = current_image_layout,
                                .src_access = tracked_slice.latest_access,
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
                            usize const split_barrier_index = impl.split_barriers.size();
                            impl.split_barriers.push_back(TaskSplitBarrier{
                                {
                                    .image_id = used_image_t_id,
                                    .slice = intersection,
                                    .layout_before = tracked_slice.latest_layout,
                                    .layout_after = current_image_layout,
                                    .src_access = tracked_slice.latest_access,
                                    .dst_access = current_image_access,
                                    .src_batch = tracked_slice.latest_access_batch_index,
                                    .dst_batch = batch_index,
                                },
                                /* .split_barrier_state = */ impl.info.device.create_split_barrier({
                                    .debug_name = std::string("TaskList \"") + impl.info.debug_name + "\" SplitBarrier (Image) Nr. " + std::to_string(split_barrier_index),
                                }),
                            });
                            // Now we give the src batch the index of this barrier to signal.
                            TaskBatchSubmitScope & src_scope = impl.batch_submit_scopes[tracked_slice.latest_access_submit_scope_index];
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
                    if (tracked_slice_iter == impl_task_image.slices_last_uses.end())
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
            // If we have a remainder left of the used image slices, there was no previous use of those slices.
            // We need to translate those image slices into the correct layout.
            for (ImageMipArraySlice const rest_used_slice : tl_new_use_slices)
            {
                usize const pipeline_barrier_index = impl.barriers.size();
                impl.barriers.push_back(TaskBarrier{
                    .image_id = used_image_t_id,
                    .slice = rest_used_slice,
                    .layout_before = impl_task_image.info.initial_layout,
                    .layout_after = current_image_layout,
                    // In the future it may be good to give an initial image state and access.
                    .src_access = impl_task_image.info.initial_access,
                    .dst_access = current_image_access,
                    //.src_batch = impl_task_image.info.creation_batch,
                    //.dst_batch = batch_index,
                });
                batch.pipeline_barrier_indices.push_back(pipeline_barrier_index);
            }
            tl_new_use_slices.clear();
            // Now we need to add the latest use and tracked range of our current access:
            impl_task_image.slices_last_uses.push_back(ret_new_use_tracked_slice);
            // The remainder tracked slices we remembered from earlier are now inserted back into the list of tracked slices.
            // We deferred this step as we dont want to check these in the loop above, as we found them to not intersect with the new use.
            impl_task_image.slices_last_uses.insert(
                impl_task_image.slices_last_uses.end(),
                tl_tracked_slice_rests.begin(),
                tl_tracked_slice_rests.end());
            tl_tracked_slice_rests.clear();
        }
    }

    void TaskList::submit(CommandSubmitInfo * info)
    {
        auto & impl = *as<ImplTaskList>();
        DAXA_DBG_ASSERT_TRUE_M(!impl.compiled, "Can only record to an uncompleted task list");
        TaskBatchSubmitScope & submit_scope = impl.batch_submit_scopes.back();
        submit_scope.submit_info = {};
        // We provide the user submit info to the submit batch.
        submit_scope.user_submit_info = info;
        // Start a new batch.
        impl.batch_submit_scopes.push_back({
            .present_info = {},
        });
    }

    void TaskList::present(TaskPresentInfo const & info)
    {
        auto & impl = *as<ImplTaskList>();
        DAXA_DBG_ASSERT_TRUE_M(!impl.compiled, "Can only record to an uncompleted task list");
        DAXA_DBG_ASSERT_TRUE_M(impl.info.swapchain.has_value(), "Can only present, when a swapchain was provided in creation");
        DAXA_DBG_ASSERT_TRUE_M(impl.batch_submit_scopes.size() > 1, "Can only present if at least one submit was issued before");
        DAXA_DBG_ASSERT_TRUE_M(!impl.swapchain_image.is_empty(), "Can only present when an image was annotated as swapchain image");
        DAXA_DBG_ASSERT_TRUE_M(!impl.impl_task_images[impl.swapchain_image.index].swapchain_semaphore_waited_upon, "Can only present once");
        impl.impl_task_images[impl.swapchain_image.index].swapchain_semaphore_waited_upon = true;

        TaskImageTrackedSlice const & tracked_slice = impl.impl_task_images[impl.swapchain_image.index].slices_last_uses.back();
        usize submit_scope_index = tracked_slice.latest_access_submit_scope_index;
        DAXA_DBG_ASSERT_TRUE_M(submit_scope_index < impl.batch_submit_scopes.size() - 1, "the last swapchain image use MUST be before the last submit when presenting");
        TaskBatchSubmitScope & submit_scope = impl.batch_submit_scopes[submit_scope_index];
        usize const batch_index = tracked_slice.latest_access_batch_index;
        // We need to insert a pipeline barrier to transition the swapchain image layout to present src optimal.
        usize const barrier_index = impl.barriers.size();
        impl.barriers.push_back(TaskBarrier{
            .image_id = impl.swapchain_image,
            .slice = tracked_slice.slice,
            .layout_before = tracked_slice.latest_layout,
            .layout_after = ImageLayout::PRESENT_SRC,
            .src_access = tracked_slice.latest_access,
            .dst_access = {.stages = PipelineStageFlagBits::BOTTOM_OF_PIPE},
            .src_batch = batch_index,
            .dst_batch = batch_index + 1,
        });
        submit_scope.last_minute_barrier_indices.push_back(barrier_index);
        // Now we need to insert the binary semaphore between submit and present.
        submit_scope.present_info = ImplPresentInfo{
            .user_binary_semaphores = info.user_binary_semaphores,
        };
    }

    void TaskList::complete()
    {
        auto & impl = *as<ImplTaskList>();
        DAXA_DBG_ASSERT_TRUE_M(!impl.compiled, "Can only complete a task list one time");
        impl.compiled = true;
    }

    auto TaskList::get_command_lists() -> std::vector<CommandList>
    {
        auto & impl = *as<ImplTaskList>();
        DAXA_DBG_ASSERT_TRUE_M(impl.compiled, "Can only get command lists of a finished task list");
        DAXA_DBG_ASSERT_TRUE_M(impl.executed, "Can only get command lists of a task list that has been executed");
        auto command_lists = std::move(impl.left_over_command_lists);
        impl.left_over_command_lists = {};
        return command_lists;
    }

    void TaskList::output_graphviz()
    {
        auto & impl = *as<ImplTaskList>();
        DAXA_DBG_ASSERT_TRUE_M(impl.compiled, "must compile before getting graphviz");
        impl.output_graphviz();
    }

    thread_local std::vector<SplitBarrierWaitInfo> tl_split_barrier_wait_infos = {};
    thread_local std::vector<ImageBarrierInfo> tl_image_barrier_infos = {};
    thread_local std::vector<MemoryBarrierInfo> tl_memory_barrier_infos = {};
    void insert_pipeline_barrier(CommandList & command_list, TaskBarrier & barrier, std::vector<daxa::ImplTaskImage> & task_images)
    {
        // Check if barrier is image barrier or normal barrier (see TaskBarrier struct comments).
        if (barrier.image_id.is_empty())
        {
            command_list.pipeline_barrier({
                .awaited_pipeline_access = barrier.src_access,
                .waiting_pipeline_access = barrier.dst_access,
            });
        }
        else
        {
            for (auto & image : task_images[barrier.image_id.index].images)
            {
                command_list.pipeline_barrier_image_transition({
                    .awaited_pipeline_access = barrier.src_access,
                    .waiting_pipeline_access = barrier.dst_access,
                    .before_layout = barrier.layout_before,
                    .after_layout = barrier.layout_after,
                    .image_slice = barrier.slice,
                    .image_id = image,
                });
            }
        }
    }

    void TaskList::execute()
    {
        auto & impl = *as<ImplTaskList>();
        ImplTaskRuntime impl_runtime{.task_list = impl};
        impl_runtime.command_lists.push_back(impl.info.device.create_command_list({.debug_name = std::string("Task Command List ") + std::to_string(impl_runtime.command_lists.size())}));
        // DAXA_DBG_ASSERT_TRUE_M(impl.compiled, "must compile before executing");

        //- Go through all TaskBatchSubmitScopes
        //  - Go through all TaskBatches
        //      - Wait for all pipeline barriers
        //      - Wait for all split barrier indices
        //      - Create all barriers for this TaskBatch
        //      - Execute all task in this task batch
        //      - reset all split barriers that were waited on
        //      - Set all split barriers
        //  - insert all last minute pipeline barriers
        //  - do optional present
        usize submit_scope_index = 0;
        for (auto & submit_scope : impl.batch_submit_scopes)
        {
            for (auto & task_batch : submit_scope.task_batches)
            {
                // Wait on pipeline barriers before batch execution.
                for (auto barrier_index : task_batch.pipeline_barrier_indices)
                {
                    TaskBarrier & barrier = impl.barriers[barrier_index];
                    insert_pipeline_barrier(impl_runtime.command_lists.back(), barrier, impl.impl_task_images);
                }
                // Wait on split barriers before batch execution.
                if (!impl.info.use_split_barriers)
                {
                    for (auto barrier_index : task_batch.wait_split_barrier_indices)
                    {
                        TaskSplitBarrier const & split_barrier = impl.split_barriers[barrier_index];
                        // Convert split barrier to normal barrier.
                        TaskBarrier barrier = split_barrier;
                        insert_pipeline_barrier(impl_runtime.command_lists.back(), barrier, impl.impl_task_images);
                    }
                }
                else
                {
                    usize needed_image_barriers = 0;
                    for (auto barrier_index : task_batch.wait_split_barrier_indices)
                    {
                        TaskSplitBarrier const & split_barrier = impl.split_barriers[barrier_index];
                        if (!split_barrier.image_id.is_empty())
                        {
                            needed_image_barriers += impl.impl_task_images[split_barrier.image_id.index].images.size();
                        }
                    }
                    tl_split_barrier_wait_infos.reserve(task_batch.wait_split_barrier_indices.size());
                    tl_memory_barrier_infos.reserve(task_batch.wait_split_barrier_indices.size());
                    tl_image_barrier_infos.reserve(needed_image_barriers);
                    for (auto barrier_index : task_batch.wait_split_barrier_indices)
                    {
                        TaskSplitBarrier & split_barrier = impl.split_barriers[barrier_index];
                        if (split_barrier.image_id.is_empty())
                        {
                            tl_memory_barrier_infos.push_back(MemoryBarrierInfo{
                                .awaited_pipeline_access = split_barrier.src_access,
                                .waiting_pipeline_access = split_barrier.dst_access,
                            });
                            tl_split_barrier_wait_infos.push_back(SplitBarrierWaitInfo{
                                .memory_barriers = std::span{&tl_memory_barrier_infos.back(), 1},
                                .split_barrier = split_barrier.split_barrier_state,
                            });
                        }
                        else
                        {
                            usize const img_bar_vec_start_size = tl_image_barrier_infos.size();
                            for (auto image : impl.impl_task_images[split_barrier.image_id.index].images)
                            {
                                tl_image_barrier_infos.push_back(ImageBarrierInfo{
                                    .awaited_pipeline_access = split_barrier.src_access,
                                    .waiting_pipeline_access = split_barrier.dst_access,
                                    .before_layout = split_barrier.layout_before,
                                    .after_layout = split_barrier.layout_after,
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
                for (TaskId const task_id : task_batch.tasks)
                {
                    // We always allow to reuse the last command list ONCE.
                    // when the get command list function is called in a task this is set to false.
                    impl_runtime.reuse_last_command_list = true;
                    Task & task = impl.tasks[task_id];
                    impl_runtime.current_task = &task;
                    task.info.task(TaskRuntime(&impl_runtime));
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
                            .barrier = impl.split_barriers[barrier_index].split_barrier_state,
                            .stage_masks = impl.split_barriers[barrier_index].dst_access.stages,
                        });
                    }
                    // Signal all signal split barriers after batch execution.
                    for (usize const barrier_index : task_batch.signal_split_barrier_indices)
                    {
                        TaskSplitBarrier & task_split_barrier = impl.split_barriers[barrier_index];
                        if (task_split_barrier.image_id.is_empty())
                        {
                            MemoryBarrierInfo memory_barrier{
                                .awaited_pipeline_access = task_split_barrier.src_access,
                                .waiting_pipeline_access = task_split_barrier.dst_access,
                            };
                            impl_runtime.command_lists.back().signal_split_barrier({
                                .memory_barriers = std::span{&memory_barrier, 1},
                                .split_barrier = task_split_barrier.split_barrier_state,
                            });
                        }
                        else
                        {
                            for (auto image : impl.impl_task_images[task_split_barrier.image_id.index].images)
                            {
                                tl_image_barrier_infos.push_back({
                                    .awaited_pipeline_access = task_split_barrier.src_access,
                                    .waiting_pipeline_access = task_split_barrier.dst_access,
                                    .before_layout = task_split_barrier.layout_before,
                                    .after_layout = task_split_barrier.layout_after,
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
            }
            for (usize const barrier_index : submit_scope.last_minute_barrier_indices)
            {
                TaskBarrier & barrier = impl.barriers[barrier_index];
                insert_pipeline_barrier(impl_runtime.command_lists.back(), barrier, impl.impl_task_images);
            }
            for (auto & command_list : impl_runtime.command_lists)
            {
                DAXA_DBG_ASSERT_TRUE_M(
                    !command_list.is_complete(),
                    "it is illegal to complete command lists in tasks that are obtained by the runtime!");
                command_list.complete();
            }

            if (&submit_scope != &impl.batch_submit_scopes.back())
            {
                auto submit_info = submit_scope.submit_info;
                submit_info.command_lists.insert(submit_info.command_lists.end(), impl_runtime.command_lists.begin(), impl_runtime.command_lists.end());
                if (impl.info.swapchain.has_value())
                {
                    Swapchain const & swapchain = impl.info.swapchain.value();
                    if (submit_scope_index == impl.swapchain_image_first_use_submit_scope_index)
                    {
                        submit_info.wait_binary_semaphores.push_back(swapchain.get_acquire_semaphore());
                    }
                    if (submit_scope_index == impl.swapchain_image_last_use_submit_scope_index)
                    {
                        submit_info.signal_binary_semaphores.push_back(swapchain.get_present_semaphore());
                        submit_info.signal_timeline_semaphores.emplace_back(
                            swapchain.get_gpu_timeline_semaphore(),
                            swapchain.get_cpu_timeline_value());
                    }
                }
                if (submit_scope.user_submit_info != nullptr)
                {
                    submit_info.command_lists.insert(submit_info.command_lists.end(), submit_scope.user_submit_info->command_lists.begin(), submit_scope.user_submit_info->command_lists.end());
                    submit_info.wait_binary_semaphores.insert(submit_info.wait_binary_semaphores.end(), submit_scope.user_submit_info->wait_binary_semaphores.begin(), submit_scope.user_submit_info->wait_binary_semaphores.end());
                    submit_info.signal_binary_semaphores.insert(submit_info.signal_binary_semaphores.end(), submit_scope.user_submit_info->signal_binary_semaphores.begin(), submit_scope.user_submit_info->signal_binary_semaphores.end());
                    submit_info.wait_timeline_semaphores.insert(submit_info.wait_timeline_semaphores.end(), submit_scope.user_submit_info->wait_timeline_semaphores.begin(), submit_scope.user_submit_info->wait_timeline_semaphores.end());
                    submit_info.signal_timeline_semaphores.insert(submit_info.signal_timeline_semaphores.end(), submit_scope.user_submit_info->signal_timeline_semaphores.begin(), submit_scope.user_submit_info->signal_timeline_semaphores.end());
                }
                impl.info.device.submit_commands(submit_info);

                if (submit_scope.present_info.has_value())
                {
                    ImplPresentInfo & impl_present_info = submit_scope.present_info.value();
                    std::vector<BinarySemaphore> present_wait_semaphores = impl_present_info.binary_semaphores;
                    present_wait_semaphores.push_back(impl.info.swapchain.value().get_present_semaphore());
                    if (impl_present_info.user_binary_semaphores != nullptr)
                    {
                        present_wait_semaphores.insert(
                            present_wait_semaphores.end(),
                            impl_present_info.user_binary_semaphores->begin(),
                            impl_present_info.user_binary_semaphores->end());
                    }
                    impl.info.device.present_frame(PresentInfo{
                        .wait_binary_semaphores = present_wait_semaphores,
                        .swapchain = impl.info.swapchain.value(),
                    });
                }
                // We need to clear all completed command lists that have been submitted.
                impl_runtime.command_lists.clear();
                impl_runtime.command_lists.push_back(impl.info.device.create_command_list({.debug_name = std::string("Task Command List ") + std::to_string(impl_runtime.command_lists.size())}));
            }
            ++submit_scope_index;
        }

        impl.left_over_command_lists = std::move(impl_runtime.command_lists);
        impl.executed = true;
    }

    auto TaskList::last_access(TaskBufferId buffer) -> Access
    {
        auto & impl = *as<ImplTaskList>();
        DAXA_DBG_ASSERT_TRUE_M(impl.compiled, "final access only available after compilation");

        return impl.impl_task_buffers[buffer.index].latest_access;
    }

    auto TaskList::last_uses(TaskImageId image) -> std::vector<TaskImageLastUse>
    {
        auto & impl = *as<ImplTaskList>();
        DAXA_DBG_ASSERT_TRUE_M(impl.compiled, "final access only available after compilation");

        std::vector<TaskImageLastUse> result = {};
        result.reserve(impl.impl_task_images[image.index].slices_last_uses.size());

        for (auto const & tracked_slice : impl.impl_task_images[image.index].slices_last_uses)
        {
            result.push_back({
                .slice = tracked_slice.slice,
                .layout = tracked_slice.latest_layout,
                .access = tracked_slice.latest_access,
            });
        }

        return result;
    }

    ImplTaskList::ImplTaskList(TaskListInfo a_info)
        : info{std::move(a_info)}
    {
    }

    ImplTaskList::~ImplTaskList() = default;

    void ImplTaskList::output_graphviz()
    {
        // TODO(grundlett): Implement this!
        std::string const filename = this->info.debug_name + ".dot";
        std::ofstream dot_file{filename};

        dot_file << "digraph TaskGraph {\nrankdir=\"LR\"\nnode [style=filled, shape=box, color=\"#d3f4ff\"]\n";

        // usize scope_index = 0;
        for (auto const & scope : batch_submit_scopes)
        {
            usize batch_index = 0;
            for (auto const & batch : scope.task_batches)
            {
                auto batch_name = std::string("b_") + std::to_string(batch_index);
                auto batch_debug_name = "Batch " + std::to_string(batch_index);

                {
                    dot_file << "subgraph cluster_pb_" << batch_name << " {\n"
                             //  << "label=\"" << batch_debug_name << "\"\n"
                             << "style=filled\ncolor=\"#b5bec4\"\n";

                    for (auto const & barrier_index : batch.pipeline_barrier_indices)
                    {
                        auto const & barrier = this->barriers[barrier_index];
                        std::string const name = batch_name + std::string("_pb_") + std::to_string(barrier.src_batch) + std::string("_") + std::to_string(barrier.dst_batch);
                        dot_file << "node_" << name << " [label=\"" << name << "\", shape=box, color=\"#faaff0\"]\n";
                    }
                    for (auto const & barrier_index : batch.wait_split_barrier_indices)
                    {
                        auto const & barrier = this->split_barriers[barrier_index];
                        std::string const name0 = std::string("b_") + std::to_string(barrier.src_batch) + std::string("_ssb_") + std::to_string(barrier.src_batch) + std::string("_") + std::to_string(barrier.dst_batch);
                        std::string const name = batch_name + std::string("_wsb_") + std::to_string(barrier.src_batch) + std::string("_") + std::to_string(barrier.dst_batch);
                        dot_file << "node_" << name << " [label=\"" << name << "\", shape=box, color=\"#fa8989\"]\n";
                        dot_file << "node_" << name0 << "->node_" << name << "\n";
                    }

                    dot_file << "node_pb_" << batch_name << " [label=\"" << batch_debug_name << " Barriers\", shape=box]\n";
                    dot_file << "}\n";
                }

                {
                    dot_file << "subgraph cluster_" << batch_name << " {\n"
                             //  << "label=\"" << batch_debug_name << "\"\n"
                             << "style=filled\ncolor=\"#b5bec4\"\n";
                    for (auto const & task_id : batch.tasks)
                    {
                        auto task_name = batch_name + std::string("_t_") + std::to_string(task_id);
                        auto task_debug_name = tasks[task_id].info.debug_name;
                        dot_file << "subgraph cluster_" << task_name << " {\n"
                                 << "label=\"" << task_debug_name << "\"\n"
                                 << "style=filled\ncolor=\"#d1e2ed\"\n";
                        // dot_file << "node_" << task_name << " [label=\"" << task_debug_name << "\", shape=box]\n";
                        usize resource_index = 0;

                        resource_index = 0;
                        for (auto const & [task_buffer_id, task_buffer_access] : tasks[task_id].info.used_buffers)
                        {
                            auto const & task_resource = this->impl_task_buffers[task_buffer_id.index];
                            auto const & resource_debug_name = task_resource.info.debug_name;
                            dot_file << "node_" << task_name << "_br" << resource_index << " [label=\"" << resource_debug_name << "\", shape=box, color=\"#d3fabe\"]\n";
                            ++resource_index;
                        }

                        resource_index = 0;
                        for (auto const & [task_image_id, task_buffer_access, image_slice] : tasks[task_id].info.used_images)
                        {
                            auto const & task_resource = this->impl_task_images[task_image_id.index];
                            auto const & resource_debug_name = task_resource.info.debug_name;
                            dot_file << "node_" << task_name << "_ir" << resource_index << " [label=\"" << resource_debug_name << "\", shape=box, color=\"#fffec2\"]\n";
                            ++resource_index;
                        }

                        dot_file << "}\n";
                    }

                    dot_file << "node_" << batch_name << " [label=\"" << batch_debug_name << "\", shape=box]\n";
                    dot_file << "}\n";
                }

                {
                    dot_file << "subgraph cluster_ssb_" << batch_name << " {\n"
                             //  << "label=\"" << batch_debug_name << "\"\n"
                             << "style=filled\ncolor=\"#b5bec4\"\n";

                    for (auto const & barrier_index : batch.signal_split_barrier_indices)
                    {
                        auto const & barrier = this->split_barriers[barrier_index];
                        std::string const name = batch_name + std::string("_ssb_") + std::to_string(barrier.src_batch) + std::string("_") + std::to_string(barrier.dst_batch);
                        dot_file << "node_" << name << " [label=\"" << name << "\", shape=box, color=\"#fcc5c5\"]\n";
                    }

                    dot_file << "node_ssb_" << batch_name << " [label=\"" << batch_debug_name << " Signal Split Barriers \", shape=box]\n";
                    dot_file << "}\n";
                }

                if (batch_index > 0)
                {
                    dot_file << "node_ssb_b_" << (batch_index - 1) << "->node_pb_b_" << (batch_index) << "\n";
                }
                dot_file << "node_pb_b_" << (batch_index) << "->node_b_" << (batch_index) << "\n";
                dot_file << "node_b_" << (batch_index) << "->node_ssb_b_" << (batch_index) << "\n";

                ++batch_index;
            }
            // ++scope_index;
        }

        // for (auto & buffer_link : compiled_graph.buffer_links)
        // {
        //     auto a = buffer_link.event_a;
        //     auto b = buffer_link.event_b;
        //     auto i = buffer_link.resource;
        //     if (ImplCreateBufferTask * task_ptr = std::get_if<ImplCreateBufferTask>(&events[a].event_variant))
        //         dot_file << "c_";
        //     dot_file << "bnode_" << a << "_" << i << "->bnode_" << b << "_" << i;
        //     dot_file << " [label=\"Sync\", labeltooltip=\"between "
        //              << to_string(buffer_link.barrier.awaited_pipeline_access.stages) << " "
        //              << to_string(buffer_link.barrier.awaited_pipeline_access.type) << " and "
        //              << to_string(buffer_link.barrier.waiting_pipeline_access.stages) << " "
        //              << to_string(buffer_link.barrier.waiting_pipeline_access.type) << "\"]\n";
        // }

        // for (auto & image_link : compiled_graph.image_links)
        // {
        //     auto a = image_link.event_a;
        //     auto b = image_link.event_b;
        //     auto i = image_link.resource;
        //     if (TaskImageCreateEvent * task_ptr = std::get_if<TaskImageCreateEvent>(&events[a].event_variant))
        //         dot_file << "c_";
        //     dot_file << "inode_" << a << "_" << i << "->inode_" << b << "_" << i;
        //     dot_file << " [label=\"Sync\", labeltooltip=\"between "
        //              << to_string(image_link.barrier.awaited_pipeline_access.stages) << " "
        //              << to_string(image_link.barrier.awaited_pipeline_access.type) << " and "
        //              << to_string(image_link.barrier.waiting_pipeline_access.stages) << " "
        //              << to_string(image_link.barrier.waiting_pipeline_access.type) << "\"]\n";
        // }

        dot_file << "}\n";
    }

    void ImplTaskList::debug_print_task_barrier(TaskBarrier & barrier, usize index, std::string_view prefix)
    {
#ifdef DAXA_TASK_LIST_DEBUG
        // Check if barrier is image barrier or normal barrier (see TaskBarrier struct comments).
        if (barrier.image_id.is_empty())
        {
            std::cout << prefix << "Begin Memory barrier\n";
            std::cout << prefix << "\tbarrier index: " << index << "\n";
            std::cout << prefix << "\t.src_access = " << to_string(barrier.src_access) << "\n";
            std::cout << prefix << "\t.dst_access = " << to_string(barrier.dst_access) << "\n";
            std::cout << prefix << "End   Memory barrier\n";
        }
        else
        {
            ImplTaskImage const & impl_task_image = impl_task_images[barrier.image_id.index];
            std::cout << prefix << "Begin image memory barrier\n";
            std::cout << prefix << "\tbarrier index: " << index << "\n";
            std::cout << prefix << "\ttask_image_id: " << barrier.image_id.index << " \n";
            std::cout << prefix << "\ttask image debug name: " << impl_task_image.info.debug_name << " \n";
            std::cout << prefix << "\tBegin bound images\n";
            for (auto image : impl_task_images[barrier.image_id.index].images)
            {
                std::cout << prefix << "\timage id: " << to_string(image)
                          << "\timage debug name: " << (image.is_empty() ? std::string("INVALID ID") : info.device.info_image(image).debug_name) << " \n";
            }
            std::cout << prefix << "\tEnd   bound images \n";
            std::cout << prefix << "\tsrc access: " << to_string(barrier.src_access) << "\n";
            std::cout << prefix << "\tdst access: " << to_string(barrier.dst_access) << "\n";
            std::cout << prefix << "\timage mip array slice: " << to_string(barrier.slice) << "\n";
            std::cout << prefix << "\tbefore layout: " << to_string(barrier.layout_before) << "\n";
            std::cout << prefix << "\tafter layout: " << to_string(barrier.layout_after) << "\n";
            std::cout << prefix << "End   image memory barrier\n";
        }
#endif // #ifdef DAXA_TASK_LIST_DEBUG
    }

    void ImplTaskList::debug_print_task_split_barrier(TaskSplitBarrier & barrier, usize index, std::string_view prefix)
    {
#ifdef DAXA_TASK_LIST_DEBUG
        // Check if barrier is image barrier or normal barrier (see TaskBarrier struct comments).
        if (barrier.image_id.is_empty())
        {
            std::cout << prefix << "Begin split memory barrier\n";
            std::cout << prefix << "\tsplit barrier index: " << index << "\n";
            std::cout << prefix << "\tsrc_access = " << to_string(barrier.src_access) << "\n";
            std::cout << prefix << "\tdst_access = " << to_string(barrier.dst_access) << "\n";
            std::cout << prefix << "End   split memory barrier\n";
        }
        else
        {
            ImplTaskImage const & impl_task_image = impl_task_images[barrier.image_id.index];
            std::cout << prefix << "Begin image memory barrier\n";
            std::cout << prefix << "\tbarrier index: " << index << "\n";
            std::cout << prefix << "\ttask_image_id: " << barrier.image_id.index << " \n";
            std::cout << prefix << "\ttask image debug name: " << impl_task_image.info.debug_name << " \n";
            std::cout << prefix << "\tBegin bound images\n";
            for (auto image : impl_task_images[barrier.image_id.index].images)
            {
                std::cout << prefix << "\timage id: " << to_string(image)
                          << "\timage debug name: " << (image.is_empty() ? std::string("INVALID ID") : info.device.info_image(image).debug_name) << " \n";
            }
            std::cout << prefix << "\tEnd   bound images \n";
            std::cout << prefix << "\tsrc access: " << to_string(barrier.src_access) << "\n";
            std::cout << prefix << "\tdst access: " << to_string(barrier.dst_access) << "\n";
            std::cout << prefix << "\timage mip array slice: " << to_string(barrier.slice) << "\n";
            std::cout << prefix << "\tbefore layout: " << to_string(barrier.layout_before) << "\n";
            std::cout << prefix << "\tafter layout: " << to_string(barrier.layout_after) << "\n";
            std::cout << prefix << "End   image memory barrier\n";
        }
#endif // #ifdef DAXA_TASK_LIST_DEBUG
    }

    void ImplTaskList::debug_print_task(Task & task, usize task_id, std::string_view prefix)
    {
#ifdef DAXA_TASK_LIST_DEBUG
        std::cout << prefix << "Begin task " << task_id << " name: \"" << task.info.debug_name << "\"\n";
        for (auto [task_image_id, task_image_access, slice] : task.info.used_images)
        {
            ImplTaskImage const & impl_task_image = impl_task_images[task_image_id.index];
            auto [layout, access] = task_image_access_to_layout_access(task_image_access);
            std::cout << prefix << "\tBegin task image use " << task_image_id.index << "\n";
            std::cout << prefix << "\ttask_image_id: " << task_image_id.index << " \n";
            std::cout << prefix << "\ttask image debug name: " << impl_task_image.info.debug_name << " \n";
            std::cout << prefix << "\tBegin bound images\n";
            for (auto image : impl_task_images[task_image_id.index].images)
            {
                std::cout << prefix << "\timage id: " << to_string(image)
                          << "\timage debug name: " << (image.is_empty() ? std::string("INVALID ID") : info.device.info_image(image).debug_name) << " \n";
            }
            std::cout << prefix << "\tEnd   bound images \n";
            std::cout << prefix << "\t\trequired layout: " << to_string(layout) << "\n";
            std::cout << prefix << "\t\tslice: " << to_string(slice) << "\n";
            std::cout << prefix << "\t\tstage access: " << to_string(access) << "\n";
            std::cout << prefix << "\tEnd   task image use\n";
        }
        for (auto [task_buffer_id, task_buffer_access] : task.info.used_buffers)
        {
            ImplTaskBuffer const & impl_task_buffer = impl_task_buffers[task_buffer_id.index];
            auto access = task_buffer_access_to_access(task_buffer_access);
            std::cout << prefix << "\tBegin task buffer use " << task_buffer_id.index << "\n";
            std::cout << prefix << "\t\task buffer debug name: " << impl_task_buffer.info.debug_name << "\n";
            std::cout << prefix << "\tBegin bound buffers\n";
            for (auto buffer : impl_task_buffers[task_buffer_id.index].buffers)
            {
                std::cout << prefix << "\tbuffers id: " << to_string(buffer)
                          << "\tbuffers debug name: " << (buffer.is_empty() ? std::string("INVALID ID") : info.device.info_buffer(buffer).debug_name) << " \n";
            }
            std::cout << prefix << "\tEnd   bound buffers \n";
            std::cout << prefix << "\t\tstage access: " << to_string(access) << "\n";
            std::cout << prefix << "\tEnd   task buffer use\n";
        }
        std::cout << prefix << "End   task\n";
#endif // #ifdef DAXA_TASK_LIST_DEBUG
    }

    void TaskList::debug_print()
    {
#ifdef DAXA_TASK_LIST_DEBUG
        auto & impl = *as<ImplTaskList>();
        std::cout << "Begin TaskList \"" << impl.info.debug_name << "\"\n";
        std::cout << "\tSwapchain: " << (impl.info.swapchain.has_value() ? impl.info.swapchain.value().info().debug_name : "-") << "\n";
        std::cout << "\treorder tasks: " << std::boolalpha << impl.info.reorder_tasks << "\n";
        std::cout << "\tuse split barriers: " << std::boolalpha << impl.info.use_split_barriers << "\n";
        usize submit_scope_index = 0;
        for (auto & submit_scope : impl.batch_submit_scopes)
        {
            std::cout << "\tBegin submit scope " << submit_scope_index++ << "\n";
            usize batch_index = 0;
            for (auto & task_batch : submit_scope.task_batches)
            {
                std::cout << "\t\tBegin task batch " << batch_index++ << "\n";
                // Wait on pipeline barriers before batch execution.
                std::cout << "\t\t\tBegin wait pipeline barriers\n";
                for (auto barrier_index : task_batch.pipeline_barrier_indices)
                {
                    impl.debug_print_task_barrier(impl.barriers[barrier_index], barrier_index, "\t\t\t\t");
                }
                std::cout << "\t\t\tEnd   wait pipeline barriers\n";
                if (!impl.info.use_split_barriers)
                {
                    std::cout << "\t\t\tBegin wait split barriers (converted to pipeline barriers)\n";
                    for (auto barrier_index : task_batch.wait_split_barrier_indices)
                    {
                        impl.debug_print_task_barrier(impl.split_barriers[barrier_index], barrier_index, "\t\t\t\t");
                    }
                    std::cout << "\t\t\tEnd   wait split barriers (converted to pipeline barriers)\n";
                }
                else
                {
                    std::cout << "\t\t\tBegin wait split barriers\n";
                    for (auto barrier_index : task_batch.wait_split_barrier_indices)
                    {
                        impl.debug_print_task_split_barrier(impl.split_barriers[barrier_index], barrier_index, "\t\t\t\t");
                    }
                    std::cout << "\t\t\tEnd   wait split barriers\n";
                }
                std::cout << "\t\t\tBegin tasks\n";
                for (TaskId const task_id : task_batch.tasks)
                {
                    Task & task = impl.tasks[task_id];
                    impl.debug_print_task(task, task_id, "\t\t\t\t");
                }
                std::cout << "\t\t\tEnd   tasks\n";
                if (impl.info.use_split_barriers)
                {
                    std::cout << "\t\t\tBegin signal split barriers\n";
                    for (usize const barrier_index : task_batch.signal_split_barrier_indices)
                    {
                        impl.debug_print_task_split_barrier(impl.split_barriers[barrier_index], barrier_index, "\t\t\t\t");
                    }
                    std::cout << "\t\t\tEnd   signal split barriers\n";
                }
                std::cout << "\t\tEnd   task batch\n";
            }
            std::cout << "\t\tBegin last minute pipeline barriers\n";
            for (usize const barrier_index : submit_scope.last_minute_barrier_indices)
            {
                impl.debug_print_task_barrier(impl.barriers[barrier_index], barrier_index, "\t\t\t\t");
            }
            std::cout << "\t\tEnd   last minute pipeline barriers\n";
            if (&submit_scope != &impl.batch_submit_scopes.back())
            {
                std::cout << "\t\t<<Submit>>\n";
                if (submit_scope.present_info.has_value())
                {
                    std::cout << "\t\t<<Present>>\n";
                }
            }
            std::cout << "\tEnd   submit scope\n";
        }
        std::cout << "End TaskList\n";
        std::cout.flush();
#endif // #ifdef DAXA_TASK_LIST_DEBUG
    }
} // namespace daxa

#endif
