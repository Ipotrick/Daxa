#pragma once

#include <daxa/core.hpp>

#include <daxa/command_list.hpp>

namespace daxa
{
    struct TaskGPUResourceId
    {
        u32 index = {};

        auto is_empty() const -> bool;
    };

    struct TaskBufferId : public TaskGPUResourceId
    {
    };

    struct TaskImageId : public TaskGPUResourceId
    {
    };

    struct TaskImageViewId : public TaskGPUResourceId
    {
    };

    struct TaskResources
    {
        std::vector<std::pair<AccessFlags, TaskBufferId>> buffers = {};
        std::vector<std::pair<AccessFlags, TaskImageId>> images = {};
        std::vector<std::pair<AccessFlags, TaskImageViewId>> image_views = {};
    };

    struct TaskInterface
    {
        Device & device;
        CommandList & cmd_list;
        TaskResources & resources;
        auto get_buffer(TaskBufferId const & task_id) -> BufferId;
        auto get_image(TaskImageId const & task_id) -> ImageId;
        auto get_image_view(TaskImageViewId const & task_id) -> ImageViewId;
    };

    using TaskCallback = std::function<void(TaskInterface &)>;
    using CreateTaskBufferCallback = std::function<BufferId(TaskInterface &)>;
    using CreateTaskImageCallback = std::function<ImageId(TaskInterface &)>;
    using CreateTaskImageViewCallback = std::function<ImageViewId(TaskInterface &)>;

    struct TaskBufferInfo
    {
        CreateTaskBufferCallback fetch_callback = {};
    };

    struct TaskImageInfo
    {
        CreateTaskImageCallback fetch_callback = {};
    };

    struct TaskImageViewInfo
    {
        CreateTaskImageViewCallback fetch_callback = {};
    };

    struct TaskInfo
    {
        TaskResources resources = {};
        TaskCallback task = {};
    };

    struct TaskRenderAttachmentInfo
    {
        TaskImageId image = {};
        TaskImageViewId image_view{};
        // optional field:
        ImageLayout layout_override = {};
        AttachmentLoadOp load_op = AttachmentLoadOp::DONT_CARE;
        AttachmentStoreOp store_op = AttachmentStoreOp::STORE;
        ClearValue clear_value = {};
    };

    struct TaskRenderPassBeginInfo
    {
        std::vector<TaskRenderAttachmentInfo> color_attachments = {};
        std::optional<TaskRenderAttachmentInfo> depth_attachment = {};
        std::optional<TaskRenderAttachmentInfo> stencil_attachment = {};
        Rect2D render_area = {};
    };

    struct TaskRenderInfo
    {
        TaskRenderPassBeginInfo render_info = {};
        TaskResources resources = {};
        TaskCallback task = {};
    };

    struct CommandSubmitInfo;
    struct PresentInfo;

    struct TaskListInfo
    {
        std::string debug_name = {};
    };

    struct TaskList : Handle
    {
        TaskList();
        ~TaskList();

        auto create_task_buffer(TaskBufferInfo const & info) -> TaskBufferId;
        auto create_task_image(TaskImageInfo const & info) -> TaskImageId;
        auto create_task_image_view(TaskImageViewInfo const & info) -> TaskImageViewId;

        void add_task(TaskInfo const & info);
        void add_render_task(TaskRenderInfo const & info);

        void add_present(TaskImageId const & id);
        void add_present(TaskImageViewId const & id);

        void compile();
        void execute();

      private:
        friend struct Device;
        TaskList(std::shared_ptr<void> impl);
    };
} // namespace daxa
