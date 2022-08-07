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

    struct ImplConditionalTaskBegin
    {
        std::vector<TaskPipelineBarrier> barriers = {};
        TaskResources resources = {};
        TaskConditionalInfo info = {};
        u64 depth = {}, end_index = {};
    };

    struct ImplConditionalTaskEnd
    {
        u64 depth = {}, begin_index = {};
    };

    using TaskEvent = std::variant<
        ImplGenericTask,
        ImplCreateBufferTask,
        ImplCreateImageTask,
        ImplConditionalTaskBegin,
        ImplConditionalTaskEnd>;

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
        u64 conditional_depth = {};
        std::stack<u64> conditional_task_indices = {};
    };

    // TODO: In sync check if a resource access is across scopes.
    // If so, check if the scope of the previous access has ended
    // if that is the case, write to the previous accesses scope the final state that resource needs to be in in the end of that scope.
    // in execution, this is needed to perform a pipeline barrier, that transitions resources to the correct layout when the usual tasks do not execute.
    struct ImplTaskList final : ManagedSharedState
    {
        TaskListInfo info;

        std::vector<TaskEvent> tasks = {};
        std::vector<ImplTaskBuffer> impl_task_buffers = {};
        std::vector<ImplTaskImage> impl_task_images = {};
        std::vector<CommandList> recorded_command_lists = {};

        TaskRecordState record_state = {};

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

        void insert_synchronization();

        auto managed_cleanup() -> bool override;

        ImplTaskList(TaskListInfo const & info);
        virtual ~ImplTaskList() override final;
    };
} // namespace daxa
