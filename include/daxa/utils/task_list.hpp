#pragma once

#if !DAXA_BUILT_WITH_UTILS_TASK_LIST
#error "[package management error] You must build Daxa with the DAXA_ENABLE_UTILS_TASK_LIST CMake option enabled, or request the utils-task-list feature in vcpkg"
#endif

#include <span>

#include <daxa/core.hpp>
#include <daxa/device.hpp>

namespace daxa
{
    enum struct TaskBufferAccess
    {
        NONE,
        SHADER_READ_ONLY,
        VERTEX_SHADER_READ_ONLY,
        TESSELLATION_CONTROL_SHADER_READ_ONLY,
        TESSELLATION_EVALUATION_SHADER_READ_ONLY,
        GEOMETRY_SHADER_READ_ONLY,
        FRAGMENT_SHADER_READ_ONLY,
        COMPUTE_SHADER_READ_ONLY,
        SHADER_WRITE_ONLY,
        VERTEX_SHADER_WRITE_ONLY,
        TESSELLATION_CONTROL_SHADER_WRITE_ONLY,
        TESSELLATION_EVALUATION_SHADER_WRITE_ONLY,
        GEOMETRY_SHADER_WRITE_ONLY,
        FRAGMENT_SHADER_WRITE_ONLY,
        COMPUTE_SHADER_WRITE_ONLY,
        SHADER_READ_WRITE,
        VERTEX_SHADER_READ_WRITE,
        TESSELLATION_CONTROL_SHADER_READ_WRITE,
        TESSELLATION_EVALUATION_SHADER_READ_WRITE,
        GEOMETRY_SHADER_READ_WRITE,
        FRAGMENT_SHADER_READ_WRITE,
        COMPUTE_SHADER_READ_WRITE,
        TRANSFER_READ,
        TRANSFER_WRITE,
        HOST_TRANSFER_READ,
        HOST_TRANSFER_WRITE,
    };

    auto to_string(TaskBufferAccess const & usage) -> std::string_view;

    enum struct TaskImageAccess
    {
        NONE,
        SHADER_READ_ONLY,
        VERTEX_SHADER_READ_ONLY,
        TESSELLATION_CONTROL_SHADER_READ_ONLY,
        TESSELLATION_EVALUATION_SHADER_READ_ONLY,
        GEOMETRY_SHADER_READ_ONLY,
        FRAGMENT_SHADER_READ_ONLY,
        COMPUTE_SHADER_READ_ONLY,
        SHADER_WRITE_ONLY,
        VERTEX_SHADER_WRITE_ONLY,
        TESSELLATION_CONTROL_SHADER_WRITE_ONLY,
        TESSELLATION_EVALUATION_SHADER_WRITE_ONLY,
        GEOMETRY_SHADER_WRITE_ONLY,
        FRAGMENT_SHADER_WRITE_ONLY,
        COMPUTE_SHADER_WRITE_ONLY,
        SHADER_READ_WRITE,
        VERTEX_SHADER_READ_WRITE,
        TESSELLATION_CONTROL_SHADER_READ_WRITE,
        TESSELLATION_EVALUATION_SHADER_READ_WRITE,
        GEOMETRY_SHADER_READ_WRITE,
        FRAGMENT_SHADER_READ_WRITE,
        COMPUTE_SHADER_READ_WRITE,
        TRANSFER_READ,
        TRANSFER_WRITE,
        COLOR_ATTACHMENT,
        DEPTH_ATTACHMENT,
        STENCIL_ATTACHMENT,
        DEPTH_STENCIL_ATTACHMENT,
        DEPTH_ATTACHMENT_READ_ONLY,
        STENCIL_ATTACHMENT_READ_ONLY,
        DEPTH_STENCIL_ATTACHMENT_READ_ONLY,
        RESOLVE_WRITE,
        PRESENT,
    };

    auto to_string(TaskImageAccess const & usage) -> std::string_view;

    struct TaskGPUResourceId
    {
        u32 index = std::numeric_limits<u32>::max();

        auto is_empty() const -> bool;

        auto operator<=>(TaskGPUResourceId const & other) const = default;
    };

    struct TaskBufferId : public TaskGPUResourceId
    {
    };

    struct TaskImageId : public TaskGPUResourceId
    {
    };

    struct TaskBufferUse
    {
        TaskBufferId id = {};
        TaskBufferAccess access = {};
    };

    struct TaskImageUse
    {
        TaskImageId id = {};
        TaskImageAccess access = {};
        ImageMipArraySlice slice = {};
    };

    struct InitialTaskImageUse
    {
        Access access = {};
        ImageLayout layout = {};
        ImageMipArraySlice slice = {};
    };

    using UsedTaskBuffers = std::vector<TaskBufferUse>;
    using UsedTaskImages = std::vector<TaskImageUse>;

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
        void * backend;
    };

    using TaskCallback = std::function<void(TaskRuntimeInterface const &)>;

    struct TaskBufferInfo
    {
        // For execution_persistent resources, task list will synch from the initial use to the first use ONCE.
        // After the FIRST execution, it will use the runtime state of the resource.
        // For non-execution_persistent resources, task list will synch from the initial use to first use EVERY EXECUTION.
        Access initial_access = AccessConsts::NONE;
        bool execution_persistent = {};
        std::span<BufferId> execution_buffers = {};
        std::string debug_name = {};
    };

    struct TaskImageInfo
    {
        // For execution_persistent resources, task list will synch from the initial use to the first use ONCE.
        // After the FIRST execution, it will use the runtime state of the resource.
        // For non-execution_persistent resources, task list will synch from the initial use to first use EVERY EXECUTION.
        std::span<InitialTaskImageUse> initial_access = {};
        bool execution_persistent = {};
        bool swapchain_image = {};
        std::span<ImageId> execution_images = {};
        std::string debug_name = {};
    };

    struct TaskInfo
    {
        UsedTaskBuffers used_buffers = {};
        UsedTaskImages used_images = {};
        TaskCallback task = {};
        std::string debug_name = {};
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
        std::string debug_name = {};
    };

    struct TaskPresentInfo
    {
        std::vector<BinarySemaphore> * user_binary_semaphores = {};
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
        bool record_debug_string  = {};
    };

    struct TaskList : ManagedPtr
    {
        TaskList() = default;

        TaskList(TaskListInfo const & info);
        ~TaskList();

        auto create_task_buffer(TaskBufferInfo const & info) -> TaskBufferId;
        auto create_task_image(TaskImageInfo const & info) -> TaskImageId;

        void conditional(TaskListConditionalInfo const & conditional_info);

        void add_task(TaskInfo const & info);

        void submit(CommandSubmitInfo * info);
        void present(TaskPresentInfo const & info);

        void complete();

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
