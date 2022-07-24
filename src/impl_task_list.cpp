#include "impl_task_list.hpp"
#include <iostream>

namespace daxa
{
    auto TaskGPUResourceId::is_empty() const -> bool
    {
        return index == std::numeric_limits<u32>::max();
    }

    auto to_string(TaskBufferAccess const & usage) -> std::string_view
    {
        switch (usage)
        {
        case TaskBufferAccess::SHADER_READ_ONLY: return std::string_view{"SHADER_READ_ONLY"};
        case TaskBufferAccess::VERTEX_SHADER_READ_ONLY: return std::string_view{"VERTEX_SHADER_READ_ONLY"};
        case TaskBufferAccess::TESSELLATION_CONTROL_SHADER_READ_ONLY: return std::string_view{"TESSELLATION_CONTROL_SHADER_READ_ONLY"};
        case TaskBufferAccess::TESSELLATION_EVALUATION_SHADER_READ_ONLY: return std::string_view{"TESSELLATION_EVALUATION_SHADER_READ_ONLY"};
        case TaskBufferAccess::GEOMETRY_SHADER_READ_ONLY: return std::string_view{"GEOMETRY_SHADER_READ_ONLY"};
        case TaskBufferAccess::FRAGMENT_SHADER_READ_ONLY: return std::string_view{"FRAGMENT_SHADER_READ_ONLY"};
        case TaskBufferAccess::COMPUTE_SHADER_READ_ONLY: return std::string_view{"COMPUTE_SHADER_READ_ONLY"};
        case TaskBufferAccess::SHADER_WRITE_ONLY: return std::string_view{"SHADER_WRITE_ONLY"};
        case TaskBufferAccess::VERTEX_SHADER_WRITE_ONLY: return std::string_view{"VERTEX_SHADER_WRITE_ONLY"};
        case TaskBufferAccess::TESSELLATION_CONTROL_SHADER_WRITE_ONLY: return std::string_view{"TESSELLATION_CONTROL_SHADER_WRITE_ONLY"};
        case TaskBufferAccess::TESSELLATION_EVALUATION_SHADER_WRITE_ONLY: return std::string_view{"TESSELLATION_EVALUATION_SHADER_WRITE_ONLY"};
        case TaskBufferAccess::GEOMETRY_SHADER_WRITE_ONLY: return std::string_view{"GEOMETRY_SHADER_WRITE_ONLY"};
        case TaskBufferAccess::FRAGMENT_SHADER_WRITE_ONLY: return std::string_view{"FRAGMENT_SHADER_WRITE_ONLY"};
        case TaskBufferAccess::COMPUTE_SHADER_WRITE_ONLY: return std::string_view{"COMPUTE_SHADER_WRITE_ONLY"};
        case TaskBufferAccess::SHADER_READ_WRITE: return std::string_view{"SHADER_READ_WRITE"};
        case TaskBufferAccess::VERTEX_SHADER_READ_WRITE: return std::string_view{"VERTEX_SHADER_READ_WRITE"};
        case TaskBufferAccess::TESSELLATION_CONTROL_SHADER_READ_WRITE: return std::string_view{"TESSELLATION_CONTROL_SHADER_READ_WRITE"};
        case TaskBufferAccess::TESSELLATION_EVALUATION_SHADER_READ_WRITE: return std::string_view{"TESSELLATION_EVALUATION_SHADER_READ_WRITE"};
        case TaskBufferAccess::GEOMETRY_SHADER_READ_WRITE: return std::string_view{"GEOMETRY_SHADER_READ_WRITE"};
        case TaskBufferAccess::FRAGMENT_SHADER_READ_WRITE: return std::string_view{"FRAGMENT_SHADER_READ_WRITE"};
        case TaskBufferAccess::COMPUTE_SHADER_READ_WRITE: return std::string_view{"COMPUTE_SHADER_READ_WRITE"};
        case TaskBufferAccess::TRANSFER_READ: return std::string_view{"TRANSFER_READ"};
        case TaskBufferAccess::TRANSFER_WRITE: return std::string_view{"TRANSFER_WRITE"};
        default: DAXA_DBG_ASSERT_TRUE_M(false, "unreachable");
        }
        return "invalid";
    }

