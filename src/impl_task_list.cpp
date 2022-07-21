#include "impl_task_list.hpp"

namespace daxa
{
    auto TaskGPUResourceId::is_empty() const -> bool
    {
        return index == std::numeric_limits<u32>::max();
    }

    TaskList::TaskList() : Handle{nullptr} {}

    TaskList::~TaskList() {}

    TaskList::TaskList(std::shared_ptr<void> a_impl) : Handle{std::move(a_impl)} {}

    auto TaskList::create_task_buffer(TaskBufferInfo const & callback) -> TaskBufferId
    {
        return {};
    }

    auto TaskList::create_task_image(TaskImageInfo const & callback) -> TaskImageId
    {
        return {};
    }

    auto TaskList::create_task_image_view(TaskImageViewInfo const & callback) -> TaskImageViewId
    {
        return {};
    }

    
    void TaskList::add_task(TaskInfo const & info)
    {

    }

    void TaskList::add_render_task(TaskRenderInfo const & info)
    {
        auto & impl = *reinterpret_cast<ImplTaskList *>(this->impl.get());

        auto convert_attachment = [&](TaskRenderAttachmentInfo const & attach, auto & images, auto & images_view, auto & access) -> RenderAttachmentInfo
        {
            ImageLayout layout =
                attach.layout_override != ImageLayout::UNDEFINED ? attach.layout_override : ImageLayout::COLOR_ATTACHMENT_OPTIMAL;
            if (!attach.image.is_empty())
            {
                images.push_back({access, attach.image});
                return RenderAttachmentInfo{
                    .image = impl.get_image(attach.image),
                    .layout = layout,
                    .load_op = attach.load_op,
                    .store_op = attach.store_op,
                    .clear_value = attach.clear_value,
                };
            }
            else if (!attach.image_view.is_empty())
            {
                images_view.push_back({access, attach.image_view});
                return RenderAttachmentInfo{
                    .image_view = impl.get_image_view(attach.image_view),
                    .layout = layout,
                    .load_op = attach.load_op,
                    .store_op = attach.store_op,
                    .clear_value = attach.clear_value,
                };
            }
            else
            {
                DAXA_DBG_ASSERT_TRUE_M(false, "must declare image or image view for render pass attachment");
                return {};
            }
        };

        TaskInfo task = {};
        std::vector<RenderAttachmentInfo> color_attachments = {};
        task.resources = info.resources;
        for (auto const & attach : info.render_info.color_attachments)
        {
            color_attachments.push_back(convert_attachment(attach, task.resources.images, task.resources.image_views, daxa::AccessFlagBits::COLOR_ATTACHMENT_OUTPUT_WRITE));
        }
        std::optional<RenderAttachmentInfo> depth_attach = {};
        if (info.render_info.depth_attachment.has_value())
        {
            depth_attach = convert_attachment(
                info.render_info.depth_attachment.value(),
                task.resources.images,
                task.resources.image_views,
                daxa::AccessFlagBits::EARLY_FRAGMENT_TESTS_READ);
        }
        std::optional<RenderAttachmentInfo> stencil_attach = {};
        if (info.render_info.stencil_attachment.has_value())
        {
            stencil_attach = convert_attachment(
                info.render_info.stencil_attachment.value(),
                task.resources.images,
                task.resources.image_views,
                daxa::AccessFlagBits::EARLY_FRAGMENT_TESTS_READ);
        }
        task.task = [=](TaskInterface & interf)
        {
            interf.cmd_list.begin_renderpass({
                .color_attachments = color_attachments,
                .depth_attachment = depth_attach,
                .stencil_attachment = stencil_attach,
                .render_area = info.render_info.render_area,
            });
            info.task(interf);
            interf.cmd_list.end_renderpass();
        };

        add_task(task);
    }

    void TaskList::add_present(TaskImageId const & id)
    {

    }

    void TaskList::add_present(TaskImageViewId const & id)
    {

    }

    void TaskList::compile()
    {

    }

    void TaskList::execute()
    {

    }

    ImplTaskList::ImplTaskList(std::shared_ptr<ImplDevice> a_impl_device, TaskListInfo const & info)
        : impl_device{std::move(a_impl_device)}, info{info}
    {
    }

    ImplTaskList::~ImplTaskList() 
    {
        
    }
    
    auto ImplTaskList::get_buffer(BufferId) -> BufferId
    {
        return {};
    }
    
    auto ImplTaskList::get_image(TaskImageId) -> ImageId
    {
        return {};
    }
    
    auto ImplTaskList::get_image_view(TaskImageViewId) -> ImageViewId
    {
        return {};
    }

} // namespace daxa