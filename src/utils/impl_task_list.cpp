#if DAXA_BUILT_WITH_UTILS

#include "impl_task_list.hpp"
#include <iostream>

#include <fstream>

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
        case TaskImageAccess::DEPTH_ATTACHMENT: return {ImageLayout::ATTACHMENT_OPTIMAL, {PipelineStageFlagBits::EARLY_FRAGMENT_TESTS | PipelineStageFlagBits::LATE_FRAGMENT_TESTS, AccessTypeFlagBits::READ_WRITE}};
        case TaskImageAccess::STENCIL_ATTACHMENT: return {ImageLayout::ATTACHMENT_OPTIMAL, {PipelineStageFlagBits::EARLY_FRAGMENT_TESTS | PipelineStageFlagBits::LATE_FRAGMENT_TESTS, AccessTypeFlagBits::READ_WRITE}};
        case TaskImageAccess::DEPTH_STENCIL_ATTACHMENT: return {ImageLayout::ATTACHMENT_OPTIMAL, {PipelineStageFlagBits::EARLY_FRAGMENT_TESTS | PipelineStageFlagBits::LATE_FRAGMENT_TESTS, AccessTypeFlagBits::READ_WRITE}};
        case TaskImageAccess::DEPTH_ATTACHMENT_READ_ONLY: return {ImageLayout::READ_ONLY_OPTIMAL, {PipelineStageFlagBits::EARLY_FRAGMENT_TESTS | PipelineStageFlagBits::LATE_FRAGMENT_TESTS, AccessTypeFlagBits::READ}};
        case TaskImageAccess::STENCIL_ATTACHMENT_READ_ONLY: return {ImageLayout::READ_ONLY_OPTIMAL, {PipelineStageFlagBits::EARLY_FRAGMENT_TESTS | PipelineStageFlagBits::LATE_FRAGMENT_TESTS, AccessTypeFlagBits::READ}};
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

    TaskInterface::TaskInterface(void * backend, TaskUsedBuffers * used_task_buffers, TaskUsedImages * used_task_images)
        : backend{backend}, used_task_buffers{used_task_buffers}, used_task_images{used_task_images}
    {
    }

    auto TaskInterface::get_device() -> Device &
    {
        auto & impl = *reinterpret_cast<TaskRuntime *>(backend);
        return impl.current_device;
    }

    auto TaskInterface::get_command_list() -> CommandList
    {
        auto & impl = *reinterpret_cast<TaskRuntime *>(backend);
        if (impl.reuse_last_command_list)
        {
            impl.reuse_last_command_list = false;
            return impl.command_lists.back();
        }
        else
        {
            impl.command_lists.push_back({impl.current_device.create_command_list({.debug_name = std::string("Task Command List ") + std::to_string(impl.command_lists.size())})});
            return impl.command_lists.back();
        }
    }

    auto TaskInterface::get_used_task_buffers() -> TaskUsedBuffers &
    {
        return *used_task_buffers;
    }

    auto TaskInterface::get_used_task_images() -> TaskUsedImages &
    {
        return *used_task_images;
    }

    auto TaskInterface::get_buffer(TaskBufferId const & task_id) -> BufferId
    {
        auto & impl = *reinterpret_cast<TaskRuntime *>(backend);
        // TODO(pahrens): IMPLEMENT THIS!
        return {};
    }

    auto TaskInterface::get_image(TaskImageId const & task_id) -> ImageId
    {
        auto & impl = *reinterpret_cast<TaskRuntime *>(backend);
        // TODO(pahrens): IMPLEMENT THIS!
        return {};
    }

    TaskList::TaskList(TaskListInfo const & info)
        : ManagedPtr{new ImplTaskList(info)}
    {
        auto & impl = *reinterpret_cast<ImplTaskList *>(this->object);
        impl.batch_submit_scopes.push_back({});
    }

    TaskList::~TaskList() {}

    auto TaskList::create_task_buffer(TaskBufferInfo const & info) -> TaskBufferId
    {
        // TODO(pahrens): IMPLEMENT THIS
        return {};
    }

    auto TaskList::create_task_image(TaskImageInfo const & info) -> TaskImageId
    {
        // TODO(pahrens): IMPLEMENT THIS
        return {};
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
                    std::get<0>(info.used_buffers[current_i]).index != std::get<0>(info.used_buffers[other_i]).index,
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
                auto const & [current_image_t_id, current_image_t_access, current_image_slice] = info.used_images[current_i];
                auto const & [other_image_t_id, other_image_t_access, other_image_slice] = info.used_images[other_i];

                DAXA_DBG_ASSERT_TRUE_M(
                    !current_image_slice.intersects(other_image_slice),
                    "illegal to specify multiple uses for one image with overlapping slices for one task.");
            }
        }
    }

    auto find_first_possible_batch_index(
        ImplTaskList const & impl,
        TaskBatchSubmitScope const & current_submit_scope,
        usize const current_submit_scope_index,
        TaskInfo const & info) -> usize
    {
        usize first_possible_batch_index = 0;
        if (impl.info.dont_reorder_tasks)
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

            Access current_buffer_access = task_buffer_access_to_access(used_buffer_t_access);
            // Every other access (NONE, READ_WRITE, WRITE) are interpreted as writes in this context.
            bool is_last_access_read = impl_task_buffer.latest_access.type == AccessTypeFlagBits::READ;
            bool is_current_access_read = current_buffer_access.type == AccessTypeFlagBits::READ;

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
                bool is_last_access_read = tracked_slice.latest_access.type == AccessTypeFlagBits::READ;
                bool is_current_access_read = this_task_image_access.type == AccessTypeFlagBits::READ;
                // When the image layouts differ, we must do a layout transition between reads.
                // This forces us to place the task into a batch AFTER the tracked uses last batch.
                bool is_layout_identical = this_task_image_layout == tracked_slice.latest_layout;
                usize current_buffer_first_possible_batch_index = tracked_slice.latest_access_batch_index;
                // If either the image layouts differ, or not both accesses are reads, we must place the task in a later batch.
                if (!(is_last_access_read && is_current_access_read && is_layout_identical))
                {
                    current_buffer_first_possible_batch_index += 1;
                }
                first_possible_batch_index = std::max(first_possible_batch_index, tracked_slice.latest_access_batch_index);
            }
        }
        return first_possible_batch_index;
    }

    thread_local std::vector<TaskImageTrackedSlice> tl_tracked_slice_rests = {};
    thread_local std::vector<ImageMipArraySlice> tl_new_use_slices = {};
    void TaskList::add_task(TaskInfo const & info)
    {
        auto & impl = *reinterpret_cast<ImplTaskList *>(this->object);
        DAXA_DBG_ASSERT_TRUE_M(!impl.compiled, "can not record to completed command list");

        TaskId task_id = impl.tasks.size();
        impl.tasks.emplace_back(GenericTask{
            .info = info,
        });

        usize current_submit_scope_index = impl.batch_submit_scopes.size() - 1;
        TaskBatchSubmitScope & current_submit_scope = impl.batch_submit_scopes[current_submit_scope_index];

        // All tasks are reordered while recording.
        // Tasks are grouped into "task batches" wich are just a group of tasks,
        // that can execute together overlappingly without synchronization between them.
        // Task batches are furthor grouped into submit scopes.
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

        // Now that we know what batch we need to insert the task into, we need to add synchronization between batches.
        // As stated earlier batches are groups of tasks wich can execute together without sync between them.
        // To simplyfy and optimize the sync placement daxa only synchronizes between batches.
        // This effectively means that all the resource uses, and their memory and execution dependencies in a batch
        // are combined into a single unit wich is synchronized against other batches.
        for (auto & [used_buffer_t_id, used_buffer_t_access] : info.used_buffers)
        {
            ImplTaskBuffer & impl_task_buffer = impl.impl_task_buffers[used_buffer_t_id.index];
            Access current_buffer_access = task_buffer_access_to_access(used_buffer_t_access);
            // Every other access (NONE, READ_WRITE, WRITE) are interpreted as writes in this context.
            // When the last use was a read AND the new use of the buffer is a read AND,
            // we need to add our stage flags to the existing barrier of the last use.
            bool is_last_access_read = impl_task_buffer.latest_access.type == AccessTypeFlagBits::READ;
            bool is_current_access_read = current_buffer_access.type == AccessTypeFlagBits::READ;
            if (is_last_access_read && is_current_access_read)
            {
                auto & last_read_split_barrier = impl.split_barriers[impl_task_buffer.latest_access_read_barrier_index];
                last_read_split_barrier.dst_access = last_read_split_barrier.dst_access | current_buffer_access;
            }
            else
            {
                // When the uses are incompatible (no read on read) we need to insert a new barrier.
                usize split_barrier_index = impl.split_barriers.size();
                impl.split_barriers.push_back(TaskSplitBarrier{
                    .split_barrier_state = impl.info.device.create_split_barrier({
                        .debug_name = std::string("TaskList \"") +
                                      impl.info.debug_name +
                                      "\" SplitBarrier Nr. " +
                                      std::to_string(split_barrier_index),
                    }),
                    .image_id = {}, // {} signals that this is not an image barrier.
                    .src_access = impl_task_buffer.latest_access,
                    .dst_access = current_buffer_access,
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
                    impl_task_buffer.latest_access_read_barrier_index = split_barrier_index;
                }
            }
            // Now that we inserted/updated the synchronization, we update the latest access.
            impl_task_buffer.latest_access = current_buffer_access;
            impl_task_buffer.latest_access_batch_index = batch_index;
            impl_task_buffer.latest_access_submit_scope_index = current_submit_scope_index;
        }
        for (auto & [used_image_t_id, used_image_t_access, used_image_slice] : info.used_images)
        {
            ImplTaskImage & impl_task_image = impl.impl_task_images[used_image_t_id.index];
            auto [current_image_layout, current_image_access] = task_image_access_to_layout_access(used_image_t_access);
            // Now this seems strange, why would be need multiple current use slices, as we only have one here.
            // This is because when we intersect this slice with the tracked slices, we get an intersection and a rest.
            // We need to then test the rest against all the remaining tracked uses.
            // If we keep a rest in the end we also need to transition all of them into the correct layout.
            tl_new_use_slices.push_back(used_image_slice);
            // This is the tracked slice we will insert after we finished analyzing the current used image.
            TaskImageTrackedSlice ret_tracked_slice{
                .latest_access = current_image_access,
                .latest_layout = current_image_layout,
                .latest_access_batch_index = batch_index,
                .latest_access_submit_scope_index = current_submit_scope_index,
                .latest_access_read_barrier_index = {}, // this is a dummy value
                .slice = used_image_slice,
            };
            // As image subresources can be in different layouts and also different synchronization scopes,
            // we need to track these image ranges individually.
            for (
                auto tracked_slice_iter = impl_task_image.slices_last_uses.begin();
                tracked_slice_iter != impl_task_image.slices_last_uses.end();
                ++tracked_slice_iter)
            {
                TaskImageTrackedSlice tracked_slice = *tracked_slice_iter;
                for (
                    auto used_image_slice_iter = tl_new_use_slices.begin();
                    used_image_slice_iter != tl_new_use_slices.end();
                    ++used_image_slice_iter)
                {
                    ImageMipArraySlice used_image_slice = *used_image_slice_iter;
                    // We are only intrested in intersecting ranges, as use of non intersecting ranges does not need synchronization.
                    if (!used_image_slice.intersects(tracked_slice.slice))
                    {
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
                    tracked_slice_iter = impl_task_image.slices_last_uses.erase(tracked_slice_iter);
                    // Now we remember the left over slice from the origina tracked slice.
                    for (usize rest_i; rest_i < tracked_slice_rest_count; ++rest_i)
                    {
                        // The rest tracked slices are the same as the original tracked slice,
                        tl_tracked_slice_rests.push_back(tracked_slice);
                        // except for the slice itself, wich is the remainder of the substraction of the intersection.
                        tl_tracked_slice_rests.back().slice = tracked_slice_rest[rest_i];
                    }
                    // Now we remember the left over slice from our current used slice.
                    for (usize rest_i; rest_i < tracked_slice_rest_count; ++rest_i)
                    {
                        // We reassign the iterator here as it is getting invalidated by the insert.
                        // The new iterator points to the newly inserted element.
                        // When the next iteration of the outer for loop starts, all these elements get skipped.
                        // This is good as these elements do NOT intersect with the currently inspected tracked slice.
                        used_image_slice_iter = tl_new_use_slices.insert(
                            tl_new_use_slices.end(),
                            new_use_slice_rest[rest_i]);
                        // except for the slice itself, wich is the remainder of the substraction of the intersection.
                        tl_tracked_slice_rests.back().slice = tracked_slice_rest[rest_i];
                    }
                    // Every other access (NONE, READ_WRITE, WRITE) are interpreted as writes in this context.
                    // When the last use was a read AND the new use of the buffer is a read AND,
                    // we need to add our stage flags to the existing barrier of the last use.
                    // To be able to do this the layout of the image slice must also match.
                    // If they differ we need to insert an execution barrier with a layout transition.
                    bool is_last_access_read = tracked_slice.latest_access.type == AccessTypeFlagBits::READ;
                    bool is_current_access_read = current_image_access.type == AccessTypeFlagBits::READ;
                    bool are_layouts_identical = tracked_slice.latest_layout == current_image_layout;
                    if (is_last_access_read && is_current_access_read && are_layouts_identical)
                    {
                        auto & last_read_split_barrier = impl.split_barriers[tracked_slice.latest_access_read_barrier_index];
                        last_read_split_barrier.dst_access = last_read_split_barrier.dst_access | current_image_access;
                    }
                    else
                    {
                        // When the uses are incompatible (no read on read, or no identical layout) we need to insert a new barrier.
                        usize split_barrier_index = impl.split_barriers.size();
                        impl.split_barriers.push_back(TaskSplitBarrier{
                            .split_barrier_state = impl.info.device.create_split_barrier({
                                .debug_name = std::string("TaskList \"") +
                                              impl.info.debug_name +
                                              "\" SplitBarrier (Image) Nr. " +
                                              std::to_string(split_barrier_index),
                            }),
                            .image_id = used_image_t_id,
                            .slice = intersection,
                            .layout_before = tracked_slice.latest_layout,
                            .layout_after = current_image_layout,
                            .src_access = tracked_slice.latest_access,
                            .dst_access = current_image_access,
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
                            ret_tracked_slice.latest_access_read_barrier_index = split_barrier_index;
                        }
                    }
                }
            }
            // If we have a remainder left of the used image slices, there was no previous use of those slices.
            // We need to translate those image slices into the correct layout.
            for (ImageMipArraySlice rest_used_slice : tl_new_use_slices)
            {
                usize pipeline_barrier_index = impl.barriers.size();
                impl.barriers.push_back(TaskBarrier{
                    .image_id = used_image_t_id,
                    .slice = rest_used_slice,
                    .layout_before = impl_task_image.initial_layout,
                    .layout_after = current_image_layout,
                    // In the future it may be good to give an initial image state and access.
                    .src_access = impl_task_image.initial_access,
                    .dst_access = current_image_access,
                });
            }
            // Now we need to add the latest use and tracked range of our current access:
            impl_task_image.slices_last_uses.push_back(ret_tracked_slice);
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
    }

    void TaskList::present(TaskPresentInfo const & info)
    {
    }

    void TaskList::complete()
    {
        auto & impl = *as<ImplTaskList>();
        DAXA_DBG_ASSERT_TRUE_M(!impl.compiled, "Can only complete a task list one time");

        impl.compiled = true;
    }

    void TaskList::output_graphviz()
    {
        auto & impl = *as<ImplTaskList>();
        DAXA_DBG_ASSERT_TRUE_M(impl.compiled, "must compile before getting graphviz");
        impl.output_graphviz();
    }

    auto TaskList::command_lists() -> std::vector<CommandList>
    {
        return {};
    }

    void TaskList::execute()
    {
    }

    auto TaskList::last_access(TaskBufferId buffer) -> Access
    {
        auto & impl = *as<ImplTaskList>();
        DAXA_DBG_ASSERT_TRUE_M(impl.compiled, "final access only available after compilation");

        return impl.impl_task_buffers[buffer.index].latest_access;
    }

    auto TaskList::last_access(TaskImageId image) -> Access
    {
        auto & impl = *as<ImplTaskList>();
        DAXA_DBG_ASSERT_TRUE_M(impl.compiled, "final access only available after compilation");

        DAXA_DBG_ASSERT_TRUE_M(false, "THIS NEEDS TO BE IMPLEMENTED");

        return Access{};
    }

    auto TaskList::last_layout(TaskImageId image) -> ImageLayout
    {
        auto & impl = *as<ImplTaskList>();
        DAXA_DBG_ASSERT_TRUE_M(impl.compiled, "final layout only available after compilation");

        DAXA_DBG_ASSERT_TRUE_M(false, "THIS NEEDS TO BE IMPLEMENTED");

        return ImageLayout{};
    }

    ImplTaskList::ImplTaskList(TaskListInfo const & info)
        : info{info}
    {
    }

    ImplTaskList::~ImplTaskList()
    {
    }

    auto ImplTaskList::get_buffer(TaskBufferId) -> BufferId
    {
        return {};
    }

    auto ImplTaskList::get_image(TaskImageId) -> ImageId
    {
        return {};
    }

    auto ImplTaskList::get_image_view(TaskImageId) -> ImageViewId
    {
        return {};
    }

    void ImplTaskList::output_graphviz()
    {
        // TODO(grundlett): Implement this!
        std::string filename = this->info.debug_name + ".dot";
        std::ofstream dot_file{filename};

        dot_file << "digraph TaskGraph {\nnode [style=filled, shape=box, color=\"#d3f4ff\"]\n";

        usize scope_index = 0;
        for (auto const & scope : batch_submit_scopes)
        {
            usize batch_index = 0;
            for (auto const & batch : scope.task_batches)
            {
                dot_file << "subgraph cluster_" << std::to_string(batch_index) << " {\n";
                for (auto const & task_id : batch.tasks)
                {
                    std::string task_name = std::string("task_") + std::to_string(task_id);
                    // if (ImplGenericTask * task_ptr = std::get_if<ImplGenericTask>(&events[task_index].event_variant))
                    // {
                    //     dot_file << "subgraph cluster_" << task_name << " {\n";
                    //     dot_file << "label=\"" << task_ptr->info.debug_name << "\"\n";
                    //     dot_file << "shape=box\nstyle=filled\ncolor=lightgray\n";
                    //     for (auto & [task_buffer_id, t_access] : task_ptr->info.used_buffers)
                    //     {
                    //         ImplTaskBuffer & task_buffer = this->impl_task_buffers[task_buffer_id.index];
                    //         dot_file << "bnode_" << task_index << "_" << task_buffer_id.index;
                    //         dot_file << " [label=\"" << task_buffer.debug_name << "\", shape=box]\n";
                    //     }
                    //     for (auto & [task_image_id, t_access] : task_ptr->info.used_images)
                    //     {
                    //         ImplTaskImage & task_buffer = this->impl_task_images[task_image_id.index];
                    //         dot_file << "inode_" << task_index << "_" << task_image_id.index;
                    //         dot_file << " [label=\"" << task_buffer.debug_name << "\", shape=box]\n";
                    //     }
                    //     dot_file << "}\n";
                    // }
                    // else if (ImplCreateBufferTask * task_ptr = std::get_if<ImplCreateBufferTask>(&events[task_index].event_variant))
                    // {
                    //     ImplTaskBuffer & task_buffer = this->impl_task_buffers[task_ptr->id.index];
                    //     dot_file << "c_bnode_" << task_index << "_" << task_ptr->id.index;
                    //     dot_file << " [label=\"Create " << task_buffer.debug_name << "\", shape=box]\n";
                    // }
                    // else if (TaskImageCreateEvent * task_ptr = std::get_if<TaskImageCreateEvent>(&events[task_index].event_variant))
                    // {
                    //     // TODO(pahrens): make the `ids[]` use the "right" index (instead of 0)
                    //     ImplTaskImage & task_image = this->impl_task_images[task_ptr->ids[0].index];
                    //     dot_file << "c_inode_" << task_index << "_" << task_ptr->ids[0].index;
                    //     dot_file << " [label=\"Create " << task_image.debug_name << "\", shape=box]\n";
                    // }
                    // else
                    {
                        dot_file << "node" << task_id << " [label=\"unknown task\", shape=box]\n";
                    }
                }
                ++batch_index;
            }
            ++scope_index;
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

    // void ImplTaskList::insert_synchronization()
    // {
    //     // TODO(pahrens): Implement this!
    // }
} // namespace daxa

#endif
