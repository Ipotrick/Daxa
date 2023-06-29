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
    struct CommandSubmitInfo;
    struct PresentInfo;

    using TaskInputDefaultT = std::span<GenericTaskResourceUse>;

    struct TaskInterfaceUses
    {
        auto operator[](TaskBufferHandle const & handle) const -> TaskBufferUse<> const &;
        auto operator[](TaskImageHandle const & handle) const -> TaskImageUse<> const &;
        auto get_uniform_buffer_info() const -> SetConstantBufferInfo;
      protected:
        friend struct ImplTaskRuntimeInterface;
        friend struct TaskList;
        friend struct ImplTaskList;
        friend struct TaskInterface;
        TaskInterfaceUses(void * a_backend);
        void * backend = {};
    };

    struct TaskInterface
    {
        auto get_device() const -> Device &;
        auto get_command_list() const -> CommandList;
        auto get_allocator() const -> TransferMemoryPool &;

        TaskInterfaceUses uses;

      protected:
        friend struct ImplTaskRuntimeInterface;
        friend struct TaskList;
        friend struct ImplTaskList;
        friend struct TaskInterface;
        TaskInterface(void * a_backend);
        void * backend = {};
    };

    using TaskCallback = std::function<void(TaskInterface const &)>;

    struct TaskTransientBufferInfo
    {
        u32 size = {};
        std::string name = {};
    };

    struct TaskTransientImageInfo
    {
        u32 dimensions = 2;
        Format format = Format::R8G8B8A8_UNORM;
        ImageAspectFlags aspect = ImageAspectFlagBits::COLOR;
        Extent3D size = {0, 0, 0};
        u32 mip_level_count = 1;
        u32 array_layer_count = 1;
        u32 sample_count = 1;
        std::string name = {};
    };

    struct TaskImageAliasInfo
    {
        std::string alias = {};
        std::variant<TaskImageHandle, std::string> aliased_image = {};
        u32 base_mip_level_offset = {};
        u32 base_array_layer_offset = {};
    };

    struct TaskBufferAliasInfo
    {
        std::string alias = {};
        std::variant<TaskBufferHandle, std::string> aliased_buffer = {};
    };

    template <typename TaskArgs>
    struct TaskInfo
    {
        TaskArgs args = {};
        TaskCallback task = {};
        std::string name = {};
    };

    struct TaskListInfo
    {
        Device device;
        /// @brief Optionally the user can provide a swapchain. This enables the use of present.
        std::optional<Swapchain> swapchain = {};
        /// @brief Task reordering can drastically improve performance,
        /// yet is it also nice to have sequential callback execution.
        bool reorder_tasks = true;
        /// @brief Some drivers have bad implementations for split barriers.
        /// If that is the case for you, you can turn off all use of split barriers.
        /// Daxa will use pipeline barriers instead if this is set.
        bool use_split_barriers = true;
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
        u32 staging_memory_pool_size = 262'144; // 2^16 bytes.
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

    struct TrackedBuffers
    {
        std::span<BufferId const> buffers = {};
        Access latest_access = {};
    };

    struct TaskBufferInfo
    {
        TrackedBuffers initial_buffers = {};
        std::string name = {};
    };

    struct TaskBuffer : ManagedPtr
    {
        TaskBuffer() = default;
        TaskBuffer(TaskBufferInfo const & info);

        operator TaskBufferHandle() const;

        auto handle() const -> TaskBufferHandle;
        auto info() const -> TaskBufferInfo const &;
        auto get_state() const -> TrackedBuffers;

        void set_buffers(TrackedBuffers const & buffers);
        void swap_buffers(TaskBuffer & other);
    };

    struct TrackedImages
    {
        std::span<ImageId const> images = {};
        // optional:
        std::span<ImageSliceState const> latest_slice_states = {};
    };

    struct TaskImageInfo
    {
        TrackedImages initial_images = {};
        bool swapchain_image = {};
        std::string name = {};
    };

    struct TaskImage : ManagedPtr
    {
        TaskImage() = default;
        TaskImage(TaskImageInfo const & info);

        operator TaskImageHandle() const;

        auto handle() const -> TaskImageHandle;
        auto info() const -> TaskImageInfo const &;
        auto get_state() const -> TrackedImages;

        void set_images(TrackedImages const & images);
        void swap_images(TaskImage & other);
    };

    struct InlineTaskInfo
    {
        std::vector<GenericTaskResourceUse> uses = {};
        TaskCallback task = {};
        isize constant_buffer_slot = -1;
        std::string name = {};
    };

    struct TaskList : ManagedPtr
    {
        TaskList() = default;

        TaskList(TaskListInfo const & info);
        ~TaskList();

        void use_persistent_buffer(TaskBuffer const & buffer);
        void use_persistent_image(TaskImage const & image);

        auto create_transient_buffer(TaskTransientBufferInfo const & info) -> TaskBufferHandle;
        auto create_transient_image(TaskTransientImageInfo const & info) -> TaskImageHandle;

        template <typename Task>
        void add_task(Task const & task)
        {
            std::unique_ptr<BaseTask> base_task = std::make_unique<PredeclaredTask<Task>>(task);
            add_task(std::move(base_task));
        }

        void add_task(InlineTaskInfo && info)
        {
            std::unique_ptr<BaseTask> base_task = std::make_unique<InlineTask>(
                std::move(info.uses),
                std::move(info.task),
                std::move(info.name),
                info.constant_buffer_slot);
            add_task(std::move(base_task));
        }

        void conditional(TaskListConditionalInfo const & conditional_info);
        void submit(TaskSubmitInfo const & info);
        void present(TaskPresentInfo const & info);

        void complete(TaskCompleteInfo const & info);

        void execute(ExecutionInfo const & info);
        auto get_command_lists() -> std::vector<CommandList>;

        auto get_debug_string() -> std::string;

      private:
        void add_task(std::unique_ptr<BaseTask> && base_task);
    };
} // namespace daxa
