#include "impl_core.hpp"

#include <daxa/task_list.hpp>

namespace daxa
{
    struct ImplDevice;

    struct ImplTaskList
    {
        std::shared_ptr<ImplDevice> impl_device = {};
        TaskListInfo info = {};

        auto get_buffer(BufferId) -> BufferId;
        auto get_image(TaskImageId) -> ImageId;
        auto get_image_view(TaskImageViewId) -> ImageViewId;

        ImplTaskList(std::shared_ptr<ImplDevice> a_impl_device, TaskListInfo const & info);
        ~ImplTaskList();
    };
} // namespace daxa