    auto to_string(TaskImageAccess const & usage) -> std::string_view
    {
        switch (usage)
        {
        case TaskImageAccess::SHADER_READ_ONLY: return std::string_view{"SHADER_READ_ONLY"};
        case TaskImageAccess::VERTEX_SHADER_READ_ONLY: return std::string_view{"VERTEX_SHADER_READ_ONLY"};
        case TaskImageAccess::TESSELLATION_CONTROL_SHADER_READ_ONLY: return std::string_view{"TESSELLATION_CONTROL_SHADER_READ_ONLY"};
        case TaskImageAccess::TESSELLATION_EVALUATION_SHADER_READ_ONLY: return std::string_view{"TESSELLATION_EVALUATION_SHADER_READ_ONLY"};
        case TaskImageAccess::GEOMETRY_SHADER_READ_ONLY: return std::string_view{"GEOMETRY_SHADER_READ_ONLY"};
        case TaskImageAccess::FRAGMENT_SHADER_READ_ONLY: return std::string_view{"FRAGMENT_SHADER_READ_ONLY"};
        case TaskImageAccess::COMPUTE_SHADER_READ_ONLY: return std::string_view{"COMPUTE_SHADER_READ_ONLY"};
        case TaskImageAccess::SHADER_WRITE_ONLY: return std::string_view{"SHADER_WRITE_ONLY"};
        case TaskImageAccess::VERTEX_SHADER_WRITE_ONLY: return std::string_view{"VERTEX_SHADER_WRITE_ONLY"};
        case TaskImageAccess::TESSELLATION_CONTROL_SHADER_WRITE_ONLY: return std::string_view{"TESSELLATION_CONTROL_SHADER_WRITE_ONLY"};
        case TaskImageAccess::TESSELLATION_EVALUATION_SHADER_WRITE_ONLY: return std::string_view{"TESSELLATION_EVALUATION_SHADER_WRITE_ONLY"};
        case TaskImageAccess::GEOMETRY_SHADER_WRITE_ONLY: return std::string_view{"GEOMETRY_SHADER_WRITE_ONLY"};
        case TaskImageAccess::FRAGMENT_SHADER_WRITE_ONLY: return std::string_view{"FRAGMENT_SHADER_WRITE_ONLY"};
        case TaskImageAccess::COMPUTE_SHADER_WRITE_ONLY: return std::string_view{"COMPUTE_SHADER_WRITE_ONLY"};
        case TaskImageAccess::SHADER_READ_WRITE: return std::string_view{"SHADER_READ_WRITE"};
        case TaskImageAccess::VERTEX_SHADER_READ_WRITE: return std::string_view{"VERTEX_SHADER_READ_WRITE"};
        case TaskImageAccess::TESSELLATION_CONTROL_SHADER_READ_WRITE: return std::string_view{"TESSELLATION_CONTROL_SHADER_READ_WRITE"};
        case TaskImageAccess::TESSELLATION_EVALUATION_SHADER_READ_WRITE: return std::string_view{"TESSELLATION_EVALUATION_SHADER_READ_WRITE"};
        case TaskImageAccess::GEOMETRY_SHADER_READ_WRITE: return std::string_view{"GEOMETRY_SHADER_READ_WRITE"};
        case TaskImageAccess::FRAGMENT_SHADER_READ_WRITE: return std::string_view{"FRAGMENT_SHADER_READ_WRITE"};
        case TaskImageAccess::COMPUTE_SHADER_READ_WRITE: return std::string_view{"COMPUTE_SHADER_READ_WRITE"};
        case TaskImageAccess::TRANSFER_READ: return std::string_view{"TRANSFER_READ"};
        case TaskImageAccess::TRANSFER_WRITE: return std::string_view{"TRANSFER_WRITE"};
        case TaskImageAccess::COLOR_ATTACHMENT: return std::string_view{"COLOR_ATTACHMENT"};
        case TaskImageAccess::DEPTH_ATTACHMENT: return std::string_view{"DEPTH_ATTACHMENT"};
        case TaskImageAccess::STENCIL_ATTACHMENT: return std::string_view{"STENCIL_ATTACHMENT"};
        case TaskImageAccess::DEPTH_STENCIL_ATTACHMENT: return std::string_view{"DEPTH_STENCIL_ATTACHMENT"};
        case TaskImageAccess::DEPTH_ATTACHMENT_READ_ONLY: return std::string_view{"DEPTH_ATTACHMENT_READ_ONLY"};
        case TaskImageAccess::STENCIL_ATTACHMENT_READ_ONLY: return std::string_view{"STENCIL_ATTACHMENT_READ_ONLY"};
        case TaskImageAccess::DEPTH_STENCIL_ATTACHMENT_READ_ONLY: return std::string_view{"DEPTH_STENCIL_ATTACHMENT_READ_ONLY"};
        case TaskImageAccess::RESOLVE_WRITE: return std::string_view{"RESOLVE_WRITE"};
        case TaskImageAccess::PRESENT: return std::string_view{"PRESENT"};
        default: DAXA_DBG_ASSERT_TRUE_M(false, "unreachable");
        }
        return "invalid";
    }

