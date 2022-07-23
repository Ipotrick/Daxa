#include "impl_task_list.hpp"
#include "impl_device.hpp"

namespace daxa
{
    auto TaskGPUResourceId::is_empty() const -> bool
    {
        return index == std::numeric_limits<u32>::max();
    }

    auto to_string(TaskBufferUsage const & usage) -> std::string_view
    {
        switch (usage)
        {
        case TaskBufferUsage::SHADER_READ_ONLY: return std::string_view{"SHADER_READ_ONLY"};
        case TaskBufferUsage::VERTEX_SHADER_READ_ONLY: return std::string_view{"VERTEX_SHADER_READ_ONLY"};
        case TaskBufferUsage::TESSELLATION_CONTROL_SHADER_READ_ONLY: return std::string_view{"TESSELLATION_CONTROL_SHADER_READ_ONLY"};
        case TaskBufferUsage::TESSELLATION_EVALUATION_SHADER_READ_ONLY: return std::string_view{"TESSELLATION_EVALUATION_SHADER_READ_ONLY"};
        case TaskBufferUsage::GEOMETRY_SHADER_READ_ONLY: return std::string_view{"GEOMETRY_SHADER_READ_ONLY"};
        case TaskBufferUsage::FRAGMENT_SHADER_READ_ONLY: return std::string_view{"FRAGMENT_SHADER_READ_ONLY"};
        case TaskBufferUsage::COMPUTE_SHADER_READ_ONLY: return std::string_view{"COMPUTE_SHADER_READ_ONLY"};
        case TaskBufferUsage::SHADER_WRITE_ONLY: return std::string_view{"SHADER_WRITE_ONLY"};
        case TaskBufferUsage::VERTEX_SHADER_WRITE_ONLY: return std::string_view{"VERTEX_SHADER_WRITE_ONLY"};
        case TaskBufferUsage::TESSELLATION_CONTROL_SHADER_WRITE_ONLY: return std::string_view{"TESSELLATION_CONTROL_SHADER_WRITE_ONLY"};
        case TaskBufferUsage::TESSELLATION_EVALUATION_SHADER_WRITE_ONLY: return std::string_view{"TESSELLATION_EVALUATION_SHADER_WRITE_ONLY"};
        case TaskBufferUsage::GEOMETRY_SHADER_WRITE_ONLY: return std::string_view{"GEOMETRY_SHADER_WRITE_ONLY"};
        case TaskBufferUsage::FRAGMENT_SHADER_WRITE_ONLY: return std::string_view{"FRAGMENT_SHADER_WRITE_ONLY"};
        case TaskBufferUsage::COMPUTE_SHADER_WRITE_ONLY: return std::string_view{"COMPUTE_SHADER_WRITE_ONLY"};
        case TaskBufferUsage::SHADER_READ_WRITE: return std::string_view{"SHADER_READ_WRITE"};
        case TaskBufferUsage::VERTEX_SHADER_READ_WRITE: return std::string_view{"VERTEX_SHADER_READ_WRITE"};
        case TaskBufferUsage::TESSELLATION_CONTROL_SHADER_READ_WRITE: return std::string_view{"TESSELLATION_CONTROL_SHADER_READ_WRITE"};
        case TaskBufferUsage::TESSELLATION_EVALUATION_SHADER_READ_WRITE: return std::string_view{"TESSELLATION_EVALUATION_SHADER_READ_WRITE"};
        case TaskBufferUsage::GEOMETRY_SHADER_READ_WRITE: return std::string_view{"GEOMETRY_SHADER_READ_WRITE"};
        case TaskBufferUsage::FRAGMENT_SHADER_READ_WRITE: return std::string_view{"FRAGMENT_SHADER_READ_WRITE"};
        case TaskBufferUsage::COMPUTE_SHADER_READ_WRITE: return std::string_view{"COMPUTE_SHADER_READ_WRITE"};
        case TaskBufferUsage::TRANSFER_READ: return std::string_view{"TRANSFER_READ"};
        case TaskBufferUsage::TRANSFER_WRTIE: return std::string_view{"TRANSFER_WRTIE"};
        default: DAXA_DBG_ASSERT_TRUE_M(false, "unreachable");
        }
    }

