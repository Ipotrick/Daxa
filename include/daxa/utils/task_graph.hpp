#pragma once

#include <daxa/daxa.hpp>

#include "task_graph.inl"

#if !DAXA_BUILT_WITH_UTILS_TASK_GRAPH
#error "[package management error] You must build Daxa with the DAXA_ENABLE_UTILS_TASK_GRAPH CMake option enabled, or request the utils-task-graph feature in vcpkg"
#endif

#include "mem.hpp"

namespace daxa
{
    struct TaskGraph;
    struct Device;
    struct CommandSubmitInfo;
    struct PresentInfo;

    using TaskInputDefaultT = std::span<GenericTaskResourceUse>;

    struct TaskInterfaceUses
    {
        auto operator[](TaskBufferView const & handle) const -> TaskBufferUse<> const &;
        auto operator[](TaskImageView const & handle) const -> TaskImageUse<> const &;
        auto get_uniform_buffer_info() const -> SetUniformBufferInfo;
      protected:
        friend struct ImplTaskRuntimeInterface;
        friend struct TaskGraph;
        friend struct ImplTaskGraph;
        friend struct TaskInterface;
        TaskInterfaceUses(void * a_backend);
        void * backend = {};
    };

    struct TaskInterface
    {
        auto get_device() const -> Device &;
        auto get_recorder() const -> CommandRecorder &;
        auto get_allocator() const -> TransferMemoryPool &;

        TaskInterfaceUses uses;

      protected:
        friend struct ImplTaskRuntimeInterface;
        friend struct TaskGraph;
        friend struct ImplTaskGraph;
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
        Extent3D size = {0, 0, 0};
        u32 mip_level_count = 1;
        u32 array_layer_count = 1;
        u32 sample_count = 1;
        std::string name = {};
    };

    struct TaskImageAliasInfo
    {
        std::string alias = {};
        Variant<TaskImageView, std::string> aliased_image = {};
        u32 base_mip_level_offset = {};
        u32 base_array_layer_offset = {};
    };

    struct TaskBufferAliasInfo
    {
        std::string alias = {};
        Variant<TaskBufferView, std::string> aliased_buffer = {};
    };

    template <typename TaskArgs>
    struct TaskInfo
    {
        TaskArgs args = {};
        TaskCallback task = {};
        std::string name = {};
    };

    struct TaskGraphInfo
    {
        Device device = {};
        /// @brief  Optionally the user can provide a swapchain. This enables the use of present.
        std::optional<Swapchain> swapchain = {};
        /// @brief  Task reordering can drastically improve performance,
        ///         yet is it also nice to have sequential callback execution.
        bool reorder_tasks = true;
        /// @brief  Allows task graph to alias transient resources memory (ofc only when that wont break the program) 
        bool alias_transients = {};
        /// @brief  Some drivers have bad implementations for split barriers.
        ///         If that is the case for you, you can turn off all use of split barriers.
        ///         Daxa will use pipeline barriers instead if this is set.
        bool use_split_barriers = true;
        /// @brief  Each condition doubled the number of permutations.
        ///         For a low number of permutations its is preferable to precompile all permutations.
        ///         For a large number of permutations it might be preferable to only create the permutations actually used on the fly just before they are needed.
        ///         The second option is enabled by using jit (just in time) compilation.
        bool jit_compile_permutations = {};
        /// @brief  Task graph can branch the execution based on conditionals. All conditionals must be set before execution and stay constant while executing.
        ///         This is usefull to create permutations of a task graph without having to create a seperate task graph.
        ///         Another benefit is that task graph can generate synch between executions of permutations while it can not generate synch between two seperate task graphs.
        usize permutation_condition_count = {};
        /// @brief  Task graph will put performance markers that are used by profilers like nsight around each tasks execution by default.
        bool enable_command_labels = true;
        std::array<f32, 4> task_graph_label_color = {0.463f, 0.333f, 0.671f, 1.0f};
        std::array<f32, 4> task_batch_label_color = {0.563f, 0.433f, 0.771f, 1.0f};
        std::array<f32, 4> task_label_color = {0.663f, 0.533f, 0.871f, 1.0f};
        /// @brief  Records debug information about the execution if enabled. This string is retrievable with the function get_debug_string.
        bool record_debug_information = {};
        /// @brief  Sets the size of the linear allocator of device local, host visible memory used by the linear staging allocator.
        ///         This memory is used internally as well as by tasks via the TaskInterface::get_allocator().
        ///         Setting the size to 0, disables a few task list features but also eliminates the memory allocation.
        u32 staging_memory_pool_size = 262'144; // 2^16 bytes.
        std::string name = {};
    };

    struct TaskSubmitInfo
    {
        PipelineStageFlags * additional_src_stages = {};
        std::vector<ExecutableCommandList> * additional_command_lists = {};
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

    struct TaskGraphConditionalInfo
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

    struct InlineTaskInfo
    {
        std::vector<GenericTaskResourceUse> uses = {};
        TaskCallback task = {};
        isize constant_buffer_slot = -1;
        std::string name = {};
    };

    struct ImplTaskGraph;

    struct TaskGraph : ManagedPtr<TaskGraph, ImplTaskGraph*>
    {
        TaskGraph() = default;

        TaskGraph(TaskGraphInfo const & info);
        ~TaskGraph();

        void use_persistent_buffer(TaskBuffer const & buffer);
        void use_persistent_image(TaskImage const & image);

        auto create_transient_buffer(TaskTransientBufferInfo const & info) -> TaskBufferView;
        auto create_transient_image(TaskTransientImageInfo const & info) -> TaskImageView;

        template <typename Task>
        void add_task(Task const & task)
        {
            std::unique_ptr<detail::BaseTask> base_task = std::make_unique<detail::PredeclaredTask<Task>>(task);
            add_task(std::move(base_task));
        }

        void add_task(InlineTaskInfo && info)
        {
            std::unique_ptr<detail::BaseTask> base_task = std::make_unique<detail::InlineTask>(
                std::move(info.uses),
                std::move(info.task),
                std::move(info.name),
                info.constant_buffer_slot);
            add_task(std::move(base_task));
        }

        void add_preamble(TaskCallback callback);

        void conditional(TaskGraphConditionalInfo const & conditional_info);
        void submit(TaskSubmitInfo const & info);
        void present(TaskPresentInfo const & info);

        // TODO: make move only. Return ExecutableTaskGraph.
        void complete(TaskCompleteInfo const & info);

        void execute(ExecutionInfo const & info);
        // TODO: Reimplement in another way.
        // auto get_command_lists() -> std::vector<CommandRecorder>;

        auto get_debug_string() -> std::string;
        auto get_transient_memory_size() -> daxa::usize;
        
      protected:
        template <typename T, typename H_T>
        friend struct ManagedPtr;
        static auto inc_refcnt(ImplHandle const * object) -> u64;
        static auto dec_refcnt(ImplHandle const * object) -> u64;

      private:
        void add_task(std::unique_ptr<detail::BaseTask> && base_task);
    };
} // namespace daxa