    TaskInterface::TaskInterface(void * backend) : backend{backend} {}

    auto TaskInterface::get_device() -> Device &
    {
        ImplTaskList impl = *reinterpret_cast<ImplTaskList *>(backend);
        return *impl.current_device;
    }

    auto TaskInterface::get_command_list() -> CommandList &
    {
        ImplTaskList impl = *reinterpret_cast<ImplTaskList *>(backend);
        return *impl.current_command_list;
    }

    auto TaskInterface::get_resources() -> TaskResources &
    {
        ImplTaskList impl = *reinterpret_cast<ImplTaskList *>(backend);
        return *impl.current_resources;
    }

    auto TaskInterface::get_buffer(TaskBufferId const & task_id) -> BufferId
    {
        ImplTaskList impl = *reinterpret_cast<ImplTaskList *>(backend);
        return impl.impl_task_buffers[task_id.index].runtime_id;
    }

    auto TaskInterface::get_image(TaskImageId const & task_id) -> ImageId
    {
        ImplTaskList impl = *reinterpret_cast<ImplTaskList *>(backend);
        return impl.impl_task_images[task_id.index].runtime_id;
    }

    auto TaskInterface::get_image_view(TaskImageId const & task_id) -> ImageViewId
    {
        ImplTaskList impl = *reinterpret_cast<ImplTaskList *>(backend);
        return impl.impl_task_images[task_id.index].runtime_view_id;
    }

    TaskList::TaskList(TaskListInfo const & info)
        : Handle{std::make_shared<ImplTaskList>(info)}
    {
    }

    TaskList::~TaskList() {}

    TaskList::TaskList(std::shared_ptr<void> a_impl) : Handle{std::move(a_impl)} {}

    auto TaskList::create_task_buffer(TaskBufferInfo const & info) -> TaskBufferId
    {
        auto & impl = *reinterpret_cast<ImplTaskList *>(this->impl.get());
        DAXA_DBG_ASSERT_TRUE_M(!impl.compiled, "can only record to uncompleted task list");

        auto access = impl.task_buffer_access_to_access(info.prev_access);

        impl.impl_task_buffers.push_back(ImplTaskBuffer{
            .last_access = access,
            .last_access_task_index = 0,
            .fetch_callback = info.fetch_callback,
        });

        u32 tid = ++impl.next_task_buffer_id;

        impl.tasks.back().create_task_buffer_ids.push_back(tid);

        return TaskBufferId{{.index = tid}};
    }