    auto to_string(TaskImageUsage const & usage) -> std::string_view
    {
        switch (usage)
        {
        case TaskImageUsage::SHADER_READ_ONLY: return std::string_view{"SHADER_READ_ONLY"};
        case TaskImageUsage::VERTEX_SHADER_READ_ONLY: return std::string_view{"VERTEX_SHADER_READ_ONLY"};
        case TaskImageUsage::TESSELLATION_CONTROL_SHADER_READ_ONLY: return std::string_view{"TESSELLATION_CONTROL_SHADER_READ_ONLY"};
        case TaskImageUsage::TESSELLATION_EVALUATION_SHADER_READ_ONLY: return std::string_view{"TESSELLATION_EVALUATION_SHADER_READ_ONLY"};
        case TaskImageUsage::GEOMETRY_SHADER_READ_ONLY: return std::string_view{"GEOMETRY_SHADER_READ_ONLY"};
        case TaskImageUsage::FRAGMENT_SHADER_READ_ONLY: return std::string_view{"FRAGMENT_SHADER_READ_ONLY"};
        case TaskImageUsage::COMPUTE_SHADER_READ_ONLY: return std::string_view{"COMPUTE_SHADER_READ_ONLY"};
        case TaskImageUsage::SHADER_WRITE_ONLY: return std::string_view{"SHADER_WRITE_ONLY"};
        case TaskImageUsage::VERTEX_SHADER_WRITE_ONLY: return std::string_view{"VERTEX_SHADER_WRITE_ONLY"};
        case TaskImageUsage::TESSELLATION_CONTROL_SHADER_WRITE_ONLY: return std::string_view{"TESSELLATION_CONTROL_SHADER_WRITE_ONLY"};
        case TaskImageUsage::TESSELLATION_EVALUATION_SHADER_WRITE_ONLY: return std::string_view{"TESSELLATION_EVALUATION_SHADER_WRITE_ONLY"};
        case TaskImageUsage::GEOMETRY_SHADER_WRITE_ONLY: return std::string_view{"GEOMETRY_SHADER_WRITE_ONLY"};
        case TaskImageUsage::FRAGMENT_SHADER_WRITE_ONLY: return std::string_view{"FRAGMENT_SHADER_WRITE_ONLY"};
        case TaskImageUsage::COMPUTE_SHADER_WRITE_ONLY: return std::string_view{"COMPUTE_SHADER_WRITE_ONLY"};
        case TaskImageUsage::SHADER_READ_WRITE: return std::string_view{"SHADER_READ_WRITE"};
        case TaskImageUsage::VERTEX_SHADER_READ_WRITE: return std::string_view{"VERTEX_SHADER_READ_WRITE"};
        case TaskImageUsage::TESSELLATION_CONTROL_SHADER_READ_WRITE: return std::string_view{"TESSELLATION_CONTROL_SHADER_READ_WRITE"};
        case TaskImageUsage::TESSELLATION_EVALUATION_SHADER_READ_WRITE: return std::string_view{"TESSELLATION_EVALUATION_SHADER_READ_WRITE"};
        case TaskImageUsage::GEOMETRY_SHADER_READ_WRITE: return std::string_view{"GEOMETRY_SHADER_READ_WRITE"};
        case TaskImageUsage::FRAGMENT_SHADER_READ_WRITE: return std::string_view{"FRAGMENT_SHADER_READ_WRITE"};
        case TaskImageUsage::COMPUTE_SHADER_READ_WRITE: return std::string_view{"COMPUTE_SHADER_READ_WRITE"};
        case TaskImageUsage::TRANSFER_READ: return std::string_view{"TRANSFER_READ"};
        case TaskImageUsage::TRANSFER_WRTIE: return std::string_view{"TRANSFER_WRTIE"};
        case TaskImageUsage::COLOR_ATTACHMENT: return std::string_view{"COLOR_ATTACHMENT"};
        case TaskImageUsage::DEPTH_ATTACHMENT: return std::string_view{"DEPTH_ATTACHMENT"};
        case TaskImageUsage::STENCIL_ATTACHMENT: return std::string_view{"STENCIL_ATTACHMENT"};
        case TaskImageUsage::DEPTH_STENCIL_ATTACHMENT: return std::string_view{"DEPTH_STENCIL_ATTACHMENT"};
        case TaskImageUsage::DEPTH_ATTACHMENT_READ_ONLY: return std::string_view{"DEPTH_ATTACHMENT_READ_ONLY"};
        case TaskImageUsage::STENCIL_ATTACHMENT_READ_ONLY: return std::string_view{"STENCIL_ATTACHMENT_READ_ONLY"};
        case TaskImageUsage::DEPTH_STENCIL_ATTACHMENT_READ_ONLY: return std::string_view{"DEPTH_STENCIL_ATTACHMENT_READ_ONLY"};
        case TaskImageUsage::RESOLVE_WRITE: return std::string_view{"RESOLVE_WRITE"};
        case TaskImageUsage::PRESENT: return std::string_view{"PRESENT"};
        default: DAXA_DBG_ASSERT_TRUE_M(false, "unreachable");
        }
    }

