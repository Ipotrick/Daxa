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

    using EventBatchId = usize;

    struct EventId
    {
        usize submit_scope_index = {};
        usize event_index = {};
    };

    struct ImplTaskBuffer
    {
        Access latest_access = AccessConsts::NONE;
        EventId latest_access_event = {};
        BufferId * buffer = {};
        std::string debug_name = {};
    };

    struct TaskImageTrackedSlice
    {
        Access latest_access = AccessConsts::NONE;
        ImageLayout latest_layout = ImageLayout::UNDEFINED;
        EventId latest_access_batch = {};
        usize latest_access_batch_barrier_index_begin = {};
        usize latest_access_batch_barrier_index_count = {};
        ImageMipArraySlice slice = {};
    };

    struct ImplTaskImage
    {
        std::optional<std::pair<Swapchain, BinarySemaphore>> parent_swapchain = {};
        bool swapchain_semaphore_waited_upon = {};
        std::vector<TaskImageTrackedSlice> slices = {};
        ImageId * image = {};
        std::string debug_name = {};
    };

    struct TaskImageBarrierInfo
    {
        Access awaited_pipeline_access = AccessConsts::NONE;
        Access waiting_pipeline_access = AccessConsts::NONE;
        ImageLayout before_layout = ImageLayout::UNDEFINED;
        ImageLayout after_layout = ImageLayout::UNDEFINED;
        ImageMipArraySlice image_slice = {};
        TaskImageId task_image_id = {};
    };

    auto get_image_barrier(TaskImageBarrierInfo const & task_image_barrier, ImageId image_id) -> ImageBarrierInfo;

    struct TaskEvent
    {
        TaskInfo info = {};
    };

    struct CreateTaskBufferEvent
    {
        TaskBufferId id = {};
    };

    struct CreateTaskImageEvent
    {
        std::array<TaskImageId, 2> ids = {};
        usize id_count = {};
    };

    struct SubmitEvent
    {
        CommandSubmitInfo submit_info;
        CommandSubmitInfo * user_submit_info;
    };

    struct PresentEvent
    {
        PresentInfo present_info;
        std::vector<BinarySemaphore> * user_binary_semaphores = {};
        TaskImageId presented_image = {};
    };

    using EventVariant = std::variant<
        TaskEvent,
        CreateTaskBufferEvent,
        CreateTaskImageEvent,
        SubmitEvent,
        PresentEvent,
        std::monostate>;

    struct EventBatchDependency
    {
        EventId src = {};
        EventId dst = {};
        std::vector<TaskImageBarrierInfo> memory_barriers = {};
        std::vector<MemoryBarrierInfo> image_barriers = {};
    };

    struct Event
    {
        std::vector<usize> src_event_dependency_indices = {};
        std::vector<usize> dst_event_dependency_indices = {};
        EventVariant event_variant = {};
    };

    struct TaskSubmitScope
    {
        CommandSubmitInfo submit_info = {};
        std::vector<u64> used_swapchain_task_images = {};
    };

    struct TaskRuntime
    {
        // interface:
        bool reuse_last_command_list = true;

        Device current_device;
        std::vector<CommandList> command_lists = {};
        std::vector<ImplTaskBuffer> & impl_task_buffers;
        std::vector<ImplTaskImage> & impl_task_images;
        std::vector<TaskSubmitScope> & submit_scopes;

        std::optional<BinarySemaphore> last_submit_semaphore = {};

        void execute_task(TaskEvent & task, usize task_index);
    };

    struct EventSubmitScope
    {
        std::vector<Event> events = {};
    };

    struct EventBatch
    {
        std::vector<EventId> events = {};
        std::vector<EventProtoBarrier> dependenceis = {};
    };

    auto task_image_access_to_layout_access(TaskImageAccess const & access) -> std::tuple<ImageLayout, Access>;
    auto task_buffer_access_to_access(TaskBufferAccess const & access) -> Access;

    // TODO(pahrens): Implement the body of this, and define TaskPipelineBarrierInfo
    // auto compute_needed_barrier(Access const & previous_access, Access const & new_access) -> std::optional<TaskPipelineBarrierInfo>;

    void insert_task()
    {
        usize batch_index = find_earliest_possible_batch_index(batches, event);

        for (buffer : event.buffers)
        {
            insert_split_barrier_prototypes(buffer);
        }

        for (image : event.images)
        {
            insert_split_barrier_prototypes(buffer);
        }
    }

    void insert_split_barrier_prototypes(TaskImageAccess t_access, ImageMipArraySlice slice, usize event_batch_index, TaskImageId image_id)
    {
        auto [layout, access] = task_image_access_to_layout_access(t_access);

        ImplTaskImage& task_image = task_images[image_id.index];

        TaskImageTrackedSlice new_tracked_slice = {};

        // When we go over all tracked slices we can find ones that overlap with the new one.
        // If we find overlap, we need to make sure that we put the new event after (write or diff read layout) or in the same (same read layout) batch.
        // While iterating over the tracked slices we find the max batch index we need to put the new event into.
        // The max batch index is the first one we can put the new event into without violating any synchronization.
        usize batch_index = {};

        // We need to make a list of barriers that will need to be inserted before the new event
        std::vector<TaskImageBarrierInfo> barriers = {}; 
        for (auto& old_use : task_image.slices)
        {
            if (old_use.slice.intersects(slice))
            {
                if (old_use.latest_layout == layout)
                {
                    // 1. find rest old, rest new, insersection.
                    // 2. ignore old for pipeline barrier it doesnt matter. 
                    // 3. we allready have a barrier. Add src and dst to the existing barrier.
                    // 5. or on the slice we are building the accesses from old
                    // 4. remove old, replace it with rest old
                }
                else
                {
                    // The first batch index we can put the new event into is one after the currently tested old use.
                    // max the batch_index with old_use.batch_index+1

                    // 1. find rest old, rest new, insersection.

                    // 2. we need to create a new barrier
                    
                    // 3. we allready have a barrier. Add src and dst to the existing barrier.
                    // 5. or on the slice we are building the accesses from old
                    // 4. remove old, replace it with rest old
                    // 5. max the batch_index with the 
                }
            }
            else
            {
                // put event into first batch, no barriers
            }
        }
    }

    struct ImplTaskList final : ManagedSharedState
    {
        TaskListInfo info;

        std::vector<EventSubmitScope> event_submit_scopes = {};
        std::vector<EventBatch> event_batches = {};
        std::vector<ImplTaskBuffer> impl_task_buffers = {};
        std::vector<ImplTaskImage> impl_task_images = {};

        bool compiled = false;

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
