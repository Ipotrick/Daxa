#pragma once

#include "impl_core.hpp"

#include <daxa/task_list.hpp>

namespace daxa
{
    struct ImplDevice;

    struct ImplTaskBuffer
    {
        Access last_access = AccessConsts::NONE;
        TaskImageAccess last_task_access = TaskImageAccess::NONE;
        usize last_access_task_index = {};
        BufferId runtime_id = {};
    };

    struct ImplTaskImage
    {
        Access last_access = AccessConsts::NONE;
        TaskImageAccess last_task_access = TaskImageAccess::NONE;
        usize last_access_task_index = {};
        ImageId runtime_id = {};
        ImageViewId runtime_view_id = {};
    };

    using TaskInfoVariant = std::variant<TaskInfo, TaskRenderInfo, std::monostate>;

    struct TaskPipelineBarrier
    {
        bool with_image_transition = {};
    };

    struct ImplTask{
        TaskInfoVariant info = {};
        std::vector<TaskPipelineBarrier> barriers = {};
    };

    struct ImplTaskList
    {
        TaskListInfo info;
        std::weak_ptr<ImplDevice> impl_device = {};

        bool compiled = false;
        std::vector<ImplTask> tasks = {};
        usize last_task_index_with_barrier = 0;

        u32 next_task_buffer_id = 0;
        u32 next_task_image_id = 0;
        std::vector<ImplTaskBuffer> impl_task_buffers = {};
        std::vector<ImplTaskImage> impl_task_images = {};

        auto task_image_access_to_layout_access(TaskImageAccess const & access) -> std::tuple<ImageLayout, Access>;
        auto task_buffer_access_to_access(TaskImageAccess const & access) -> Access;
        auto compute_needed_barrier(Access const & previous_access, Access const & new_access) -> std::optional<TaskPipelineBarrier>;

        auto slot(TaskBufferId id) -> ImplTaskBuffer &;
        auto slot(TaskImageId id) -> ImplTaskImage &;

        auto get_buffer(TaskBufferId) -> BufferId;
        auto get_image(TaskImageId) -> ImageId;
        auto get_image_view(TaskImageId) -> ImageViewId;

        void analyze_dependencies();
        auto insert_synchonization();

        ImplTaskList(std::shared_ptr<ImplDevice> a_impl_device, TaskListInfo const & info);
        ~ImplTaskList();
    };
} // namespace daxa