    TaskList::TaskList() : Handle{nullptr} {}

    TaskList::~TaskList() {}

    TaskList::TaskList(std::shared_ptr<void> a_impl) : Handle{std::move(a_impl)} {}

    auto TaskList::create_task_buffer(TaskBufferInfo const & callback) -> TaskBufferId
    {
        auto & impl = *reinterpret_cast<ImplTaskList *>(this->impl.get());
        DAXA_DBG_ASSERT_TRUE_M(!impl.compiled, "can only record to uncompleted task list");
        impl.impl_task_buffers.push_back(ImplTaskBuffer{.last_access = AccessConsts::NONE, .last_access_task_index = 0});
        return TaskBufferId{{.index = ++impl.next_task_buffer_id}};
    }

    auto TaskList::create_task_image(TaskImageInfo const & callback) -> TaskImageId
    {
        auto & impl = *reinterpret_cast<ImplTaskList *>(this->impl.get());
        DAXA_DBG_ASSERT_TRUE_M(!impl.compiled, "can only record to uncompleted task list");
        impl.impl_task_images.push_back(ImplTaskImage{.last_access = AccessConsts::NONE, .last_access_task_index = 0});
        return TaskImageId{{.index = ++impl.next_task_image_id}};
    }

    void TaskList::add_task(TaskInfo const & info)
    {
        auto & impl = *reinterpret_cast<ImplTaskList *>(this->impl.get());
        DAXA_DBG_ASSERT_TRUE_M(!impl.compiled, "can only record to uncompleted task list");
        impl.tasks.push_back(ImplTask{.info = info});
    }

