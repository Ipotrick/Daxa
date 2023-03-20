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

    TaskRuntimeInterface::TaskRuntimeInterface(void * a_backend)
        : backend{a_backend}
    {
    }

    auto TaskRuntimeInterface::get_device() const -> Device &
    {
        auto & impl = *static_cast<ImplTaskRuntimeInterface *>(this->backend);
        return impl.task_list.info.device;
    }

    auto TaskRuntimeInterface::get_command_list() const -> CommandList
    {
        auto & impl = *static_cast<ImplTaskRuntimeInterface *>(this->backend);
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

    auto TaskRuntimeInterface::get_used_task_buffers() const -> UsedTaskBuffers const &
    {
        auto & impl = *static_cast<ImplTaskRuntimeInterface *>(this->backend);
        return impl.current_task->info.used_buffers;
    }

    auto TaskRuntimeInterface::get_used_task_images() const -> UsedTaskImages const &
    {
        auto & impl = *static_cast<ImplTaskRuntimeInterface *>(this->backend);
        return impl.current_task->info.used_images;
    }

    auto TaskRuntimeInterface::get_buffers(TaskBufferId const & task_resource_id) const -> std::span<BufferId>
    {
        auto & impl = *static_cast<ImplTaskRuntimeInterface *>(this->backend);
        DAXA_DBG_ASSERT_TRUE_M(impl.task_list.exec_task_buffers[task_resource_id.index].actual_buffers.size() > 0, "task buffer must be backed by execution buffer(s)!");
        return impl.task_list.exec_task_buffers[task_resource_id.index].actual_buffers;
    }

    auto TaskRuntimeInterface::get_images(TaskImageId const & task_resource_id) const -> std::span<ImageId>
    {
        auto & impl = *static_cast<ImplTaskRuntimeInterface *>(this->backend);
        DAXA_DBG_ASSERT_TRUE_M(impl.task_list.exec_task_images[task_resource_id.index].actual_images.size() > 0, "task image must be backed by execution image(s)!");
        return impl.task_list.exec_task_images[task_resource_id.index].actual_images;
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

    auto TaskList::create_task_buffer(TaskBufferInfo const & info) -> TaskBufferId
    {
        auto & impl = *reinterpret_cast<ImplTaskList *>(this->object);
        DAXA_DBG_ASSERT_TRUE_M(!impl.compiled, "can not record to completed command list");
        TaskBufferId task_buffer_id{{.index = static_cast<u32>(impl.exec_task_buffers.size())}};

        for (auto & permutation : impl.permutations)
        {
            permutation.buffer_infos.push_back(TaskBuffer{
                .valid = permutation.active,
                .info = info,
            });
        }

        impl.exec_task_buffers.push_back(ExecutionTimeTaskBuffer{.actual_buffers = std::vector<BufferId>(info.execution_buffers.begin(), info.execution_buffers.end())});

        return task_buffer_id;
    }

    auto TaskList::create_task_image(TaskImageInfo const & info) -> TaskImageId
    {
        auto & impl = *reinterpret_cast<ImplTaskList *>(this->object);
        DAXA_DBG_ASSERT_TRUE_M(!impl.compiled, "can not record to completed command list");
        TaskImageId task_image_id{{.index = static_cast<u32>(impl.exec_task_images.size())}};

        std::vector<TaskImageTrackedSlice> initial_accesses;
        initial_accesses.reserve(info.initial_access.size());
        for (auto & initial_access : info.initial_access)
        {
            initial_accesses.push_back({
                .latest_access = initial_access.access,
                .latest_layout = initial_access.layout,
                .slice = initial_access.slice,
            });
        }

        for (auto & permutation : impl.permutations)
        {
            // For non-persistent resources task list will synch on the initial to fisrt use every execution.
            permutation.image_infos.push_back(TaskImage{
                .valid = permutation.active,
                .info = info,
                .swapchain_semaphore_waited_upon = false,
                .slices_last_uses = info.execution_persistent ? std::vector<TaskImageTrackedSlice>{} : initial_accesses,
                .first_accesses = {},
            });
            if (info.swapchain_image)
            {
                permutation.swapchain_image = task_image_id;
            }
        }

        impl.exec_task_images.push_back(ExecutionTimeTaskImage{.actual_images = std::vector<ImageId>(info.execution_images.begin(), info.execution_images.end())});
        // For persistent resources, the initial access only applies ONCE.
        // In the FIRST execution task list will synch from initial access to the first use in the permutation.
        // After the first execution it will not.
        if (info.execution_persistent)
        {
            impl.exec_task_images.back().previous_execution_last_slices = std::move(initial_accesses);
        }
        return task_image_id;
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

    void TaskList::conditional(TaskListConditionalInfo const & conditional_info)
    {
        auto & impl = *reinterpret_cast<ImplTaskList *>(this->object);
        DAXA_DBG_ASSERT_TRUE_M(!impl.compiled, "can not record to completed command list");
        bool const already_active = ((impl.record_active_conditional_scopes >> conditional_info.condition_index) & 1u) != 0;
        DAXA_DBG_ASSERT_TRUE_M(!already_active, "can not nest scopes of the same condition in itself.");
        DAXA_DBG_ASSERT_TRUE_M(conditional_info.condition_index < impl.info.permutation_condition_count,
                               "using conditional index " + std::to_string(conditional_info.condition_index) +
                                   " which is larger than the number of conditions specified during task list creation - " + std::to_string(impl.info.permutation_condition_count));
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

    void TaskRuntimeInterface::add_runtime_buffer(TaskBufferId tid, BufferId id)
    {
        auto & impl = *static_cast<ImplTaskRuntimeInterface *>(this->backend);
        impl.task_list.add_runtime_buffer(tid, id);
    }
    void TaskRuntimeInterface::add_runtime_image(TaskImageId tid, ImageId id)
    {
        auto & impl = *static_cast<ImplTaskRuntimeInterface *>(this->backend);
        impl.task_list.add_runtime_image(tid, id);
    }
    void TaskRuntimeInterface::remove_runtime_buffer(TaskBufferId tid, BufferId id)
    {
        auto & impl = *static_cast<ImplTaskRuntimeInterface *>(this->backend);
        impl.task_list.remove_runtime_buffer(tid, id);
    }
    void TaskRuntimeInterface::remove_runtime_image(TaskImageId tid, ImageId id)
    {
        auto & impl = *static_cast<ImplTaskRuntimeInterface *>(this->backend);
        impl.task_list.remove_runtime_image(tid, id);
    }
    void TaskRuntimeInterface::clear_runtime_buffers(TaskBufferId tid)
    {
        auto & impl = *static_cast<ImplTaskRuntimeInterface *>(this->backend);
        impl.task_list.clear_runtime_buffers(tid);
    }
    void TaskRuntimeInterface::clear_runtime_images(TaskImageId tid)
    {
        auto & impl = *static_cast<ImplTaskRuntimeInterface *>(this->backend);
        impl.task_list.clear_runtime_images(tid);
    }

    void ImplTaskList::add_runtime_buffer(TaskBufferId tid, BufferId id)
    {
        exec_task_buffers.at(tid.index).actual_buffers.push_back(id);
    }
    void ImplTaskList::add_runtime_image(TaskImageId tid, ImageId id)
    {
        exec_task_images.at(tid.index).actual_images.push_back(id);
    }
    void ImplTaskList::remove_runtime_buffer(TaskBufferId tid, BufferId id)
    {
        exec_task_buffers[tid.index].actual_buffers.erase(std::remove(exec_task_buffers[tid.index].actual_buffers.begin(), exec_task_buffers[tid.index].actual_buffers.end(), id), exec_task_buffers[tid.index].actual_buffers.end());
    }
    void ImplTaskList::remove_runtime_image(TaskImageId tid, ImageId id)
    {
        exec_task_images[tid.index].actual_images.erase(std::remove(exec_task_images[tid.index].actual_images.begin(), exec_task_images[tid.index].actual_images.end(), id), exec_task_images[tid.index].actual_images.end());
    }
    void ImplTaskList::clear_runtime_buffers(TaskBufferId tid)
    {
        exec_task_buffers.at(tid.index).actual_buffers.clear();
    }
    void ImplTaskList::clear_runtime_images(TaskImageId tid)
    {
        exec_task_images.at(tid.index).actual_images.clear();
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
        TaskListPermutation & perm,
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
            TaskBuffer const & task_buffer = perm.buffer_infos[used_buffer_t_id.index];
            // If the latest access is in a previous submit scope, the earliest batch we can insert into is
            // the current scopes first batch.
            if (task_buffer.latest_access_submit_scope_index < current_submit_scope_index)
            {
                continue;
            }

            Access const current_buffer_access = task_buffer_access_to_access(used_buffer_t_access);
            // Every other access (NONE, READ_WRITE, WRITE) are interpreted as writes in this context.
            bool const is_last_access_read = task_buffer.latest_access.type == AccessTypeFlagBits::READ;
            bool const is_current_access_read = current_buffer_access.type == AccessTypeFlagBits::READ;

            // When a buffer has been read in a previous use AND the current task also reads the buffer,
            // we must insert the task at or after the last use batch.
            usize current_buffer_first_possible_batch_index = task_buffer.latest_access_batch_index;
            // So when not both, the last access and the current access, are reads, we need to insert AFTER the latest access.
            if (!(is_last_access_read && is_current_access_read))
            {
                current_buffer_first_possible_batch_index += 1;
            }
            first_possible_batch_index = std::max(first_possible_batch_index, current_buffer_first_possible_batch_index);
        }
        for (auto const & [used_image_t_id, used_image_t_access, used_image_slice] : info.used_images)
        {
            TaskImage const & task_image = perm.image_infos[used_image_t_id.index];
            DAXA_DBG_ASSERT_TRUE_M(!task_image.swapchain_semaphore_waited_upon, "swapchain image is already presented!");
            if (task_image.info.swapchain_image)
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

            auto [this_task_image_layout, this_task_image_access] = task_image_access_to_layout_access(used_image_t_access);
            // As image subresources can be in different layouts and also different synchronization scopes,
            // we need to track these image ranges individually.
            for (TaskImageTrackedSlice const & tracked_slice : task_image.slices_last_uses)
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

    void TaskList::add_task(TaskInfo const & info)
    {
        auto & impl = *reinterpret_cast<ImplTaskList *>(this->object);
        DAXA_DBG_ASSERT_TRUE_M(!impl.compiled, "can not record to completed command list");

        for (auto permutation : impl.record_active_permutations)
        {
            permutation->add_task(impl, info);
        }
    }

    thread_local std::vector<ImageMipArraySlice> tl_new_access_slices = {};
    void update_image_initial_access_slices(
        TaskImage & task_image,
        TaskImageTrackedSlice new_access_slice)
    {
        // We need to test if a new use adds and or subtracts from initial uses.
        // To do that, we need to test if the new access slice is accessing a subresource BEFORE all other already stored initial access slices.
        // We compare all new use slices with already tracked first uses.
        // We intersect the earlier and later happening slices with the new slice and store the intersection rest or the respective later executing slice access.
        // This list will contain the remainder of the new access ranges after intersections.
        tl_new_access_slices.push_back(new_access_slice.slice);
        // Name shortening:
        auto & initial_accesses = task_image.first_accesses;
        // Note(pahrens):
        // NEVER access new_access_slice.slice in this function past this point.
        // ALWAYS access the new access ImageMipArraySlice's from an iterator or via vector indexing.
        for (size_t new_access_slice_i = 0; new_access_slice_i < tl_new_access_slices.size();)
        {
            bool broke_inner_loop = false;
            for (size_t initial_access_i = 0; initial_access_i < task_image.first_accesses.size();)
            {
                bool const slices_disjoint = !tl_new_access_slices[new_access_slice_i].intersects(initial_accesses[initial_access_i].slice);
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
                // We then replace the current initial accesss slice with the resulting rest.
                if (new_use_executes_earlier)
                {
                    // When we intersect, we remove the old initial access slice and replace it with the rest of the subtraction.
                    // We need a copy of this, as we will erase this value from the vector first.
                    auto const initial_access_slice = initial_accesses[initial_access_i];
                    // Erase value from vector.
                    task_image.first_accesses.erase(initial_accesses.begin() + isize(initial_access_i));
                    // Subtract ranges.
                    auto const [slice_rest, slice_rest_count] = initial_access_slice.slice.subtract(tl_new_access_slices[new_access_slice_i]);
                    // Now construct new subranges from the rest of the subtraction.
                    // We advance the iterator each time.
                    for (usize rest_i = 0; rest_i < slice_rest_count; ++rest_i)
                    {
                        auto rest_tracked_slice = initial_access_slice;
                        rest_tracked_slice.slice = slice_rest[rest_i];
                        // We insert into the beginning, so we dont recheck these with the current new use slice.
                        // They are the result of a subtraction therefore disjoint.
                        task_image.first_accesses.insert(initial_accesses.begin(), rest_tracked_slice);
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
                    auto const [slice_rest, slice_rest_count] = tl_new_access_slices[new_access_slice_i].subtract(initial_accesses[initial_access_i].slice);
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
            new_tracked_slice.slice = new_slice;
            initial_accesses.push_back(new_tracked_slice);
        }
        tl_new_access_slices.clear();
    }

    thread_local std::vector<TaskImageTrackedSlice> tl_tracked_slice_rests = {};
    thread_local std::vector<ImageMipArraySlice> tl_new_use_slices = {};
    void TaskListPermutation::add_task(ImplTaskList & task_list_impl, TaskInfo const & info)
    {
        TaskId const task_id = this->tasks.size();
        this->tasks.emplace_back(Task{
            .info = info,
        });

        usize const current_submit_scope_index = this->batch_submit_scopes.size() - 1;
        TaskBatchSubmitScope & current_submit_scope = this->batch_submit_scopes[current_submit_scope_index];

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
            task_list_impl,
            *this,
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
            TaskBuffer & task_buffer = this->buffer_infos[used_buffer_t_id.index];
            Access const current_buffer_access = task_buffer_access_to_access(used_buffer_t_access);
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
                    // Host access needs to be handeled in a specialized way.
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
                                .debug_name = std::string("TaskList \"") + task_list_impl.info.debug_name + "\" SplitBarrier Nr. " + std::to_string(split_barrier_index),
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
        }
        // Now we insert image dependent sync
        for (auto const & [used_image_t_id, used_image_t_access, initial_used_image_slice] : info.used_images)
        {
            TaskImage & task_image = this->image_infos[used_image_t_id.index];
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
            // We update the initial access slices.
            update_image_initial_access_slices(task_image, ret_new_use_tracked_slice);
            // As image subresources can be in different layouts and also different synchronization scopes,
            // we need to track these image ranges individually.
            for (
                auto tracked_slice_iter = task_image.slices_last_uses.begin();
                tracked_slice_iter != task_image.slices_last_uses.end();)
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
                    tracked_slice_iter = task_image.slices_last_uses.erase(tracked_slice_iter);
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
                            auto & last_read_split_barrier = this->split_barriers[index0->index];
                            last_read_split_barrier.dst_access = last_read_split_barrier.dst_access | tracked_slice.latest_access;
                        }
                        else if (LastReadBarrierIndex const * index1 = std::get_if<LastReadBarrierIndex>(&tracked_slice.latest_access_read_barrier_index))
                        {
                            auto & last_read_barrier = this->barriers[index1->index];
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
                            usize const split_barrier_index = this->split_barriers.size();
                            this->split_barriers.push_back(TaskSplitBarrier{
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
                                /* .split_barrier_state = */ task_list_impl.info.device.create_split_barrier({
                                    .debug_name = std::string("TaskList \"") + task_list_impl.info.debug_name + "\" SplitBarrier (Image) Nr. " + std::to_string(split_barrier_index),
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
                    if (tracked_slice_iter == task_image.slices_last_uses.end())
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
            task_image.slices_last_uses.push_back(ret_new_use_tracked_slice);
            // The remainder tracked slices we remembered from earlier are now inserted back into the list of tracked slices.
            // We deferred this step as we dont want to check these in the loop above, as we found them to not intersect with the new use.
            task_image.slices_last_uses.insert(
                task_image.slices_last_uses.end(),
                tl_tracked_slice_rests.begin(),
                tl_tracked_slice_rests.end());
            tl_tracked_slice_rests.clear();
        }
    }

    void TaskList::submit(CommandSubmitInfo * info)
    {
        auto & impl = *as<ImplTaskList>();
        DAXA_DBG_ASSERT_TRUE_M(!impl.compiled, "Can only record to an uncompleted task list");

        for (auto & permutation : impl.record_active_permutations)
        {
            permutation->submit(info);
        }
    }

    void TaskListPermutation::submit(CommandSubmitInfo * info)
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
        DAXA_DBG_ASSERT_TRUE_M(!impl.compiled, "Can only record to an uncompleted task list");
        DAXA_DBG_ASSERT_TRUE_M(impl.info.swapchain.has_value(), "Can only present, when a swapchain was provided in creation");

        for (auto & permutation : impl.record_active_permutations)
        {
            permutation->present(info);
        }
    }

    void TaskListPermutation::present(TaskPresentInfo const & info)
    {
        DAXA_DBG_ASSERT_TRUE_M(this->batch_submit_scopes.size() > 1, "Can only present if at least one submit was issued before");
        DAXA_DBG_ASSERT_TRUE_M(!this->swapchain_image.is_empty(), "Can only present when an image was annotated as swapchain image");
        DAXA_DBG_ASSERT_TRUE_M(!this->image_infos[this->swapchain_image.index].swapchain_semaphore_waited_upon, "Can only present once");
        this->image_infos[this->swapchain_image.index].swapchain_semaphore_waited_upon = true;

        TaskImageTrackedSlice const & tracked_slice = this->image_infos[this->swapchain_image.index].slices_last_uses.back();
        usize submit_scope_index = tracked_slice.latest_access_submit_scope_index;
        DAXA_DBG_ASSERT_TRUE_M(submit_scope_index < this->batch_submit_scopes.size() - 1, "the last swapchain image use MUST be before the last submit when presenting");
        TaskBatchSubmitScope & submit_scope = this->batch_submit_scopes[submit_scope_index];
        usize const batch_index = tracked_slice.latest_access_batch_index;
        // We need to insert a pipeline barrier to transition the swapchain image layout to present src optimal.
        usize const barrier_index = this->barriers.size();
        this->barriers.push_back(TaskBarrier{
            .image_id = this->swapchain_image,
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

        // Insert static barriers initializing image layouts.
        // TODO(pahrens): when we respect the create info prior use we MUST also put in barriers for buffer uses!
        for (auto & permutation : impl.permutations)
        {
            auto & first_batch = permutation.batch_submit_scopes[0].task_batches[0];
            // Insert static initialization barriers for non persistent resources:
            // Buffers never need layout initialization, only images.
            for (u32 task_image_index = 0; task_image_index < permutation.image_infos.size(); ++task_image_index)
            {
                TaskImageId task_image_id = {task_image_index};
                auto & task_image = permutation.image_infos[task_image_index];
                if (task_image.valid && !task_image.info.execution_persistent)
                {
                    // Insert barriers, initializing all the initially accesses subresource ranges to the correct layout.
                    for (auto & initial_access : task_image.first_accesses)
                    {
                        // TODO(pahrens): Respect the given prior state from the info.
                        // TODO(pahrens): Investigate if it makes sense to insert these barriers in the batch in which the initial use appers.
                        // TODO(pahrens): Investigate if split barriers make sense here.
                        usize new_barrier_index = permutation.barriers.size();
                        permutation.barriers.push_back(TaskBarrier{
                            .image_id = task_image_id,
                            .slice = initial_access.slice,
                            .layout_before = {},
                            .layout_after = initial_access.latest_layout,
                            .src_access = {},
                            .dst_access = initial_access.latest_access,
                            .src_batch = {},
                            .dst_batch = initial_access.latest_access_batch_index,
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

    void TaskList::output_graphviz()
    {
        auto & impl = *as<ImplTaskList>();
        DAXA_DBG_ASSERT_TRUE_M(impl.compiled, "must compile before getting graphviz");
        impl.output_graphviz();
    }

    thread_local std::vector<SplitBarrierWaitInfo> tl_split_barrier_wait_infos = {};
    thread_local std::vector<ImageBarrierInfo> tl_image_barrier_infos = {};
    thread_local std::vector<MemoryBarrierInfo> tl_memory_barrier_infos = {};
    void insert_pipeline_barrier(CommandList & command_list, TaskBarrier & barrier, std::vector<daxa::ExecutionTimeTaskImage> & execution_images)
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
            for (auto & image : execution_images[barrier.image_id.index].actual_images)
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

    void generate_persistent_resource_synch(
        ImplTaskList & impl,
        TaskListPermutation & permutation,
        CommandList & cmd_list)
    {
        // Persistent resources need just in time synch between executions,
        // as pre generating the transitions between all permutations is not managable.
        DAXA_ONLY_IF_TASK_LIST_DEBUG(std::cout << "\tBegin persistent resource synchronization memory barriers\n");
        for (usize task_buffer_index = 0; task_buffer_index < permutation.buffer_infos.size(); ++task_buffer_index)
        {
            auto & task_buffer = permutation.buffer_infos[task_buffer_index];
            auto & exec_buffer = impl.exec_task_buffers[task_buffer_index];
            if (task_buffer.valid && task_buffer.info.execution_persistent && exec_buffer.previous_execution_last_access.has_value())
            {
                MemoryBarrierInfo mem_barrier_info{
                    .awaited_pipeline_access = exec_buffer.previous_execution_last_access.value(),
                    .waiting_pipeline_access = permutation.buffer_infos[task_buffer_index].initial_access,
                };
                cmd_list.pipeline_barrier(mem_barrier_info);
                DAXA_ONLY_IF_TASK_LIST_DEBUG(impl.debug_print_memory_barrier(mem_barrier_info, "\t"));
                exec_buffer.previous_execution_last_access = {};
            }
        }
        DAXA_ONLY_IF_TASK_LIST_DEBUG(std::cout << "\tEnd persistent resource synchronization memory barriers\n");
        DAXA_ONLY_IF_TASK_LIST_DEBUG(std::cout << "\tBegin persistent image synchronization image memory barriers\n");
        // If parts of the first use slices to not intersect with any previous use,
        // we must synchronize on undefined layout!
        std::vector<TaskImageTrackedSlice> remaining_first_accesses = {};
        for (usize task_image_index = 0; task_image_index < permutation.image_infos.size(); ++task_image_index)
        {
            auto & task_image = permutation.image_infos[task_image_index];
            auto & exec_image = impl.exec_task_images[task_image_index];
            remaining_first_accesses = task_image.first_accesses;
            // Iterate over all persistent images.
            // Find all intersections between tracked slices of first use and previous use.
            // Synch on the intersection and delete the intersected part from the tracked slice of the previous use.
            if (task_image.valid && task_image.info.execution_persistent && exec_image.previous_execution_last_slices.has_value())
            {
                auto & previous_access_slices = exec_image.previous_execution_last_slices.value();
                for (usize previous_access_slice_index = 0; previous_access_slice_index < previous_access_slices.size();)
                {
                    bool broke_inner_loop = false;
                    for (usize first_access_slice_index = 0; first_access_slice_index < remaining_first_accesses.size(); ++first_access_slice_index)
                    {
                        // Dont sync on same accesses following each other.
                        // Dont sync on disjoint subresource uses.
                        bool const both_accesses_read =
                            remaining_first_accesses[first_access_slice_index].latest_access.type == AccessTypeFlagBits::READ &&
                            previous_access_slices[previous_access_slice_index].latest_access.type == AccessTypeFlagBits::READ;
                        bool const both_layouts_same =
                            remaining_first_accesses[first_access_slice_index].latest_layout ==
                            previous_access_slices[previous_access_slice_index].latest_layout;
                        if (!remaining_first_accesses[first_access_slice_index].slice.intersects(previous_access_slices[previous_access_slice_index].slice) ||
                            (both_accesses_read && both_layouts_same))
                        {
                            // Disjoint subresources or read on read with same layout.
                            continue;
                        }
                        // Intersect previous use and initial use.
                        // Record synchronization for the intersecting part.
                        auto intersection = previous_access_slices[previous_access_slice_index].slice.intersect(remaining_first_accesses[first_access_slice_index].slice);
                        for (auto execution_image_id : impl.exec_task_images[task_image_index].actual_images)
                        {
                            ImageBarrierInfo img_barrier_info{
                                .awaited_pipeline_access = previous_access_slices[previous_access_slice_index].latest_access,
                                .waiting_pipeline_access = remaining_first_accesses[first_access_slice_index].latest_access,
                                .before_layout = previous_access_slices[previous_access_slice_index].latest_layout,
                                .after_layout = remaining_first_accesses[first_access_slice_index].latest_layout,
                                .image_slice = intersection,
                                .image_id = execution_image_id,
                            };
                            cmd_list.pipeline_barrier_image_transition(img_barrier_info);
                            DAXA_ONLY_IF_TASK_LIST_DEBUG(impl.debug_print_image_memory_barrier(img_barrier_info, task_image, "\t"));
                        }
                        // Put back the non intersecting rest into the previous use list.
                        auto [previous_use_slice_rest, previous_use_slice_rest_count] = previous_access_slices[previous_access_slice_index].slice.subtract(remaining_first_accesses[first_access_slice_index].slice);
                        auto [first_use_slice_rest, first_use_slice_rest_count] = remaining_first_accesses[first_access_slice_index].slice.subtract(previous_access_slices[previous_access_slice_index].slice);
                        for (usize rest_slice_index = 0; rest_slice_index < previous_use_slice_rest_count; ++rest_slice_index)
                        {
                            auto rest_previous_slice = previous_access_slices[previous_access_slice_index];
                            rest_previous_slice.slice = previous_use_slice_rest[rest_slice_index];
                            previous_access_slices.push_back(rest_previous_slice);
                        }
                        // Append the new rest first uses.
                        for (usize rest_slice_index = 0; rest_slice_index < previous_use_slice_rest_count; ++rest_slice_index)
                        {
                            auto rest_previous_slice = remaining_first_accesses[first_access_slice_index];
                            rest_previous_slice.slice = first_use_slice_rest[rest_slice_index];
                            remaining_first_accesses.push_back(rest_previous_slice);
                        }
                        // Remove the previous use from the list, it is synchronized now.
                        previous_access_slices.erase(std::next(previous_access_slices.begin(), static_cast<ptrdiff_t>(previous_access_slice_index)));
                        // Remove the first use from the remaining first uses, as it was now synchronized from.
                        remaining_first_accesses.erase(std::next(remaining_first_accesses.begin(), static_cast<ptrdiff_t>(first_access_slice_index)));
                        // As we removed an element in this place,
                        // we dont need to advance the iterator as in its place there will be a new element alreay that we do not want to skip.
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
                // For all first uses that did NOT intersect with and previous use,
                // we need to syncrhonize from an undefined state to initialize the layout of the image. 
                for (usize remaining_first_uses_index = 0; remaining_first_uses_index < remaining_first_accesses.size(); ++remaining_first_uses_index)
                {
                    for (auto execution_image_id : impl.exec_task_images[task_image_index].actual_images)
                    {
                        ImageBarrierInfo img_barrier_info{
                            .awaited_pipeline_access = AccessConsts::NONE,
                            .waiting_pipeline_access = remaining_first_accesses[remaining_first_uses_index].latest_access,
                            .before_layout = ImageLayout::UNDEFINED,
                            .after_layout = remaining_first_accesses[remaining_first_uses_index].latest_layout,
                            .image_slice = remaining_first_accesses[remaining_first_uses_index].slice,
                            .image_id = execution_image_id,
                        };
                        cmd_list.pipeline_barrier_image_transition(img_barrier_info);
                        DAXA_ONLY_IF_TASK_LIST_DEBUG(impl.debug_print_image_memory_barrier(img_barrier_info, task_image, "\t"));
                    }
                }
            }
        }
        DAXA_ONLY_IF_TASK_LIST_DEBUG(std::cout << "\tEnd persistent image synchronization image memory barriers\n");
    }

    void TaskList::execute(ExecutionInfo const & info)
    {
        auto & impl = *as<ImplTaskList>();
        DAXA_DBG_ASSERT_TRUE_M(info.permutation_condition_values.size() >= impl.info.permutation_condition_count, "must provide all permutation conditions in execution!");
        DAXA_DBG_ASSERT_TRUE_M(impl.compiled, "must compile before executing");

        u32 permutation_index = {};
        for (u32 index = 0; index < std::min(usize(32), info.permutation_condition_values.size()); ++index)
        {
            permutation_index |= info.permutation_condition_values[index] ? (1u << index) : 0;
        }
        impl.chosen_permutation_last_execution = permutation_index;
        TaskListPermutation & permutation = impl.permutations[permutation_index];

        ImplTaskRuntimeInterface impl_runtime{.task_list = impl, .permutation = permutation};
        impl_runtime.command_lists.push_back(impl.info.device.create_command_list({.debug_name = std::string("Task Command List ") + std::to_string(impl_runtime.command_lists.size())}));

        // Generate and insert synchronization for persistent resources:
        generate_persistent_resource_synch(impl, permutation, impl_runtime.command_lists.back());

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
        for (auto & submit_scope : permutation.batch_submit_scopes)
        {
            for (auto & task_batch : submit_scope.task_batches)
            {
                // Wait on pipeline barriers before batch execution.
                for (auto barrier_index : task_batch.pipeline_barrier_indices)
                {
                    TaskBarrier & barrier = permutation.barriers[barrier_index];
                    insert_pipeline_barrier(impl_runtime.command_lists.back(), barrier, impl.exec_task_images);
                }
                // Wait on split barriers before batch execution.
                if (!impl.info.use_split_barriers)
                {
                    for (auto barrier_index : task_batch.wait_split_barrier_indices)
                    {
                        TaskSplitBarrier const & split_barrier = permutation.split_barriers[barrier_index];
                        // Convert split barrier to normal barrier.
                        TaskBarrier barrier = split_barrier;
                        insert_pipeline_barrier(impl_runtime.command_lists.back(), barrier, impl.exec_task_images);
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
                            needed_image_barriers += impl.exec_task_images[split_barrier.image_id.index].actual_images.size();
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
                            for (auto image : impl.exec_task_images[split_barrier.image_id.index].actual_images)
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
                    Task & task = permutation.tasks[task_id];
                    impl_runtime.current_task = &task;
                    task.info.task(TaskRuntimeInterface(&impl_runtime));
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
                            for (auto image : impl.exec_task_images[task_split_barrier.image_id.index].actual_images)
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
                TaskBarrier & barrier = permutation.barriers[barrier_index];
                insert_pipeline_barrier(impl_runtime.command_lists.back(), barrier, impl.exec_task_images);
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

        // Insert pervious uses into execution info for tje next executions synch.
        for (usize task_buffer_index = 0; task_buffer_index < permutation.buffer_infos.size(); ++task_buffer_index)
        {
            if (permutation.buffer_infos[task_buffer_index].valid && permutation.buffer_infos[task_buffer_index].info.execution_persistent)
            {
                impl.exec_task_buffers[task_buffer_index].previous_execution_last_access = permutation.buffer_infos[task_buffer_index].latest_access;
            }
        }
        for (usize task_image_index = 0; task_image_index < permutation.image_infos.size(); ++task_image_index)
        {
            if (permutation.image_infos[task_image_index].valid && permutation.image_infos[task_image_index].info.execution_persistent)
            {
                if (!impl.exec_task_images[task_image_index].previous_execution_last_slices.has_value())
                {
                    impl.exec_task_images[task_image_index].previous_execution_last_slices = std::vector<TaskImageTrackedSlice>{};
                }
                impl.exec_task_images[task_image_index].previous_execution_last_slices.value().insert(
                    impl.exec_task_images[task_image_index].previous_execution_last_slices.value().end(),
                    permutation.image_infos[task_image_index].slices_last_uses.begin(),
                    permutation.image_infos[task_image_index].slices_last_uses.end());
            }
        }

        impl.left_over_command_lists = std::move(impl_runtime.command_lists);
        impl.executed_once = true;
        impl.prev_frame_permutation_index = permutation_index;
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
        for (auto const & permutation : this->permutations)
            for (auto const & scope : permutation.batch_submit_scopes)
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
                            auto const & barrier = permutation.barriers[barrier_index];
                            std::string const name = batch_name + std::string("_pb_") + std::to_string(barrier.src_batch) + std::string("_") + std::to_string(barrier.dst_batch);
                            dot_file << "node_" << name << " [label=\"" << name << "\", shape=box, color=\"#faaff0\"]\n";
                        }
                        for (auto const & barrier_index : batch.wait_split_barrier_indices)
                        {
                            auto const & barrier = permutation.split_barriers[barrier_index];
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
                            auto task_debug_name = permutation.tasks[task_id].info.debug_name;
                            dot_file << "subgraph cluster_" << task_name << " {\n"
                                     << "label=\"" << task_debug_name << "\"\n"
                                     << "style=filled\ncolor=\"#d1e2ed\"\n";
                            // dot_file << "node_" << task_name << " [label=\"" << task_debug_name << "\", shape=box]\n";
                            usize resource_index = 0;

                            resource_index = 0;
                            for (auto const & [task_buffer_id, task_buffer_access] : permutation.tasks[task_id].info.used_buffers)
                            {
                                TaskBuffer const & task_resource = permutation.buffer_infos[task_buffer_id.index];
                                auto const & resource_debug_name = task_resource.info.debug_name;
                                dot_file << "node_" << task_name << "_br" << resource_index << " [label=\"" << resource_debug_name << "\", shape=box, color=\"#d3fabe\"]\n";
                                ++resource_index;
                            }

                            resource_index = 0;
                            for (auto const & [task_image_id, task_buffer_access, image_slice] : permutation.tasks[task_id].info.used_images)
                            {
                                auto const & task_resource = permutation.image_infos[task_image_id.index];
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
                            auto const & barrier = permutation.split_barriers[barrier_index];
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

    void ImplTaskList::debug_print_memory_barrier(MemoryBarrierInfo & barrier, std::string_view prefix)
    {
        std::cout << prefix << "Begin Memory barrier\n";
        std::cout << prefix << "\t.awaited_pipeline_access = " << to_string(barrier.awaited_pipeline_access) << "\n";
        std::cout << prefix << "\t.waiting_pipeline_access = " << to_string(barrier.waiting_pipeline_access) << "\n";
        std::cout << prefix << "End   Memory barrier\n";
    }

    void ImplTaskList::debug_print_image_memory_barrier(ImageBarrierInfo & barrier, TaskImage & task_image, std::string_view prefix)
    {
        std::cout << prefix << "Begin image memory barrier\n";
        std::cout << prefix << "\ttask_image_id: " << barrier.image_id.index << " \n";
        std::cout << prefix << "\ttask image debug name: " << task_image.info.debug_name << " \n";
        std::cout << prefix << "\tBegin bound images\n";
        std::cout << prefix << "\timage id: " << to_string(barrier.image_id)
                  << "\timage debug name: " << info.device.info_image(barrier.image_id).debug_name << " \n";
        std::cout << prefix << "\tEnd   bound images \n";
        std::cout << prefix << "\tsrc access: " << to_string(barrier.awaited_pipeline_access) << "\n";
        std::cout << prefix << "\tdst access: " << to_string(barrier.waiting_pipeline_access) << "\n";
        std::cout << prefix << "\timage mip array slice: " << to_string(barrier.image_slice) << "\n";
        std::cout << prefix << "\tbefore layout: " << to_string(barrier.before_layout) << "\n";
        std::cout << prefix << "\tafter layout: " << to_string(barrier.after_layout) << "\n";
        std::cout << prefix << "End   image memory barrier\n";
    }

    void ImplTaskList::debug_print_task_barrier(TaskListPermutation const & permutation, TaskBarrier & barrier, usize index, std::string_view prefix)
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
            TaskImage const & task_image = permutation.image_infos[barrier.image_id.index];
            std::cout << prefix << "Begin image memory barrier\n";
            std::cout << prefix << "\tbarrier index: " << index << "\n";
            std::cout << prefix << "\ttask_image_id: " << barrier.image_id.index << " \n";
            std::cout << prefix << "\ttask image debug name: " << task_image.info.debug_name << " \n";
            std::cout << prefix << "\tBegin bound images\n";
            for (auto image : this->exec_task_images[barrier.image_id.index].actual_images)
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

    void ImplTaskList::debug_print_task_split_barrier(TaskListPermutation const & permutation, TaskSplitBarrier & barrier, usize index, std::string_view prefix)
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
            TaskImage const & task_image = permutation.image_infos[barrier.image_id.index];
            std::cout << prefix << "Begin image memory barrier\n";
            std::cout << prefix << "\tbarrier index: " << index << "\n";
            std::cout << prefix << "\ttask_image_id: " << barrier.image_id.index << " \n";
            std::cout << prefix << "\ttask image debug name: " << task_image.info.debug_name << " \n";
            std::cout << prefix << "\tBegin bound images\n";
            for (auto image : this->exec_task_images[barrier.image_id.index].actual_images)
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

    void ImplTaskList::debug_print_task(TaskListPermutation const & permutation, Task & task, usize task_id, std::string_view prefix)
    {
#ifdef DAXA_TASK_LIST_DEBUG
        std::cout << prefix << "Begin task " << task_id << " name: \"" << task.info.debug_name << "\"\n";
        for (auto [task_image_id, task_image_access, slice] : task.info.used_images)
        {
            TaskImage const & task_image = permutation.image_infos[task_image_id.index];
            auto [layout, access] = task_image_access_to_layout_access(task_image_access);
            std::cout << prefix << "\tBegin task image use " << task_image_id.index << "\n";
            std::cout << prefix << "\ttask_image_id: " << task_image_id.index << " \n";
            std::cout << prefix << "\ttask image debug name: " << task_image.info.debug_name << " \n";
            std::cout << prefix << "\tBegin bound images\n";
            for (auto image : exec_task_images[task_image_id.index].actual_images)
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
            TaskBuffer const & task_buffer = permutation.buffer_infos[task_buffer_id.index];
            auto access = task_buffer_access_to_access(task_buffer_access);
            std::cout << prefix << "\tBegin task buffer use " << task_buffer_id.index << "\n";
            std::cout << prefix << "\t\task buffer debug name: " << task_buffer.info.debug_name << "\n";
            std::cout << prefix << "\tBegin bound buffers\n";
            for (auto buffer : exec_task_buffers[task_buffer_id.index].actual_buffers)
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
        std::cout << "\tReorder tasks: " << std::boolalpha << impl.info.reorder_tasks << "\n";
        std::cout << "\tUse split barriers: " << std::boolalpha << impl.info.use_split_barriers << "\n";

        usize permutation_index = impl.chosen_permutation_last_execution;
        auto & permutation = impl.permutations[permutation_index];
        {
            permutation_index += 1;
            std::cout << "\tBegin Permutation Nr. " << permutation_index << "\n";
            usize submit_scope_index = 0;
            for (auto & submit_scope : permutation.batch_submit_scopes)
            {
                std::cout << "\t\tBegin submit scope " << submit_scope_index++ << "\n";
                usize batch_index = 0;
                for (auto & task_batch : submit_scope.task_batches)
                {
                    std::cout << "\t\t\tBegin task batch " << batch_index++ << "\n";
                    // Wait on pipeline barriers before batch execution.
                    std::cout << "\t\t\t\tBegin wait pipeline barriers\n";
                    for (auto barrier_index : task_batch.pipeline_barrier_indices)
                    {
                        impl.debug_print_task_barrier(permutation, permutation.barriers[barrier_index], barrier_index, "\t\t\t\t\t");
                    }
                    std::cout << "\t\t\t\tEnd   wait pipeline barriers\n";
                    if (!impl.info.use_split_barriers)
                    {
                        std::cout << "\t\t\t\tBegin wait split barriers (converted to pipeline barriers)\n";
                        for (auto barrier_index : task_batch.wait_split_barrier_indices)
                        {
                            impl.debug_print_task_barrier(permutation, permutation.split_barriers[barrier_index], barrier_index, "\t\t\t\t\t");
                        }
                        std::cout << "\t\t\t\tEnd   wait split barriers (converted to pipeline barriers)\n";
                    }
                    else
                    {
                        std::cout << "\t\t\t\tBegin wait split barriers\n";
                        for (auto barrier_index : task_batch.wait_split_barrier_indices)
                        {
                            impl.debug_print_task_split_barrier(permutation, permutation.split_barriers[barrier_index], barrier_index, "\t\t\t\t\t");
                        }
                        std::cout << "\t\t\t\tEnd   wait split barriers\n";
                    }
                    std::cout << "\t\t\t\tBegin tasks\n";
                    for (TaskId const task_id : task_batch.tasks)
                    {
                        Task & task = permutation.tasks[task_id];
                        impl.debug_print_task(permutation, task, task_id, "\t\t\t\t\t");
                    }
                    std::cout << "\t\t\t\tEnd   tasks\n";
                    if (impl.info.use_split_barriers)
                    {
                        std::cout << "\t\t\t\tBegin signal split barriers\n";
                        for (usize const barrier_index : task_batch.signal_split_barrier_indices)
                        {
                            impl.debug_print_task_split_barrier(permutation, permutation.split_barriers[barrier_index], barrier_index, "\t\t\t\t\t");
                        }
                        std::cout << "\t\t\t\tEnd   signal split barriers\n";
                    }
                    std::cout << "\t\t\tEnd   task batch\n";
                }
                std::cout << "\t\t\tBegin last minute pipeline barriers\n";
                for (usize const barrier_index : submit_scope.last_minute_barrier_indices)
                {
                    impl.debug_print_task_barrier(permutation, permutation.barriers[barrier_index], barrier_index, "\t\t\t\t");
                }
                std::cout << "\t\t\tEnd   last minute pipeline barriers\n";
                if (&submit_scope != &permutation.batch_submit_scopes.back())
                {
                    std::cout << "\t\t\t<<Submit>>\n";
                    if (submit_scope.present_info.has_value())
                    {
                        std::cout << "\t\t\t<<Present>>\n";
                    }
                }
                std::cout << "\t\tEnd   submit scope\n";
            }
            std::cout << "\tEnd Permutation Nr. " << permutation_index << "\n";
        }
        std::cout << "End TaskList\n";
        std::cout.flush();
#endif // #ifdef DAXA_TASK_LIST_DEBUG
    }
} // namespace daxa

#endif
