#pragma once

#include <stack>
#include <sstream>
#include <daxa/utils/task_list.hpp>

#define DAXA_TASKLIST_MAX_CONITIONALS 31

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

    struct TaskBuffer
    {
        /// Every permutation always has all buffers but they are not necessarily valid in that permutation.
        /// This boolean is used to check this.
        bool valid = {};
        Access latest_access = AccessConsts::NONE;
        usize latest_access_batch_index = {};
        usize latest_access_submit_scope_index = {};
        Access pre_task_list_slice_states = AccessConsts::NONE;
        usize initial_access_batch_index = {};
        usize initial_access_submit_scope_index = {};
        // When the last index was a read and an additional read is followed after,
        // we will combine all barriers into one, which is the first barrier that the first read generates.
        std::variant<std::monostate, LastReadSplitBarrierIndex, LastReadBarrierIndex> latest_access_read_barrier_index = std::monostate{};
    };

    struct GlobalTaskBufferInfos
    {
        TaskBufferInfo info = {};
        // One task buffer can back multiple buffers.
        std::vector<BufferId> actual_buffers = {};
        // We store execution time information about the previous executions final resource states.
        // This is important, as with conditional execution and temporal resources we need to store this infomation to form correct state transitions.
        std::optional<Access> previous_execution_last_access = {};
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

    struct ExtendedImageSliceState
    {
        ImageSliceState state = {};
        usize latest_access_batch_index = {};
        usize latest_access_submit_scope_index = {};
        // When the last index was a read and an additional read is followed after,
        // we will combine all barriers into one, which is the first barrier that the first read generates.
        std::variant<std::monostate, LastReadSplitBarrierIndex, LastReadBarrierIndex> latest_access_read_barrier_index = std::monostate{};
    };

    struct TaskImage
    {
        /// Every permutation always has all buffers but they are not necessarily valid in that permutation.
        /// This boolean is used to check this.
        bool valid = {};
        bool swapchain_semaphore_waited_upon = {};
        std::vector<ExtendedImageSliceState> last_slice_states = {};
        std::vector<ExtendedImageSliceState> first_slice_states = {};
    };

    struct GlobalTaskImageInfo
    {
        TaskImageInfo info = {};
        // One task image can be backed by multiple images at execution time.
        std::vector<ImageId> actual_images = {};
        // We store runtime information about the previous executions final resource states.
        // This is important, as with conditional execution and temporal resources we need to store this infomation to form correct state transitions.
        std::optional<std::vector<ExtendedImageSliceState>> previous_execution_last_slices = {};
    };

    struct Task
    {
        TaskInfo info = {};
        std::vector<std::vector<ImageViewId>> image_view_cache = {};
        std::vector<std::variant<std::pair<TaskImageId, usize>, std::pair<TaskBufferId, usize>, std::monostate>> id_to_offset = {}; 
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
        std::vector<BinarySemaphore> binary_semaphores = {};
        std::vector<BinarySemaphore> * additional_binary_semaphores = {};
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
        TaskSubmitInfo user_submit_info = {};
        // These barriers are inserted after all batches and their sync.
        std::vector<usize> last_minute_barrier_indices = {};
        std::vector<TaskBatch> task_batches = {};
        std::vector<u64> used_swapchain_task_images = {};
        std::optional<ImplPresentInfo> present_info = {};
    };

    auto task_image_access_to_layout_access(TaskImageAccess const & access) -> std::tuple<ImageLayout, Access>;
    auto task_buffer_access_to_access(TaskBufferAccess const & access) -> Access;

    struct TaskListCondition
    {
        bool * condition = {};
    };

    struct ImplTaskList;

    struct TaskListPermutation
    {
        // record time information:
        bool active = {};
        // persistent information:
        TaskImageId swapchain_image = {};
        std::vector<TaskBuffer> buffer_infos = {};
        std::vector<TaskImage> image_infos = {};
        std::vector<Task> tasks = {};
        std::vector<TaskSplitBarrier> split_barriers = {};
        std::vector<TaskBarrier> barriers = {};
        std::vector<usize> initial_barriers = {};
        std::vector<TaskBatchSubmitScope> batch_submit_scopes = {};
        usize swapchain_image_first_use_submit_scope_index = std::numeric_limits<usize>::max();
        usize swapchain_image_last_use_submit_scope_index = std::numeric_limits<usize>::max();

        void add_task(ImplTaskList & task_list_impl, TaskInfo const & info);
        void submit(TaskSubmitInfo const & info);
        void present(TaskPresentInfo const & info);
    };

    struct ImplTaskList final : ManagedSharedState
    {
        TaskListInfo info;
        std::vector<GlobalTaskBufferInfos> global_buffer_infos = {};
        std::vector<GlobalTaskImageInfo> global_image_infos = {};
        std::vector<TaskListCondition> conditions = {};
        std::vector<TaskListPermutation> permutations = {};

        // record time information:
        u32 record_active_conditional_scopes = {};
        u32 record_conditional_states = {};
        std::vector<TaskListPermutation *> record_active_permutations = {};
        std::unordered_map<std::string, TaskBufferId> buffer_name_to_id = {}; 
        std::unordered_map<std::string, TaskImageId> image_name_to_id = {}; 
        bool compiled = {};

        // execution time information:
        daxa::TransferMemoryPool staging_memory{TransferMemoryPoolInfo{.device = info.device, .capacity = info.staging_memory_pool_size, .use_bar_memory = true, .name = info.name } };
        std::array<bool, DAXA_TASKLIST_MAX_CONITIONALS> execution_time_current_conditionals = {};
        bool enable_debug_print = {};

        // post execution information:
        usize last_execution_staging_timeline_value = 0;
        u32 chosen_permutation_last_execution = {};
        std::vector<CommandList> left_over_command_lists = {};
        bool executed_once = {};
        u32 prev_frame_permutation_index = {};
        std::stringstream debug_string_stream = {};

        void update_active_permutations();
        void update_image_view_cache(Task & task);

        void debug_print_memory_barrier(MemoryBarrierInfo & barrier, std::string_view prefix);
        void debug_print_image_memory_barrier(ImageBarrierInfo & barrier, GlobalTaskImageInfo & glob_image, std::string_view prefix);
        void debug_print_task_barrier(TaskBarrier & barrier, usize index, std::string_view prefix);
        void debug_print_task_split_barrier(TaskSplitBarrier & barrier, usize index, std::string_view prefix);
        void debug_print_task(TaskListPermutation const & permutation, Task & task, usize task_id, std::string_view prefix);
        void debug_print_permutation_image(TaskListPermutation const & permutation, TaskImageId const image_id);
        void debug_print_permutation_buffer(TaskListPermutation const & permutation, TaskBufferId const buffer_id);
        void debug_print();
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

    struct ImplTaskRuntimeInterface
    {
        // interface:
        ImplTaskList & task_list;
        TaskListPermutation & permutation;
        Task * current_task = {};
        bool reuse_last_command_list = true;
        std::vector<CommandList> command_lists = {};
        std::optional<BinarySemaphore> last_submit_semaphore = {};
    };
} // namespace daxa
