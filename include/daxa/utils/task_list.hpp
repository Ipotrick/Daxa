#pragma once

#include <daxa/daxa.hpp>

#include "task_list.inl"

#if !DAXA_BUILT_WITH_UTILS_TASK_LIST
#error "[package management error] You must build Daxa with the DAXA_ENABLE_UTILS_TASK_LIST CMake option enabled, or request the utils-task-list feature in vcpkg"
#endif

#include "mem.hpp"

namespace daxa
{
    struct TaskList;
    struct Device;

    struct TaskRuntimeInterface
    {
        auto get_device() const -> Device &;
        auto get_command_list() const -> CommandList;
        auto get_used_task_buffers() const -> UsedTaskBuffers const &;
        auto get_used_task_images() const -> UsedTaskImages const &;
        auto get_buffers(TaskBufferId const & task_resource_id) const -> std::span<BufferId>;
        auto get_images(TaskImageId const & task_resource_id) const -> std::span<ImageId>;
        auto get_image_views(TaskImageId const & task_resource_id) const -> std::span<ImageViewId>;

        auto get_allocator() const -> TransferMemoryPool &;

        void add_runtime_buffer(TaskBufferId tid, BufferId id);
        void add_runtime_image(TaskImageId tid, ImageId id);
        void remove_runtime_buffer(TaskBufferId tid, BufferId id);
        void remove_runtime_image(TaskImageId tid, ImageId id);
        void clear_runtime_buffers(TaskBufferId tid);
        void clear_runtime_images(TaskImageId tid);

      private:
        friend struct ImplTaskRuntimeInterface;
        friend struct TaskList;
        TaskRuntimeInterface(void * a_backend);
        void * backend = {};
    };

    using TaskCallback = std::function<void(TaskRuntimeInterface const &)>;

    struct TaskBufferInfo
    {
        // For non-execution_persistent resources, task list will synch from the initial use to first use EVERY EXECUTION.
        Access pre_task_list_slice_states = AccessConsts::NONE;
        std::span<BufferId> execution_buffers = {};
        std::string name = {};
    };

    struct TaskImageInfo
    {
        // For execution_persistent resources, task list will synch from the initial use to the first use ONCE.
        // After the FIRST execution, it will use the runtime state of the resource.
        // For non-execution_persistent resources, task list will synch from the initial use to first use EVERY EXECUTION.
        // This is either empty or contains an initial state FOR ALL USES SLICES of the image.
        std::span<ImageSliceState> pre_task_list_slice_states = {};
        bool execution_persistent = {};
        bool swapchain_image = {};
        std::span<ImageId> execution_images = {};
        std::string name = {};
    };

    struct TaskImageAliasInfo
    {
        std::string alias = {};
        std::variant<TaskImageId, std::string> aliased_image = {};
        u32 base_mip_level_offset = {};
        u32 base_array_layer_offset = {};
    };

    struct TaskBufferAliasInfo
    {
        std::string alias = {};
        std::variant<TaskBufferId, std::string> aliased_buffer = {};
    };

    struct TaskInfo
    {
        TaskShaderUses shader_uses = {};
        std::vector<TaskBufferAliasInfo> shader_uses_buffer_aliases = {};
        std::vector<TaskImageAliasInfo> shader_uses_image_aliases = {};
        UsedTaskBuffers used_buffers = {};
        UsedTaskImages used_images = {};
        TaskCallback task = {};
        std::string name = {};
    };

    struct CommandSubmitInfo;
    struct PresentInfo;

    struct TaskListInfo
    {
        Device device;
        /// @brief Task reordering can drastically improve performance,
        /// yet is it also nice to have sequential callback execution.
        bool reorder_tasks = true;
        /// @brief Some drivers have bad implementations for split barriers.
        /// If that is the case for you, you can turn off all use of split barriers.
        /// Daxa will use pipeline barriers instead if this is set.
        bool use_split_barriers = true;
        /// @brief Optionally the user can provide a swapchain. This enables the use of present.
        std::optional<Swapchain> swapchain = {};
        /// @brief Each condition doubled the number of permutations.
        /// For a low number of permutations its is preferable to precompile all permutations.
        /// For a large number of permutations it might be preferable to only create the permutations actually used on the fly just before they are needed.
        /// The second option is enabled by using jit (just in time) compilation.
        bool jit_compile_permutations = {};
        /// @brief Task list can branch the execution based on conditionals. All conditionals must be set before execution and stay constant while executing.
        /// This is usefull to create permutations of a task list without having to create a seperate task list.
        /// Another benefit is that task list can generate synch between executions of permutations while it can not generate synch between two seperate task lists.
        usize permutation_condition_count = {};
        /// @brief Task list will put performance markers that are used by profilers like nsight around each tasks execution by default.
        bool enable_command_labels = true;
        std::array<f32, 4> task_list_label_color = {0.463f, 0.333f, 0.671f, 1.0f};
        std::array<f32, 4> task_batch_label_color = {0.563f, 0.433f, 0.771f, 1.0f};
        std::array<f32, 4> task_label_color = {0.663f, 0.533f, 0.871f, 1.0f};
        /// @brief Records debug information about the execution if enabled. This string is retrievable with the function get_debug_string.
        bool record_debug_information = {};
        u32 staging_memory_pool_size = 4000000;
        std::string name = {};
    };

