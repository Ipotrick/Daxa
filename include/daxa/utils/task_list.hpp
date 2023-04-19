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

    struct GenericTaskInterface
    {
        auto get_device() const -> Device &;
        auto get_command_list() const -> CommandList;
        auto get_allocator() const -> TransferMemoryPool &;

        auto buffer(TaskBufferId const & task_resource_id, usize index = 0) const -> BufferId;
        auto image(TaskImageId const & task_resource_id, usize index = 0) const -> ImageId;
        auto view(TaskImageId const & task_resource_id, usize index = 0) const -> ImageViewId;

      protected:
        friend struct ImplTaskRuntimeInterface;
        friend struct TaskList;
        friend struct ImplTaskList;
        template <typename TaskInput>
        friend struct TaskInterface;
        GenericTaskInterface(void * a_backend);
        auto get_args() const -> std::span<GenericTaskResourceUse>;
        void * backend = {};
    };

    template <typename TaskInput = TaskInputDefaultT>
    struct TaskInterface final : public GenericTaskInterface
    {
        auto input() const -> TaskInput const &
            requires(!std::is_same_v<TaskInput, TaskInputDefaultT>)
        {
            return *reinterpret_cast<TaskInput const *>(get_args().data());
        }

        template <typename InputType>
            requires(std::is_same_v<TaskInput, TaskInputDefaultT>)
        auto input_as(usize index) const -> InputType const &
        {
            DAXA_DBG_ASSERT_TRUE_M(index < get_args().size(), "detected out of bounds input index! all task input indices must be smaller then the count of inputs");
            DAXA_DBG_ASSERT_TRUE_M(InputType::INPUT_TYPE == get_args().at(index).type, "detected invalid input type cast! the cast input type must match the declared input type at any given index.");
            return *reinterpret_cast<InputType const *>(get_args().data() + index);
        }

        auto operator->() const -> TaskInput const * requires(!std::is_same_v<TaskInput, TaskInputDefaultT>) {
            return reinterpret_cast<TaskInput const *>(get_args().data());
        }

        template <typename T>
        operator TaskInterface<T> const &() const
        {
            return *reinterpret_cast<TaskInterface<T> const *>(this);
        }

      private:
        friend struct ImplTaskRuntimeInterface;
        friend struct TaskList;
        friend struct ImplTaskList;
        friend struct TaskInterface;
        TaskInterface(void * a_backend) : GenericTaskInterface(a_backend) {}
    };

    using TaskCallback = std::function<void(TaskInterface<> const &)>;

    struct TransientBufferInfo
    {
        MemoryFlags memory_flags = {};
        u32 size = {};
        std::string name = {};
    };

    struct TransientImageInfo
    {
        u32 dimensions = 2;
        Format format = Format::R8G8B8A8_UNORM;
        ImageAspectFlags aspect = ImageAspectFlagBits::COLOR;
        Extent3D size = {0, 0, 0};
        u32 mip_level_count = 1;
        u32 array_layer_count = 1;
        u32 sample_count = 1;
        MemoryFlags memory_flags = {};
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

        operator TaskBufferId() const;

        auto id() const -> TaskBufferId;
        auto info() const -> TaskBufferInfo const &;
        auto get_buffers() const -> TrackedBuffers;

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

        operator TaskImageId() const;

        auto id() const -> TaskImageId;
        auto info() const -> TaskImageInfo const &;
        auto get_images() const -> TrackedImages;

        void set_images(TrackedImages const & images);
        void swap_images(TaskImage & other);
    };

    struct GenericTaskInfo
    {
        i32 shader_binding = -1;
        u32 shader_constant_buffer_size = {};
        std::vector<u32> shader_constant_buffer_offsets = {};
        GenericTaskArgsContainer task_args = {};
        TaskCallback task = {};
        std::string name = {};
    };

    struct InlineTaskInfo
    {
        std::initializer_list<GenericTaskResourceUse> args = {};
        TaskCallback task = {};
        std::string name = {};
    };

    struct TaskList : ManagedPtr
    {
        TaskList() = default;

        TaskList(TaskListInfo const & info);
        ~TaskList();

        auto use_persistent_buffer(TaskBuffer const & buffer) -> TaskBufferId;
        auto use_persistent_image(TaskImage const & image) -> TaskImageId;

        auto create_transient_buffer(TransientBufferInfo const & info) -> TaskBufferId;
        auto create_transient_image(TransientImageInfo const & info) -> TaskImageId;

        template <typename TaskInput>
        void add_task(TaskInfo<TaskInput> const & info)
            requires requires(TaskInput a) { typename TaskInput::FIRST_DERIVED; } and
                     requires(TaskInput a) { TaskInput::SHADER_BINDING; } and
                     std::derived_from<TaskInput, TaskUses<typename TaskInput::FIRST_DERIVED, TaskInput::SHADER_BINDING>>
        {
            GenericTaskArgsContainer args = {};
            args.count = TaskInput::USE_COUNT;
            args.memory.resize(sizeof(TaskInput), 0);
            std::memcpy(args.memory.data(), &info.args, sizeof(TaskInput));

            isize const shader_binding = TaskInput::SHADER_BINDING;
            auto const shader_offset_size = get_task_arg_shader_offsets_size(args);

            add_task(GenericTaskInfo{
                .shader_binding = shader_binding,
                .shader_constant_buffer_size = shader_offset_size.second,
                .shader_constant_buffer_offsets = std::move(shader_offset_size.first),
                .task_args = std::move(args),
                .task = info.task,
                .name = info.name,
            });
        }
        void add_task(InlineTaskInfo const & info);

        void conditional(TaskListConditionalInfo const & conditional_info);
        void submit(TaskSubmitInfo const & info);
        void present(TaskPresentInfo const & info);

        void complete(TaskCompleteInfo const & info);

        void execute(ExecutionInfo const & info);
        auto get_command_lists() -> std::vector<CommandList>;

        auto get_debug_string() -> std::string;

      private:
        void add_task(GenericTaskInfo && generic_info);
    }; // namespace daxa
} // namespace daxa