    void TaskList::add_render_task(TaskRenderInfo const & info)
    {
        auto & impl = *reinterpret_cast<ImplTaskList *>(this->impl.get());
        DAXA_DBG_ASSERT_TRUE_M(!impl.compiled, "can only record to uncompleted task list");

        auto convert_attachment = [&](TaskRenderAttachmentInfo const & attach, auto & images, auto usage) -> RenderAttachmentInfo
        {
            ImageLayout layout =
                attach.layout_override != ImageLayout::UNDEFINED ? attach.layout_override : ImageLayout::COLOR_ATTACHMENT_OPTIMAL;
            DAXA_DBG_ASSERT_TRUE_M(!attach.image.is_empty(), "must declare image for render pass attachment");
            images.push_back({attach.image, ImageMipArraySlice::whole(), usage});

            return RenderAttachmentInfo{
                .image = impl.get_image(attach.image),
                .image_view = impl.get_image_view(attach.image),
                .layout = layout,
                .load_op = attach.load_op,
                .store_op = attach.store_op,
                .clear_value = attach.clear_value,
            };
        };

        TaskInfo task = {};
        std::vector<RenderAttachmentInfo> color_attachments = {};
        task.resources = info.resources;
        for (auto const & attach : info.render_info.color_attachments)
        {
            color_attachments.push_back(convert_attachment(attach, task.resources.images, TaskImageUsage::COLOR_ATTACHMENT));
        }
        std::optional<RenderAttachmentInfo> depth_attach = {};
        if (info.render_info.depth_attachment.has_value())
        {
            depth_attach = convert_attachment(
                info.render_info.depth_attachment.value(),
                task.resources.images,
                TaskImageUsage::DEPTH_ATTACHMENT);
        }
        std::optional<RenderAttachmentInfo> stencil_attach = {};
        if (info.render_info.stencil_attachment.has_value())
        {
            stencil_attach = convert_attachment(
                info.render_info.stencil_attachment.value(),
                task.resources.images,
                TaskImageUsage::STENCIL_ATTACHMENT);
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

    void TaskList::add_present(Swapchain swapchain)
    {
        auto & impl = *reinterpret_cast<ImplTaskList *>(this->impl.get());
        DAXA_DBG_ASSERT_TRUE_M(!impl.compiled, "can only record to uncompleted task list");

        add_task({
            .task = [swapchain](TaskInterface & interf) mutable
            {
                BinarySemaphore binary_semaphore = interf.device.create_binary_semaphore({
                    .debug_name = "TaskList Implicit Present Semaphore",
                });

                interf.device.submit_commands({
                    .command_lists = {std::move(interf.cmd_list)},
                    .signal_binary_semaphores = {binary_semaphore},
                    // .signal_timeline_semaphores = {{gpu_framecount_timeline_sema, cpu_framecount}},
                });

                interf.device.present_frame({
                    .wait_binary_semaphores = {binary_semaphore},
                    .swapchain = swapchain,
                });
            },
        });
    }

    void TaskList::compile()
    {
        auto & impl = *reinterpret_cast<ImplTaskList *>(this->impl.get());
        DAXA_DBG_ASSERT_TRUE_M(!impl.compiled, "Can only compile a task list one time");
        impl.compiled = true;
        impl.analyze_dependencies();
    }

    void TaskList::execute()
    {
        auto & impl = *reinterpret_cast<ImplTaskList *>(this->impl.get());
        DAXA_DBG_ASSERT_TRUE_M(impl.compiled, "Can only execute a completed task list");
    }

    ImplTaskList::ImplTaskList(std::shared_ptr<ImplDevice> a_impl_device, TaskListInfo const & info)
        : impl_device{std::move(a_impl_device)}, info{info}
    {
    }

    ImplTaskList::~ImplTaskList()
    {
    }

    auto access_to_image_layout(Access const & access) -> ImageLayout
    {
        return {};
    }

    auto ImplTaskList::compute_needed_barrier(Access const & previous_access, Access const & new_access)
        -> std::optional<TaskPipelineBarrier>
    {
        if (previous_access.type == AccessTypeFlagBits::READ && new_access.type == AccessTypeFlagBits::READ)
        {
            return {};
        }

        return {};
    }

    auto ImplTaskList::slot(TaskBufferId id) -> ImplTaskBuffer &
    {
        DAXA_DBG_ASSERT_TRUE_M(id.index != 0 && id.index <= impl_task_buffers.size(), "incalid task buffer id");
        return impl_task_buffers[id.index - 1];
    }

    auto ImplTaskList::slot(TaskImageId id) -> ImplTaskImage &
    {
        DAXA_DBG_ASSERT_TRUE_M(id.index != 0 && id.index <= impl_task_images.size(), "incalid task image id");
        return impl_task_images[id.index - 1];
    }

    auto ImplTaskList::get_buffer(TaskBufferId) -> BufferId
    {
        return {};
    }

    auto ImplTaskList::get_image(TaskImageId) -> ImageId
    {
        return {};
    }

    auto ImplTaskList::get_image_view(TaskImageId) -> ImageViewId
    {
        return {};
    }

    void ImplTaskList::analyze_dependencies()
    {
        usize task_index = 0;
        for (auto & task : tasks)
        {
            std::visit(
                [&](auto & obj)
                {
                    if constexpr (std::is_base_of_v<TaskInfo, std::decay_t<decltype(obj)>>)
                    {
                        TaskResources & resources = obj.resources;

                        for (auto & [tid, usage] : resources.buffers)
                        {
                            std::cout
                                << "buffer tid: "
                                << tid.index
                                << ", previous access: "
                                << to_string(slot(tid).last_access)
                                << ", access: "
                                << to_string(usage)
                                << std::endl;

                            // slot(tid).last_access = usage;
                            slot(tid).last_access_task_index = task_index;
                        }

                        for (auto & [tid, slice, usage] : resources.images)
                        {
                            std::cout
                                << "image tid: "
                                << tid.index
                                << ", previous access: "
                                << to_string(slot(tid).last_access)
                                << ", access: " << to_string(usage)
                                << std::endl;

                            // slot(tid).last_access = access;
                            slot(tid).last_access_task_index = task_index;
                        }
                    }
                },
                task.info);
            ++task_index;
        }
    }

} // namespace daxa