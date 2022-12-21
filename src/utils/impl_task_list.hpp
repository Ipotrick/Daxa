#pragma once

#include <stack>
#include <daxa/utils/task_list.hpp>

#define DAXA_TASK_LIST_DEBUG 1

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

    using TaskBatchId = usize;

    using TaskId = usize;

    struct LastReadSplitBarrierIndex
    {
        usize index;
    };

    struct LastReadBarrierIndex
    {
        usize index;
    };

    struct ImplTaskBuffer
    {
        std::vector<BufferId> buffers = {};
        TaskBufferInfo info = {};
        Access latest_access = AccessConsts::NONE;
        usize latest_access_batch_index = {};
        usize latest_access_submit_scope_index = {};
        // When the last index was a read and an additional read is followed after,
        // we will combine all barriers into one, which is the first barrier that the first read generates.
        std::variant<LastReadSplitBarrierIndex, LastReadBarrierIndex, std::monostate> latest_access_read_barrier_index = {};
    };

    struct TaskBarrier
    {
        // when this ID is invalid, this barrier is NOT an image memory barrier but just a memory barrier.
        // So when ID invalid => memory barrier, ID valid => image memory barrier.
        TaskImageId image_id = {};
        ImageMipArraySlice slice = {};
        ImageLayout layout_before = {};
        ImageLayout layout_after = {};
        Access src_access = {};
        Access dst_access = {};
        usize src_batch = {};
        usize dst_batch = {};
    };

    struct TaskSplitBarrier : TaskBarrier
    {
        SplitBarrierState split_barrier_state;
    };

    struct TaskImageTrackedSlice
    {
        Access latest_access = AccessConsts::NONE;
        ImageLayout latest_layout = ImageLayout::UNDEFINED;
        usize latest_access_batch_index = {};
        usize latest_access_submit_scope_index = {};
        // When the last index was a read and an additional read is followed after,
        // we will combine all barriers into one, which is the first barrier that the first read generates.
        std::variant<LastReadSplitBarrierIndex, LastReadBarrierIndex, std::monostate> latest_access_read_barrier_index = {};
        ImageMipArraySlice slice = {};
    };

    struct ImplTaskImage
    {
        std::vector<ImageId> images = {};
        TaskImageInfo info = {};
        bool swapchain_semaphore_waited_upon = {};
        std::vector<TaskImageTrackedSlice> slices_last_uses = {};
    };

    struct Task
    {
        TaskInfo info = {};
    };

    struct CreateTaskBufferTask
    {
        TaskBufferId id = {};
    };

    struct CreateTaskImageTask
    {
        std::array<TaskImageId, 2> ids = {};
        usize id_count = {};
    };

    struct ImplPresentInfo
    {
        std::vector<BinarySemaphore> * user_binary_semaphores = {};
        std::vector<BinarySemaphore> binary_semaphores = {};
    };

    struct TaskBatch
    {
        std::vector<usize> pipeline_barrier_indices = {};
        std::vector<usize> wait_split_barrier_indices = {};
        std::vector<TaskId> tasks = {};
        std::vector<usize> signal_split_barrier_indices = {};
    };

    struct TaskBatchSubmitScope
    {
        CommandSubmitInfo submit_info = {};
        CommandSubmitInfo * user_submit_info = {};
        // These barriers are inserted after all batches and their sync.
        std::vector<usize> last_minute_barrier_indices = {};
        std::vector<TaskBatch> task_batches = {};
        std::vector<u64> used_swapchain_task_images = {};
        std::optional<ImplPresentInfo> present_info = {};
    };

    auto task_image_access_to_layout_access(TaskImageAccess const & access) -> std::tuple<ImageLayout, Access>;
    auto task_buffer_access_to_access(TaskBufferAccess const & access) -> Access;

    struct ImplTaskList final : ManagedSharedState
    {
        TaskListInfo info;
        TaskImageId swapchain_image = {};
        std::vector<Task> tasks = {};
        std::vector<TaskSplitBarrier> split_barriers = {};
        std::vector<TaskBarrier> barriers = {};
        std::vector<ImplTaskBuffer> impl_task_buffers = {};
        std::vector<ImplTaskImage> impl_task_images = {};
        std::vector<TaskBatchSubmitScope> batch_submit_scopes = {};
        usize swapchain_image_first_use_submit_scope_index = std::numeric_limits<usize>::max();
        usize swapchain_image_last_use_submit_scope_index = std::numeric_limits<usize>::max();
        bool compiled = false;
        bool executed = false;

        std::vector<CommandList> left_over_command_lists = {};

        void debug_print_task_barrier(TaskBarrier & barrier, usize index, std::string_view prefix);
        void debug_print_task_split_barrier(TaskSplitBarrier & barrier, usize index, std::string_view prefix);
        void debug_print_task(Task & task, usize task_id, std::string_view prefix);
        void execute_barriers();
        void output_graphviz();

        void add_runtime_buffer(TaskBufferId tid, BufferId id);
        void add_runtime_image(TaskImageId tid, ImageId id);
        void remove_runtime_buffer(TaskBufferId tid, BufferId id);
        void remove_runtime_image(TaskImageId tid, ImageId id);
        void clear_runtime_buffers(TaskBufferId tid);
        void clear_runtime_images(TaskImageId tid);

        ImplTaskList(TaskListInfo a_info);
        virtual ~ImplTaskList() override final;
    };

    struct ImplTaskRuntime
    {
        // interface:
        ImplTaskList & task_list;
        Task * current_task = {};
        bool reuse_last_command_list = true;
        std::vector<CommandList> command_lists = {};
        std::optional<BinarySemaphore> last_submit_semaphore = {};
    };
} // namespace daxa