    struct TaskSubmitInfo
    {
        PipelineStageFlags * additional_src_stages = {};
        std::vector<CommandList> * additional_command_lists = {};
        std::vector<BinarySemaphore> * additional_wait_binary_semaphores = {};
        std::vector<BinarySemaphore> * additional_signal_binary_semaphores = {};
        std::vector<std::pair<TimelineSemaphore, u64>> * additional_wait_timeline_semaphores = {};
        std::vector<std::pair<TimelineSemaphore, u64>> * additional_signal_timeline_semaphores = {};
    };

    struct TaskPresentInfo
    {
        std::vector<BinarySemaphore> * additional_binary_semaphores = {};
    };

    struct TaskCompleteInfo
    {
    };

    struct TaskImageLastUse
    {
        ImageMipArraySlice slice = {};
        ImageLayout layout = {};
        Access access = {};
    };

    struct TaskListConditionalInfo
    {
        u32 condition_index = {};
        std::function<void()> when_true = {};
        std::function<void()> when_false = {};
    };

    struct ExecutionInfo
    {
        std::span<bool> permutation_condition_values = {};
        bool record_debug_string = {};
    };

    struct PersistentTaskBuffer : ManagedPtr
    {
        PersistentTaskBuffer() = default;
        PersistentTaskBuffer(TaskBufferInfo const & info);

        operator TaskBufferId() const;

        auto id() const -> TaskBufferId;
        auto info() const -> TaskBufferInfo const &;

        auto get_buffer(usize index = 0) -> BufferId &;
        auto get_buffer_count() const -> usize;
        void add_buffer(BufferId id);
        void clear_buffers();
        void remove_buffer(BufferId id);
    };

    struct PersistentTaskImage : ManagedPtr
    {
        PersistentTaskImage() = default;
        PersistentTaskImage(TaskImageInfo const & info);

        operator TaskImageId() const;

        auto id() const -> TaskImageId;
        auto info() const -> TaskImageInfo const &;

        auto get_image(usize index = 0) -> ImageId &;
        auto get_image_count() const -> usize;
        void add_image(ImageId id);
        void clear_images();
        void remove_image(ImageId id);
    };

    struct TaskList : ManagedPtr
    {
        TaskList() = default;

        TaskList(TaskListInfo const & info);
        ~TaskList();

        auto create_transient_task_buffer(TaskBufferInfo const & info) -> TaskBufferId;
        auto use_persistent_buffer(PersistentTaskBuffer const & buffer) -> TaskBufferId;
        auto create_transient_task_image(TaskImageInfo const & info) -> TaskImageId;
        auto use_persistent_image(PersistentTaskImage const & image) -> TaskImageId;

        void conditional(TaskListConditionalInfo const & conditional_info);

        void add_task(TaskInfo const & info);

        void submit(TaskSubmitInfo const & info);
        void present(TaskPresentInfo const & info);

        void complete(TaskCompleteInfo const & info);

        void add_runtime_buffer(TaskBufferId tid, BufferId id);
        void add_runtime_image(TaskImageId tid, ImageId id);
        void remove_runtime_buffer(TaskBufferId tid, BufferId id);
        void remove_runtime_image(TaskImageId tid, ImageId id);
        void clear_runtime_buffers(TaskBufferId tid);
        void clear_runtime_images(TaskImageId tid);

        void execute(ExecutionInfo const & info);
        // All tasks recorded AFTER a submit will not be executied and submitted.
        // The resulting command lists can retrieved wit this function.
        auto get_command_lists() -> std::vector<CommandList>;
        // Returns a debug string describing the used permutation and synch.
        auto get_debug_string() -> std::string;

        void output_graphviz();
    };
} // namespace daxa
