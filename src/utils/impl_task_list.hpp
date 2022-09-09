#pragma once

#include <stack>
#include <daxa/utils/task_list.hpp>

#define DAXA_TASK_LIST_DEBUG 0

#if defined(DAXA_TASK_LIST_DEBUG)
#if DAXA_TASK_LIST_DEBUG
#define DAXA_ONLY_IF_TASK_LIST_DEBUG(x) x
#else
#define DAXA_ONLY_IF_TASK_LIST_DEBUG(x)
#endif
#else
#define DAXA_ONLY_IF_TASK_LIST_DEBUG(x)
#endif

namespace daxa
{
    struct ImplDevice;

    struct ImplTaskBuffer
    {
        Access latest_access = AccessConsts::NONE;
        usize latest_access_task_index = {};
        CreateTaskBufferCallback fetch_callback = {};
        std::string debug_name = {};
    };

    struct RuntimeTaskBuffer
    {
        BufferId buffer_id = {};
    };

    struct ImplTaskImage
    {
        Access latest_access = AccessConsts::NONE;
        ImageLayout latest_layout = ImageLayout::UNDEFINED;
        usize latest_access_task_index = {};
        CreateTaskImageCallback fetch_callback = {};
        ImageMipArraySlice slice = {};
        std::string debug_name = {};
    };

    struct RuntimeTaskImage
    {
        ImageId image_id = {};
        ImageViewId image_view_id = {};
    };

    struct TaskPipelineBarrier
    {
        bool image_barrier = false;
        Access awaited_pipeline_access = AccessConsts::NONE;
        Access waiting_pipeline_access = AccessConsts::NONE;
        ImageLayout before_layout = ImageLayout::UNDEFINED;
        ImageLayout after_layout = ImageLayout::UNDEFINED;
        TaskImageId image_id = {};
        ImageMipArraySlice image_slice = {};
    };

    struct ImplGenericTask
    {
        std::vector<TaskPipelineBarrier> barriers = {};
        TaskInfo info = {};
    };

    struct ImplCreateBufferTask
    {
        TaskBufferId id = {};
    };

    struct ImplCreateImageTask
    {
        TaskImageId id = {};
    };

    using TaskEventVariant = std::variant<
        ImplGenericTask,
        ImplCreateBufferTask,
        ImplCreateImageTask>;

    struct TaskEvent
    {
        TaskEventVariant event_variant;
    };

    struct TaskRuntime
    {
        // interface:
        bool reuse_last_command_list = true;

        Device current_device;
        std::vector<CommandList> command_lists = {};
        std::vector<ImplTaskBuffer> & impl_task_buffers;
        std::vector<ImplTaskImage> & impl_task_images;
        std::vector<RuntimeTaskBuffer> runtime_buffers = {};
        std::vector<RuntimeTaskImage> runtime_images = {};

        std::optional<BinarySemaphore> last_submit_semaphore = {};

        void execute_task(TaskEvent & task, usize task_index);

        void pipeline_barriers(std::vector<TaskPipelineBarrier> const & barriers);
    };

    struct TaskRecordState
    {
    };

    struct TaskLink
    {
        u64 event_a;
        u64 event_b;
        u64 resource;
        TaskPipelineBarrier barrier;
    };

    struct TaskGraph
    {
        std::vector<TaskLink> buffer_links;
        std::vector<TaskLink> image_links;
    };

    struct ImplTaskList final : ManagedSharedState
    {
        TaskListInfo info;

        std::vector<TaskEvent> tasks = {};
        std::vector<ImplTaskBuffer> impl_task_buffers = {};
        std::vector<ImplTaskImage> impl_task_images = {};
        std::vector<CommandList> recorded_command_lists = {};

        TaskRecordState record_state = {};
        TaskGraph compiled_graph = {};

        bool compiled = false;

        auto task_image_access_to_layout_access(TaskImageAccess const & access) -> std::tuple<ImageLayout, Access>;
        auto task_buffer_access_to_access(TaskBufferAccess const & access) -> Access;
        auto compute_needed_barrier(Access const & previous_access, Access const & new_access) -> std::optional<TaskPipelineBarrier>;

        void execute_barriers();

        auto slot(TaskBufferId id) -> ImplTaskBuffer &;
        auto slot(TaskImageId id) -> ImplTaskImage &;

        auto get_buffer(TaskBufferId) -> BufferId;
        auto get_image(TaskImageId) -> ImageId;
        auto get_image_view(TaskImageId) -> ImageViewId;

        void output_graphviz();
        void insert_synchronization();

        ImplTaskList(TaskListInfo const & info);
        virtual ~ImplTaskList() override final;
    };
} // namespace daxa
