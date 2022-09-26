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
        EventId latest_access_event = {};
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
        std::vector<EventBatchDependency> dependenceis = {};
    };

    auto task_image_access_to_layout_access(TaskImageAccess const & access) -> std::tuple<ImageLayout, Access>;
    auto task_buffer_access_to_access(TaskBufferAccess const & access) -> Access;

    // TODO(pahrens): Implement the body of this, and define TaskPipelineBarrierInfo
    // auto compute_needed_barrier(Access const & previous_access, Access const & new_access) -> std::optional<TaskPipelineBarrierInfo>;

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
