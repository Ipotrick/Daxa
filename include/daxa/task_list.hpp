#pragma once

#include <daxa/core.hpp>

#include <daxa/command_list.hpp>

namespace daxa
{
    struct TaskInterface
    {
    };

    struct TaskResources
    {
        std::vector<std::pair<ImageUsageFlags, ImageId>> images = {};
        std::vector<std::pair<ImageUsageFlags, ImageViewId>> image_views = {};
        std::vector<std::pair<BufferUsageFlags, BufferId>> buffers = {};
    };

    struct TaskInfo
    {
        TaskResources resources = {};
        std::function<void(TaskInterface &, CommandList &, TaskResources const &)> task = {};
    };

    struct RenderTaskInfo
    {
        RenderPassBeginInfo render_pass_info = {};
        TaskResources resources = {};
        std::function<void(TaskInterface &, CommandList &, TaskResources const &)> task = {};
    };

    struct CommandSubmitInfo;
    struct PresentInfo;

    struct TaskList : Handle
    {
        ~TaskList();

        // When the image layout is not anounced, it will be assumed to be undefined.
        void announce_image_layout(ImageId id, ImageLayout layout);
        // When the image layout is not anounced, it will be assumed to be undefined.
        void announce_image_view_layout(ImageViewId id, ImageLayout layout);

        void copy_buffer_to_buffer(BufferCopyInfo const & info);
        void copy_buffer_to_image(BufferImageCopy const & info);
        void copy_image_to_buffer(BufferImageCopy const & info);
        void copy_image_to_image(ImageCopyInfo const & info);
        void blit_image_to_image(ImageBlitInfo const & info);

        void clear_buffer(BufferClearInfo const & info);
        void clear_image(ImageClearInfo const & info);

        void add_task(TaskInfo const & info);
        void add_render_task(RenderTaskInfo const & ifno);

        void submit(CommandSubmitInfo const & info);

        void present(PresentInfo const & info);

        void complete();
        void compile();
        void execute();

      private:
        friend struct Device;
        TaskList(std::shared_ptr<void> impl);
    };
} // namespace daxa
