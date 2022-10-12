#pragma once

#if !DAXA_BUILT_WITH_UTILS
#error "[package management error] You must build Daxa with the UTILS option enabled"
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

    using UsedTaskBuffers = std::vector<TaskBufferUse>;
    using UsedTaskImages = std::vector<TaskImageUse>;

    struct TaskList;
    struct Device;

    struct TaskRuntime
    {
        auto get_device() const -> Device &;
        auto get_command_list() const -> CommandList;
        auto get_used_task_buffers() const -> UsedTaskBuffers const &;
        auto get_used_task_images() const -> UsedTaskImages const &;
        auto get_buffer(TaskBufferId const & task_resource_id) const -> BufferId;
        auto get_image(TaskImageId const & task_resource_id) const -> ImageId;

      private:
        friend struct ImplTaskRuntime;
        friend struct TaskList;
        TaskRuntime(void * a_backend);
        void * backend;
    };

    using TaskCallback = std::function<void(TaskRuntime const &)>;

    struct TaskBufferInfo
    {
        BufferId * buffer = {};
        Access initial_access = AccessConsts::NONE;
        std::string debug_name = {};
    };

    struct TaskImageInfo
    {
        ImageId * image = {};
        Access initial_access = AccessConsts::NONE;
        ImageLayout initial_layout = ImageLayout::UNDEFINED;
        bool swapchain_image = false;
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
        bool dont_reorder_tasks = false;
        /// @brief Some drivers have bad implementations for split barriers.
        /// If that is the case for you, you can turn off all use of split barriers.
        /// Daxa will use pipeline barriers instead if this is set.
        bool dont_use_split_barriers = false;
        /// @brief Optionally the user can provide a swapchain. This enables the use of present.
        std::optional<Swapchain> swapchain = {};
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

    struct TaskList : ManagedPtr
    {
        TaskList() = default;

        TaskList(TaskListInfo const & info);
        ~TaskList();

        auto create_task_buffer(TaskBufferInfo const & info) -> TaskBufferId;
        auto create_task_image(TaskImageInfo const & info) -> TaskImageId;

        void add_task(TaskInfo const & info);

        void submit(CommandSubmitInfo * info);
        void present(TaskPresentInfo const & info);

        void complete();
        void execute();
        auto get_command_lists() -> std::vector<CommandList>;

        void output_graphviz();
        void debug_print();

        auto last_access(TaskBufferId buffer) -> Access;
        auto last_uses(TaskImageId image) -> std::vector<TaskImageLastUse>;
    };
} // namespace daxa