    auto TaskList::create_task_image(TaskImageInfo const & info) -> TaskImageId
    {
        auto & impl = *reinterpret_cast<ImplTaskList *>(this->impl.get());
        DAXA_DBG_ASSERT_TRUE_M(!impl.compiled, "can only record to uncompleted task list");
        impl.impl_task_images.push_back(ImplTaskImage{.last_access = AccessConsts::NONE, .last_access_task_index = 0});

        auto [layout, access] = impl.task_image_access_to_layout_access(info.prev_access);

        impl.impl_task_images.push_back(ImplTaskImage{
            .last_access = access,
            .last_layout = layout,
            .last_access_task_index = 0,
            .fetch_callback = info.fetch_callback,
        });

        u32 tid = ++impl.next_task_image_id;

        impl.tasks.back().create_task_image_ids.push_back(tid);

        return TaskImageId{{.index = tid}};
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
            images.push_back({attach.image, {}, usage});

            return RenderAttachmentInfo{
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
            color_attachments.push_back(convert_attachment(attach, task.resources.images, TaskImageAccess::COLOR_ATTACHMENT));
        }
        std::optional<RenderAttachmentInfo> depth_attach = {};
        if (info.render_info.depth_attachment.has_value())
        {
            depth_attach = convert_attachment(
                info.render_info.depth_attachment.value(),
                task.resources.images,
                TaskImageAccess::DEPTH_ATTACHMENT);
        }
        std::optional<RenderAttachmentInfo> stencil_attach = {};
        if (info.render_info.stencil_attachment.has_value())
        {
            stencil_attach = convert_attachment(
                info.render_info.stencil_attachment.value(),
                task.resources.images,
                TaskImageAccess::STENCIL_ATTACHMENT);
        }
        task.task = [=](TaskInterface & interf)
        {
            auto & cmd = interf.get_command_list();
            cmd.begin_renderpass({
                .color_attachments = color_attachments,
                .depth_attachment = depth_attach,
                .stencil_attachment = stencil_attach,
                .render_area = info.render_info.render_area,
            });
            info.task(interf);
            cmd.end_renderpass();
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
                BinarySemaphore binary_semaphore = interf.get_device().create_binary_semaphore({
                    .debug_name = "TaskList Implicit Present Semaphore",
                });

                interf.get_device().submit_commands({
                    .command_lists = {std::move(interf.get_command_list())},
                    .signal_binary_semaphores = {binary_semaphore},
                    // .signal_timeline_semaphores = {{gpu_framecount_timeline_sema, cpu_framecount}},
                });

                interf.get_device().present_frame({
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
        impl.insert_synchronization();
    }

    void TaskList::execute()
    {
        auto & impl = *reinterpret_cast<ImplTaskList *>(this->impl.get());
        DAXA_DBG_ASSERT_TRUE_M(impl.compiled, "Can only execute a completed task list");

        CommandList cmd_list = impl.info.device.create_command_list({
            .debug_name = impl.info.debug_name += " command list",
        });

        impl.current_device = &impl.info.device;
        impl.current_command_list = &cmd_list;
        impl.current_device = &impl.info.device;

        TaskInterface task_interface{this};

        usize task_index = 0;
        for (ImplTask & task : impl.tasks)
        {
            impl.current_resources = &task.info.resources;
            for (u32 t_buffer_id : task.create_task_buffer_ids)
            {
                impl.impl_task_buffers[t_buffer_id].runtime_id = impl.impl_task_buffers[t_buffer_id].fetch_callback(task_interface);
            }
            for (u32 t_image_id : task.create_task_buffer_ids)
            {
                impl.impl_task_images[t_image_id].runtime_id = impl.impl_task_images[t_image_id].fetch_callback(task_interface);
            }
            
            ++task_index;
        }
    }

    ImplTaskList::ImplTaskList(TaskListInfo const & info)
        : info{info}
    {
        tasks.push_back({});
    }

    ImplTaskList::~ImplTaskList()
    {
    }

    auto ImplTaskList::task_image_access_to_layout_access(TaskImageAccess const & access) -> std::tuple<ImageLayout, Access>
    {
        switch (access)
        {
        case TaskImageAccess::NONE: return {ImageLayout::UNDEFINED, {PipelineStageFlagBits::NONE, AccessTypeFlagBits::NONE}};
        case TaskImageAccess::SHADER_READ_ONLY: return {ImageLayout::SHADER_READ_ONLY_OPTIMAL, {PipelineStageFlagBits::ALL_GRAPHICS | PipelineStageFlagBits::COMPUTE_SHADER, AccessTypeFlagBits::READ}};
        case TaskImageAccess::VERTEX_SHADER_READ_ONLY: return {ImageLayout::SHADER_READ_ONLY_OPTIMAL, {PipelineStageFlagBits::VERTEX_SHADER, AccessTypeFlagBits::READ}};
        case TaskImageAccess::TESSELLATION_CONTROL_SHADER_READ_ONLY: return {ImageLayout::SHADER_READ_ONLY_OPTIMAL, {PipelineStageFlagBits::TESSELLATION_CONTROL_SHADER, AccessTypeFlagBits::READ}};
        case TaskImageAccess::TESSELLATION_EVALUATION_SHADER_READ_ONLY: return {ImageLayout::SHADER_READ_ONLY_OPTIMAL, {PipelineStageFlagBits::TESSELLATION_EVALUATION_SHADER, AccessTypeFlagBits::READ}};
        case TaskImageAccess::GEOMETRY_SHADER_READ_ONLY: return {ImageLayout::SHADER_READ_ONLY_OPTIMAL, {PipelineStageFlagBits::GEOMETRY_SHADER, AccessTypeFlagBits::READ}};
        case TaskImageAccess::FRAGMENT_SHADER_READ_ONLY: return {ImageLayout::SHADER_READ_ONLY_OPTIMAL, {PipelineStageFlagBits::FRAGMENT_SHADER, AccessTypeFlagBits::READ}};
        case TaskImageAccess::COMPUTE_SHADER_READ_ONLY: return {ImageLayout::SHADER_READ_ONLY_OPTIMAL, {PipelineStageFlagBits::COMPUTE_SHADER, AccessTypeFlagBits::READ}};
        case TaskImageAccess::SHADER_WRITE_ONLY: return {ImageLayout::GENERAL, {PipelineStageFlagBits::PipelineStageFlagBits::ALL_GRAPHICS | PipelineStageFlagBits::COMPUTE_SHADER, AccessTypeFlagBits::WRITE}};
        case TaskImageAccess::VERTEX_SHADER_WRITE_ONLY: return {ImageLayout::GENERAL, {PipelineStageFlagBits::VERTEX_SHADER, AccessTypeFlagBits::WRITE}};
        case TaskImageAccess::TESSELLATION_CONTROL_SHADER_WRITE_ONLY: return {ImageLayout::GENERAL, {PipelineStageFlagBits::TESSELLATION_CONTROL_SHADER, AccessTypeFlagBits::WRITE}};
        case TaskImageAccess::TESSELLATION_EVALUATION_SHADER_WRITE_ONLY: return {ImageLayout::GENERAL, {PipelineStageFlagBits::TESSELLATION_EVALUATION_SHADER, AccessTypeFlagBits::WRITE}};
        case TaskImageAccess::GEOMETRY_SHADER_WRITE_ONLY: return {ImageLayout::GENERAL, {PipelineStageFlagBits::GEOMETRY_SHADER, AccessTypeFlagBits::WRITE}};
        case TaskImageAccess::FRAGMENT_SHADER_WRITE_ONLY: return {ImageLayout::GENERAL, {PipelineStageFlagBits::FRAGMENT_SHADER, AccessTypeFlagBits::WRITE}};
        case TaskImageAccess::COMPUTE_SHADER_WRITE_ONLY: return {ImageLayout::GENERAL, {PipelineStageFlagBits::COMPUTE_SHADER, AccessTypeFlagBits::WRITE}};
        case TaskImageAccess::SHADER_READ_WRITE: return {ImageLayout::GENERAL, {PipelineStageFlagBits::PipelineStageFlagBits::ALL_GRAPHICS | PipelineStageFlagBits::COMPUTE_SHADER, AccessTypeFlagBits::READ_WRITE}};
        case TaskImageAccess::VERTEX_SHADER_READ_WRITE: return {ImageLayout::GENERAL, {PipelineStageFlagBits::VERTEX_SHADER, AccessTypeFlagBits::READ_WRITE}};
        case TaskImageAccess::TESSELLATION_CONTROL_SHADER_READ_WRITE: return {ImageLayout::GENERAL, {PipelineStageFlagBits::TESSELLATION_CONTROL_SHADER, AccessTypeFlagBits::READ_WRITE}};
        case TaskImageAccess::TESSELLATION_EVALUATION_SHADER_READ_WRITE: return {ImageLayout::GENERAL, {PipelineStageFlagBits::TESSELLATION_EVALUATION_SHADER, AccessTypeFlagBits::READ_WRITE}};
        case TaskImageAccess::GEOMETRY_SHADER_READ_WRITE: return {ImageLayout::GENERAL, {PipelineStageFlagBits::GEOMETRY_SHADER, AccessTypeFlagBits::READ_WRITE}};
        case TaskImageAccess::FRAGMENT_SHADER_READ_WRITE: return {ImageLayout::GENERAL, {PipelineStageFlagBits::FRAGMENT_SHADER, AccessTypeFlagBits::READ_WRITE}};
        case TaskImageAccess::COMPUTE_SHADER_READ_WRITE: return {ImageLayout::GENERAL, {PipelineStageFlagBits::COMPUTE_SHADER, AccessTypeFlagBits::READ_WRITE}};
        case TaskImageAccess::TRANSFER_READ: return {ImageLayout::TRANSFER_SRC_OPTIMAL, {PipelineStageFlagBits::TRANSFER, AccessTypeFlagBits::READ}};
        case TaskImageAccess::TRANSFER_WRITE: return {ImageLayout::TRANSFER_DST_OPTIMAL, {PipelineStageFlagBits::TRANSFER, AccessTypeFlagBits::WRITE}};
        case TaskImageAccess::COLOR_ATTACHMENT: return {ImageLayout::COLOR_ATTACHMENT_OPTIMAL, {PipelineStageFlagBits::COLOR_ATTACHMENT_OUTPUT, AccessTypeFlagBits::READ_WRITE}};
        case TaskImageAccess::DEPTH_ATTACHMENT: return {ImageLayout::DEPTH_ATTACHMENT_OPTIMAL, {PipelineStageFlagBits::EARLY_FRAGMENT_TESTS | PipelineStageFlagBits::LATE_FRAGMENT_TESTS, AccessTypeFlagBits::READ_WRITE}};
        case TaskImageAccess::STENCIL_ATTACHMENT: return {ImageLayout::STENCIL_ATTACHMENT_OPTIMAL, {PipelineStageFlagBits::EARLY_FRAGMENT_TESTS | PipelineStageFlagBits::LATE_FRAGMENT_TESTS, AccessTypeFlagBits::READ_WRITE}};
        case TaskImageAccess::DEPTH_STENCIL_ATTACHMENT: return {ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL, {PipelineStageFlagBits::EARLY_FRAGMENT_TESTS | PipelineStageFlagBits::LATE_FRAGMENT_TESTS, AccessTypeFlagBits::READ_WRITE}};
        case TaskImageAccess::DEPTH_ATTACHMENT_READ_ONLY: return {ImageLayout::DEPTH_READ_ONLY_OPTIMAL, {PipelineStageFlagBits::EARLY_FRAGMENT_TESTS | PipelineStageFlagBits::LATE_FRAGMENT_TESTS, AccessTypeFlagBits::READ}};
        case TaskImageAccess::STENCIL_ATTACHMENT_READ_ONLY: return {ImageLayout::STENCIL_READ_ONLY_OPTIMAL, {PipelineStageFlagBits::EARLY_FRAGMENT_TESTS | PipelineStageFlagBits::LATE_FRAGMENT_TESTS, AccessTypeFlagBits::READ}};
        case TaskImageAccess::DEPTH_STENCIL_ATTACHMENT_READ_ONLY: return {ImageLayout::DEPTH_STENCIL_READ_ONLY_OPTIMAL, {PipelineStageFlagBits::EARLY_FRAGMENT_TESTS | PipelineStageFlagBits::LATE_FRAGMENT_TESTS, AccessTypeFlagBits::READ}};
        case TaskImageAccess::RESOLVE_WRITE: return {ImageLayout::ATTACHMENT_OPTIMAL, {PipelineStageFlagBits::RESOLVE, AccessTypeFlagBits::WRITE}};
        case TaskImageAccess::PRESENT: return {ImageLayout::PRESENT_SRC, {PipelineStageFlagBits::ALL_COMMANDS, AccessTypeFlagBits::READ}};
        default: DAXA_DBG_ASSERT_TRUE_M(false, "unreachable");
        }
        return {};
    }

    auto ImplTaskList::task_buffer_access_to_access(TaskBufferAccess const & access) -> Access
    {
        switch (access)
        {
        case TaskBufferAccess::NONE: return {PipelineStageFlagBits::NONE, AccessTypeFlagBits::NONE};
        case TaskBufferAccess::SHADER_READ_ONLY: return {PipelineStageFlagBits::ALL_GRAPHICS | PipelineStageFlagBits::COMPUTE_SHADER, AccessTypeFlagBits::READ};
        case TaskBufferAccess::VERTEX_SHADER_READ_ONLY: return {PipelineStageFlagBits::VERTEX_SHADER, AccessTypeFlagBits::READ};
        case TaskBufferAccess::TESSELLATION_CONTROL_SHADER_READ_ONLY: return {PipelineStageFlagBits::TESSELLATION_CONTROL_SHADER, AccessTypeFlagBits::READ};
        case TaskBufferAccess::TESSELLATION_EVALUATION_SHADER_READ_ONLY: return {PipelineStageFlagBits::TESSELLATION_EVALUATION_SHADER, AccessTypeFlagBits::READ};
        case TaskBufferAccess::GEOMETRY_SHADER_READ_ONLY: return {PipelineStageFlagBits::GEOMETRY_SHADER, AccessTypeFlagBits::READ};
        case TaskBufferAccess::FRAGMENT_SHADER_READ_ONLY: return {PipelineStageFlagBits::FRAGMENT_SHADER, AccessTypeFlagBits::READ};
        case TaskBufferAccess::COMPUTE_SHADER_READ_ONLY: return {PipelineStageFlagBits::COMPUTE_SHADER, AccessTypeFlagBits::READ};
        case TaskBufferAccess::SHADER_WRITE_ONLY: return {PipelineStageFlagBits::PipelineStageFlagBits::ALL_GRAPHICS | PipelineStageFlagBits::COMPUTE_SHADER, AccessTypeFlagBits::WRITE};
        case TaskBufferAccess::VERTEX_SHADER_WRITE_ONLY: return {PipelineStageFlagBits::VERTEX_SHADER, AccessTypeFlagBits::WRITE};
        case TaskBufferAccess::TESSELLATION_CONTROL_SHADER_WRITE_ONLY: return {PipelineStageFlagBits::TESSELLATION_CONTROL_SHADER, AccessTypeFlagBits::WRITE};
        case TaskBufferAccess::TESSELLATION_EVALUATION_SHADER_WRITE_ONLY: return {PipelineStageFlagBits::TESSELLATION_EVALUATION_SHADER, AccessTypeFlagBits::WRITE};
        case TaskBufferAccess::GEOMETRY_SHADER_WRITE_ONLY: return {PipelineStageFlagBits::GEOMETRY_SHADER, AccessTypeFlagBits::WRITE};
        case TaskBufferAccess::FRAGMENT_SHADER_WRITE_ONLY: return {PipelineStageFlagBits::FRAGMENT_SHADER, AccessTypeFlagBits::WRITE};
        case TaskBufferAccess::COMPUTE_SHADER_WRITE_ONLY: return {PipelineStageFlagBits::COMPUTE_SHADER, AccessTypeFlagBits::WRITE};
        case TaskBufferAccess::SHADER_READ_WRITE: return {PipelineStageFlagBits::PipelineStageFlagBits::ALL_GRAPHICS | PipelineStageFlagBits::COMPUTE_SHADER, AccessTypeFlagBits::READ_WRITE};
        case TaskBufferAccess::VERTEX_SHADER_READ_WRITE: return {PipelineStageFlagBits::VERTEX_SHADER, AccessTypeFlagBits::READ_WRITE};
        case TaskBufferAccess::TESSELLATION_CONTROL_SHADER_READ_WRITE: return {PipelineStageFlagBits::TESSELLATION_CONTROL_SHADER, AccessTypeFlagBits::READ_WRITE};
        case TaskBufferAccess::TESSELLATION_EVALUATION_SHADER_READ_WRITE: return {PipelineStageFlagBits::TESSELLATION_EVALUATION_SHADER, AccessTypeFlagBits::READ_WRITE};
        case TaskBufferAccess::GEOMETRY_SHADER_READ_WRITE: return {PipelineStageFlagBits::GEOMETRY_SHADER, AccessTypeFlagBits::READ_WRITE};
        case TaskBufferAccess::FRAGMENT_SHADER_READ_WRITE: return {PipelineStageFlagBits::FRAGMENT_SHADER, AccessTypeFlagBits::READ_WRITE};
        case TaskBufferAccess::COMPUTE_SHADER_READ_WRITE: return {PipelineStageFlagBits::COMPUTE_SHADER, AccessTypeFlagBits::READ_WRITE};
        case TaskBufferAccess::TRANSFER_READ: return {PipelineStageFlagBits::TRANSFER, AccessTypeFlagBits::READ};
        case TaskBufferAccess::TRANSFER_WRITE: return {PipelineStageFlagBits::TRANSFER, AccessTypeFlagBits::WRITE};
        default: DAXA_DBG_ASSERT_TRUE_M(false, "unreachable");
        }
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

    void ImplTaskList::insert_synchronization()
    {
        usize task_index = 0;
        for (ImplTask & task : tasks)
        {
            std::cout
                << "process task index : "
                << task_index
                << "\n{"
                << std::endl;

            TaskResources & resources = task.info.resources;

            for (auto & [tid, t_access] : resources.buffers)
            {

                // slot(tid).last_access = usage;
                slot(tid).last_access_task_index = task_index;
                auto new_access = task_buffer_access_to_access(t_access);
                auto & latest_access = slot(tid).last_access;

                std::cout
                    << "  task access: buffer tid: "
                    << tid.index
                    << ",\n    previous access: "
                    << to_string(slot(tid).last_access)
                    << ",\n    new access: "
                    << to_string(new_access)
                    << std::endl;

                bool need_memory_barrier = false;
                bool need_execution_barrier = false;

                if (latest_access.type & AccessTypeFlagBits::WRITE)
                {
                    need_memory_barrier = true;
                    need_execution_barrier = true;
                }
                else if ((latest_access.type & AccessTypeFlagBits::READ) != 0 && (new_access.type & AccessTypeFlagBits::WRITE) != 0)
                {
                    need_execution_barrier = true;
                }

                if (need_memory_barrier || need_execution_barrier)
                {
                    usize last_access_task_index = slot(tid).last_access_task_index;

                    usize barrier_task_index = {};

                    if (last_access_task_index >= this->last_task_index_with_barrier)
                    {
                        // reuse old barrier
                        barrier_task_index = last_task_index_with_barrier;
                    }
                    else
                    {
                        // insert new barrier
                        barrier_task_index = task_index;
                        last_task_index_with_barrier = task_index;
                    }
                    tasks[barrier_task_index].barriers.push_back(TaskPipelineBarrier{
                        .awaited_pipeline_access = Access{
                            .stages = latest_access.stages,
                            .type = need_memory_barrier ? latest_access.type : AccessTypeFlagBits::NONE,
                        },
                        .waiting_pipeline_access = Access{
                            .stages = new_access.stages,
                            .type = need_memory_barrier ? new_access.type : AccessTypeFlagBits::NONE,
                        },
                    });

                    latest_access = new_access;
                }
                else
                {
                    latest_access = latest_access | new_access;
                }
            }

            for (auto & [tid, slice, t_access] : resources.images)
            {
                slot(tid).last_access_task_index = task_index;
                auto [new_layout, new_access] = task_image_access_to_layout_access(t_access);
                auto & latest_layout = slot(tid).last_layout;
                auto & latest_access = slot(tid).last_access;

                std::cout
                    << "  task access: image tid: "
                    << tid.index
                    << ",\n    previous access: "
                    << to_string(slot(tid).last_access)
                    << ",\n    previous layout: "
                    << to_string(slot(tid).last_layout)
                    << ",\n    new access: "
                    << to_string(new_access)
                    << ",\n    new layout: "
                    << to_string(new_layout)
                    << std::endl;

                bool need_memory_barrier = false;
                bool need_execution_barrier = false;

                if (latest_access.type & AccessTypeFlagBits::WRITE)
                {
                    need_memory_barrier = true;
                    need_execution_barrier = true;
                }
                else if ((latest_access.type & AccessTypeFlagBits::READ) != 0 && (new_access.type & AccessTypeFlagBits::WRITE) != 0)
                {
                    need_execution_barrier = true;
                }

                bool need_layout_transition = new_layout != latest_layout;

                if (need_memory_barrier || need_execution_barrier || need_layout_transition)
                {
                    usize last_access_task_index = slot(tid).last_access_task_index;

                    usize barrier_task_index = {};

                    if (last_access_task_index >= this->last_task_index_with_barrier)
                    {
                        // reuse old barrier
                        barrier_task_index = last_task_index_with_barrier;
                    }
                    else
                    {
                        // insert new barrier
                        barrier_task_index = task_index;
                        last_task_index_with_barrier = task_index;
                    }
                    tasks[barrier_task_index].barriers.push_back(TaskPipelineBarrier{
                        .awaited_pipeline_access = Access{
                            .stages = latest_access.stages,
                            .type = need_memory_barrier ? latest_access.type : AccessTypeFlagBits::NONE,
                        },
                        .waiting_pipeline_access = Access{
                            .stages = new_access.stages,
                            .type = need_memory_barrier ? new_access.type : AccessTypeFlagBits::NONE,
                        },
                        .before_layout = latest_layout,
                        .after_layout = new_layout,
                        .image_id = tid,
                        .image_slice = slot(tid).slice,
                    });

                    std::cout
                        << "  inserted barrier at task index: "
                        << task_index
                        << ",\n    task image index: "
                        << tid.index
                        << ",\n    awaited_pipeline_access: "
                        << to_string(tasks[barrier_task_index].barriers.back().awaited_pipeline_access)
                        << ",\n    waiting_pipeline_access: "
                        << to_string(tasks[barrier_task_index].barriers.back().waiting_pipeline_access)
                        << ",\n    before_layout: "
                        << to_string(tasks[barrier_task_index].barriers.back().before_layout)
                        << ",\n    after_layout: "
                        << to_string(tasks[barrier_task_index].barriers.back().after_layout)
                        << std::endl;

                    latest_layout = new_layout;
                    latest_access = new_access;
                }
                else
                {
                    latest_access = latest_access | new_access;
                }
            }
            std::cout
                << "}\n"
                << std::endl;
            ++task_index;
        }
    }

} // namespace daxa