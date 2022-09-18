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

    struct EventId
    {
        usize submit_scope_index = {};
        usize event_index = {};
    };

    struct ImplTaskBuffer
    {
        Access latest_access = AccessConsts::NONE; 
        usize latest_access_task_index = {};
        EventId latest_access_event = {};
        BufferId* buffer = {};
        std::string debug_name = {};
    };

    TaskImageTrackedSlice
    {
        Access latest_access = AccessConsts::NONE;
        ImageLayout latest_layout = ImageLayout::UNDEFINED;
        EventId latest_access_event = {};
        ImageMipArraySlice slice = {};
    };

    struct ImplTaskImage
    {
        ImageId* image = {};
        std::vector<TaskImageTrackedSlice> slices = {};
        std::optional<std::pair<Swapchain, BinarySemaphore>> parent_swapchain = {};
        bool swapchain_semaphore_waited_upon = {};
        std::string debug_name = {};
    };

    struct TaskMemoryBarrier
    {
        Access awaited_pipeline_access = AccessConsts::NONE;
        Access waiting_pipeline_access = AccessConsts::NONE;
        BufferId buffer_id = {};
    };

    struct TaskImageBarrier
    {
        Access awaited_pipeline_access = AccessConsts::NONE;
        Access waiting_pipeline_access = AccessConsts::NONE;
        ImageLayout before_layout = ImageLayout::UNDEFINED;
        ImageLayout after_layout = ImageLayout::UNDEFINED;
        TaskImageId image_id = {};
        ImageMipArraySlice image_slice = {};
    };

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
        std::vector<TaskPipelineBarrier> barriers = {};
        CommandSubmitInfo submit_info;
        CommandSubmitInfo* user_submit_info;
    };  

    struct PresentEvent
    {
        PresentInfo present_info;
        std::vector<BinarySemaphore>* user_binary_semaphores = {};
        TaskImageId presented_image = {};
    };

    using EventVariant = std::variant<
        TaskEvent,
        CreateTaskBufferEvent,
        CreateTaskImageEvent,
        SubmitEvent,
        PresentEvent,
        std::monostate
    >;

    struct EventDependency
    {
        EventId src = {};
        EventId dst = {};
        std::vector<TaskMemoryBarrier> memory_barriers = {};
        std::vector<TaskMemoryBarrier> image _barriers = {};
    }

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

        void pipeline_barriers(std::vector<TaskPipelineBarrier> const & barriers);
    };

    struct EventSubmitScope
    {
        std::vector<Event> events = {};
    };

    struct EventBatch
    {
        std::vector<EventId> events = {};
    };

    struct ImplTaskList final : ManagedSharedState
    {
        TaskListInfo info;

        std::vector<EventSubmitScope> event_submit_scopes = {};
        std::vector<EventDependency> event_dependencies = {};
        std::vector<EventBatch> event_batches = {};
        std::vector<ImplTaskBuffer> impl_task_buffers = {};
        std::vector<ImplTaskImage> impl_task_images = {};

        TaskRecordState record_state = {};
        TaskGraph compiled_graph = {};
        std::vector<CommandList> left_over_command_lists = {};
        u64 last_submit_event_index = std::numeric_limits<u64>::max(); 

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
