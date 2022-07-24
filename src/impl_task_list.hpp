#pragma once

#include <daxa/task_list.hpp>

namespace daxa
{
    struct ImplDevice;

    struct ImplTaskBuffer
    {
        Access last_access = AccessConsts::NONE;
        usize last_access_task_index = {};
        CreateTaskBufferCallback fetch_callback = {};
        BufferId runtime_id = {};
    };

    struct ImplTaskImage
    {
        Access last_access = AccessConsts::NONE;
        ImageLayout last_layout = ImageLayout::UNDEFINED;
        usize last_access_task_index = {};
        CreateTaskImageCallback fetch_callback = {};
        ImageId runtime_id = {};
        ImageViewId runtime_view_id = {};
        ImageMipArraySlice slice = {};
    };

    struct TaskPipelineBarrier
    {
        Access awaited_pipeline_access = AccessConsts::NONE;
        Access waiting_pipeline_access = AccessConsts::NONE;
        ImageLayout before_layout = ImageLayout::UNDEFINED;
        ImageLayout after_layout = ImageLayout::UNDEFINED;
        TaskImageId image_id = {};
        ImageMipArraySlice image_slice = {};
    };

    struct ImplTask{
        std::vector<usize> create_task_buffer_ids = {};
        std::vector<usize> create_task_image_ids = {};
        std::vector<TaskPipelineBarrier> barriers = {};
        TaskInfo info = {};
    };

    struct ImplTaskList
    {
        TaskListInfo info;

        bool compiled = false;
        std::vector<ImplTask> tasks = {};
        usize last_task_index_with_barrier = 0;

        u32 next_task_buffer_id = 0;
        u32 next_task_image_id = 0;
        std::vector<ImplTaskBuffer> impl_task_buffers = {};
        std::vector<ImplTaskImage> impl_task_images = {};

        // execution:
        Device* current_device = {};
        CommandList* current_command_list = {};
        TaskResources* current_resources = {};

        auto task_image_access_to_layout_access(TaskImageAccess const & access) -> std::tuple<ImageLayout, Access>;
        auto task_buffer_access_to_access(TaskBufferAccess const & access) -> Access;
        auto compute_needed_barrier(Access const & previous_access, Access const & new_access) -> std::optional<TaskPipelineBarrier>;

        void execute_barriers();

        auto slot(TaskBufferId id) -> ImplTaskBuffer &;
        auto slot(TaskImageId id) -> ImplTaskImage &;

        auto get_buffer(TaskBufferId) -> BufferId;
        auto get_image(TaskImageId) -> ImageId;
        auto get_image_view(TaskImageId) -> ImageViewId;

        void insert_synchronization();

        ImplTaskList(TaskListInfo const & info);
        ~ImplTaskList();
    };
} // namespace daxa
