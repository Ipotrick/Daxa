#pragma once

#include <daxa/daxa.hpp>
#include <variant>

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
        ///         This is useful to create permutations of a task graph without having to create a separate task graph.
        ///         Another benefit is that task graph can generate synch between executions of permutations while it can not generate synch between two separate task graphs.
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

    /*
         __/\__
    . _  \\''//
    -( )-/_||_\
     .'. \_()_/
      |  | . \
      |  | .  \
    . '. \_____\.
    */

    struct InlineTaskInfo
    {
        std::vector<TaskAttachment> attachments = {};
        std::function<void(TaskInterface const &)> task = {};
        char const * name = "unnamed";
    };

    struct InlineTask : ITask
    {
        InlineTask(InlineTaskInfo const & info)
        {
            _attachments = info.attachments;
            _callback = info.task;
            _name = info.name;
        }
        constexpr virtual auto _raw_attachments() -> std::span<TaskAttachment> override
        {
            return _attachments;
        }
        constexpr virtual auto _raw_attachments() const -> std::span<TaskAttachment const> override
        {
            return _attachments;
        }
        constexpr virtual char const * name() const override { return _name; };
        virtual void callback(TaskInterface ti) const override
        {
            _callback(ti);
        };

      private:
        std::vector<TaskAttachment> _attachments = {};
        std::function<void(TaskInterface)> _callback = {};
        char const * _name = {};
    };

    struct ImplTaskGraph;

    struct TaskGraph : ManagedPtr<TaskGraph, ImplTaskGraph *>
    {
        TaskGraph() = default;

        DAXA_EXPORT_CXX TaskGraph(TaskGraphInfo const & info);
        DAXA_EXPORT_CXX ~TaskGraph();

        DAXA_EXPORT_CXX void use_persistent_buffer(TaskBuffer const & buffer);
        DAXA_EXPORT_CXX void use_persistent_image(TaskImage const & image);

        DAXA_EXPORT_CXX auto create_transient_buffer(TaskTransientBufferInfo const & info) -> TaskBufferView;
        DAXA_EXPORT_CXX auto create_transient_image(TaskTransientImageInfo const & info) -> TaskImageView;

        template <typename TTask>
            requires std::is_base_of_v<ITask, TTask>
        void add_task(TTask const& task)
        {
            // DAXA_DBG_ASSERT_TRUE_M(task._raw_attachments().size() == task._raw_attachments()._offset, "Detected task attachment count differing from Task head declared attachment count!");
            add_task(std::unique_ptr<ITask>(new std::remove_reference_t<TTask>(task)));
        }
        void add_task(InlineTaskInfo const & inline_task_info)
        {
            add_task(std::unique_ptr<ITask>(new InlineTask(inline_task_info)));
        }

        DAXA_EXPORT_CXX void conditional(TaskGraphConditionalInfo const & conditional_info);
        DAXA_EXPORT_CXX void submit(TaskSubmitInfo const & info);
        DAXA_EXPORT_CXX void present(TaskPresentInfo const & info);

        // TODO: make move only. Return ExecutableTaskGraph.
        DAXA_EXPORT_CXX void complete(TaskCompleteInfo const & info);

        DAXA_EXPORT_CXX void execute(ExecutionInfo const & info);

        DAXA_EXPORT_CXX auto get_debug_string() -> std::string;
        DAXA_EXPORT_CXX auto get_transient_memory_size() -> daxa::usize;

      protected:
        template <typename T, typename H_T>
        friend struct ManagedPtr;
        DAXA_EXPORT_CXX static auto inc_refcnt(ImplHandle const * object) -> u64;
        DAXA_EXPORT_CXX static auto dec_refcnt(ImplHandle const * object) -> u64;

      private:
        DAXA_EXPORT_CXX void add_task(std::unique_ptr<ITask> && task);
    };
} // namespace daxa
