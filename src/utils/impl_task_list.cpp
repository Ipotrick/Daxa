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
        case TaskBufferAccess::HOST_TRANSFER_READ: return std::string_view{"HOST_TRANSFER_READ"};
        case TaskBufferAccess::HOST_TRANSFER_WRITE: return std::string_view{"HOST_TRANSFER_WRITE"};
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

    TaskInterface::TaskInterface(void * backend, TaskResources * resources)
        : backend{backend}, resources{resources}
    {
    }

    auto TaskInterface::get_device() -> Device &
    {
        auto & impl = *reinterpret_cast<TaskRuntime *>(backend);
        return impl.current_device;
    }

    auto TaskInterface::get_command_list() -> CommandList &
    {
        auto & impl = *reinterpret_cast<TaskRuntime *>(backend);
        return impl.current_command_list;
    }

    auto TaskInterface::get_resources() -> TaskResources &
    {
        return *resources;
    }

    auto TaskInterface::get_buffer(TaskBufferId const & task_id) -> BufferId
    {
        auto & impl = *reinterpret_cast<TaskRuntime *>(backend);
        return impl.runtime_buffers[task_id.index].buffer_id;
    }

    auto TaskInterface::get_image(TaskImageId const & task_id) -> ImageId
    {
        auto & impl = *reinterpret_cast<TaskRuntime *>(backend);
        return impl.runtime_images[task_id.index].image_id;
    }

    auto TaskInterface::get_image_view(TaskImageId const & task_id) -> ImageViewId
    {
        auto & impl = *reinterpret_cast<TaskRuntime *>(backend);
        return impl.runtime_images[task_id.index].image_view_id;
    }

    TaskList::TaskList(TaskListInfo const & info)
        : ManagedPtr{new ImplTaskList(info)}
    {
    }

    TaskList::~TaskList() {}

    auto TaskList::create_task_buffer(TaskBufferInfo const & info) -> TaskBufferId
    {
        auto & impl = *as<ImplTaskList>();
        DAXA_DBG_ASSERT_TRUE_M(!impl.compiled, "can only record to uncompleted task list");

        usize task_index = impl.tasks.size() + 1;

        impl.impl_task_buffers.push_back(ImplTaskBuffer{
            .latest_access = info.last_access,
            .latest_access_task_index = task_index,
            .fetch_callback = info.fetch_callback,
        });

        auto task_buffer_id = TaskBufferId{{.index = static_cast<u32>(impl.impl_task_buffers.size() - 1)}};

        impl.tasks.push_back(ImplCreateBufferTask{
            .id = task_buffer_id,
        });

        return task_buffer_id;
    }

    auto TaskList::create_task_image(TaskImageInfo const & info) -> TaskImageId
    {
        auto & impl = *as<ImplTaskList>();
        DAXA_DBG_ASSERT_TRUE_M(!impl.compiled, "can only record to uncompleted task list");

        usize task_index = impl.tasks.size() + 1;

        impl.impl_task_images.push_back(ImplTaskImage{
            .latest_access = info.last_access,
            .latest_layout = info.last_layout,
            .latest_access_task_index = task_index,
            .fetch_callback = info.fetch_callback,
            .slice = info.slice,
        });

        auto task_image_id = TaskImageId{{.index = static_cast<u32>(impl.impl_task_images.size() - 1)}};

        impl.tasks.push_back(ImplCreateImageTask{
            .id = task_image_id,
        });

        return task_image_id;
    }

    void TaskList::add_task(TaskInfo const & info)
    {
        auto & impl = *as<ImplTaskList>();
        DAXA_DBG_ASSERT_TRUE_M(!impl.compiled, "can only record to uncompleted task list");
        impl.tasks.push_back(ImplGenericTask{.info = info});
    }

    void TaskList::add_render_task(TaskRenderInfo const & info)
    {
        auto & impl = *as<ImplTaskList>();
        DAXA_DBG_ASSERT_TRUE_M(!impl.compiled, "can only record to uncompleted task list");

        auto convert_attachment = [&](TaskRenderAttachmentInfo const & attach, auto & images, auto usage) -> RenderAttachmentInfo
        {
            ImageLayout layout =
                attach.layout_override != ImageLayout::UNDEFINED ? attach.layout_override : ImageLayout::COLOR_ATTACHMENT_OPTIMAL;
            DAXA_DBG_ASSERT_TRUE_M(!attach.image.is_empty(), "must declare image for render pass attachment");
            images.push_back({attach.image, usage});

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
        auto & impl = *as<ImplTaskList>();
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
        auto & impl = *as<ImplTaskList>();
        DAXA_DBG_ASSERT_TRUE_M(!impl.compiled, "Can only compile a task list one time");
        impl.compiled = true;
        impl.insert_synchronization();
    }

    void TaskList::execute()
    {
        auto & impl = *as<ImplTaskList>();
        DAXA_DBG_ASSERT_TRUE_M(impl.compiled, "Can only execute a completed task list");

        TaskRuntime runtime{
            .current_device = impl.info.device,
            .current_command_list = impl.info.device.create_command_list({.debug_name = {std::string(impl.info.debug_name) + " generated Command List"}}),
            .impl_task_buffers = impl.impl_task_buffers,
            .impl_task_images = impl.impl_task_images,
        };

        for (usize task_index = 0; task_index < impl.tasks.size(); ++task_index)
        {
            runtime.execute_task(impl.tasks[task_index], task_index);
        }

        runtime.current_command_list.complete();

        // TODO: submit and present
    }

    void TaskRuntime::execute_task(TaskVariant & task_variant, usize task_index)
    {
        DAXA_ONLY_IF_TASK_LIST_DEBUG(std::cout << "execute task (index: " << task_index << ")" << std::endl);

        if (ImplGenericTask * generic_task = std::get_if<ImplGenericTask>(&task_variant))
        {
            DAXA_ONLY_IF_TASK_LIST_DEBUG(std::cout << "  executing ImplGenericTask (name: " << generic_task->info.debug_name << ")" << std::endl);

            this->pipeline_barriers(generic_task->barriers);
            auto interface = TaskInterface(this, &generic_task->info.resources);
            generic_task->info.task(interface);
        }
        else if (ImplCreateBufferTask * create_buffer_task = std::get_if<ImplCreateBufferTask>(&task_variant))
        {
            DAXA_ONLY_IF_TASK_LIST_DEBUG(std::cout << "  executing ImplCreateBufferTask" << std::endl);

            ImplTaskBuffer & impl_task_buffer = this->impl_task_buffers[create_buffer_task->id.index];
            BufferId buffer_id = impl_task_buffer.fetch_callback();
            this->runtime_buffers.push_back(RuntimeTaskBuffer{
                .buffer_id = buffer_id,
            });
        }
        else if (ImplCreateImageTask * create_image_task = std::get_if<ImplCreateImageTask>(&task_variant))
        {
            DAXA_ONLY_IF_TASK_LIST_DEBUG(std::cout << "  executing ImplCreateImageTask" << std::endl);

            ImplTaskImage & impl_task_image = this->impl_task_images[create_image_task->id.index];
            ImageId image_id = impl_task_image.fetch_callback();
            ImageViewId image_view_id = image_id.default_view();
            auto image_slice = this->current_device.info_image_view(image_view_id).slice;

            if (image_slice != impl_task_image.slice)
            {
                ImageViewInfo image_view_info = this->current_device.info_image_view(image_view_id);
                image_view_info.slice = impl_task_image.slice;

                // TODO(pahrens): This leaks. The view is never destroyed. Make sure the runtime kills it in the end of execution.
                image_view_id = this->current_device.create_image_view(image_view_info);
            }

            this->runtime_images.push_back(RuntimeTaskImage{
                .image_id = image_id,
                .image_view_id = image_view_id,
            });
        }
    }

    void TaskRuntime::pipeline_barriers(std::vector<TaskPipelineBarrier> const & barriers)
    {
        for (auto const & barrier : barriers)
        {
            if (barrier.image_barrier)
            {
                ImplTaskImage const & task_image = this->impl_task_images[barrier.image_id.index];
                RuntimeTaskImage const & runtime_image = this->runtime_images[barrier.image_id.index];

                this->current_command_list.pipeline_barrier_image_transition({
                    .awaited_pipeline_access = barrier.awaited_pipeline_access,
                    .waiting_pipeline_access = barrier.waiting_pipeline_access,
                    .before_layout = barrier.before_layout,
                    .after_layout = barrier.after_layout,
                    .image_id = runtime_image.image_id,
                    .image_slice = task_image.slice,
                });
            }
            else
            {
                this->current_command_list.pipeline_barrier({
                    .awaited_pipeline_access = barrier.awaited_pipeline_access,
                    .waiting_pipeline_access = barrier.waiting_pipeline_access,
                });
            }
        }
    }

    ImplTaskList::ImplTaskList(TaskListInfo const & info)
        : info{info}
    {
    }

    ImplTaskList::~ImplTaskList()
    {
    }

    auto ImplTaskList::managed_cleanup() -> bool
    {
        return true;
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
        case TaskBufferAccess::HOST_TRANSFER_READ: return {PipelineStageFlagBits::HOST, AccessTypeFlagBits::READ};
        case TaskBufferAccess::HOST_TRANSFER_WRITE: return {PipelineStageFlagBits::HOST, AccessTypeFlagBits::WRITE};
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
        DAXA_ONLY_IF_TASK_LIST_DEBUG(std::cout << "insert task list" << std::endl);
        for (usize task_index = 0; task_index < this->tasks.size(); ++task_index)
        {
            if (ImplGenericTask * task_ptr = std::get_if<ImplGenericTask>(&tasks[task_index]))
            {
                auto & task = *task_ptr;

                DAXA_ONLY_IF_TASK_LIST_DEBUG(std::cout
                                             << "  process task index : "
                                             << task_index
                                             << "\n  {"
                                             << std::endl);

                TaskResources & resources = task.info.resources;

                for (auto & [task_buffer_id, t_access] : resources.buffers)
                {
                    ImplTaskBuffer & task_buffer = this->impl_task_buffers[task_buffer_id.index];

                    // slot(task_buffer_id).latest_access = usage;
                    auto new_access = task_buffer_access_to_access(t_access);
                    auto & latest_access = task_buffer.latest_access;

                    DAXA_ONLY_IF_TASK_LIST_DEBUG(std::cout
                                                 << "    task access: buffer tid: "
                                                 << task_buffer_id.index
                                                 << ",\n      previous access: "
                                                 << to_string(task_buffer.latest_access)
                                                 << ",\n      new access: "
                                                 << to_string(new_access)
                                                 << std::endl);

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
                        usize latest_access_task_index = task_buffer.latest_access_task_index;

                        usize barrier_task_index = {};

                        if (latest_access_task_index >= this->last_task_index_with_barrier)
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
                        std::get_if<ImplGenericTask>(&tasks[barrier_task_index])->barriers.push_back(TaskPipelineBarrier{
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
                    task_buffer.latest_access_task_index = task_index;
                }

                for (auto & [task_image_id, t_access] : resources.images)
                {
                    ImplTaskImage & task_image = this->impl_task_images[task_image_id.index];

                    auto [new_layout, new_access] = task_image_access_to_layout_access(t_access);
                    auto & latest_layout = task_image.latest_layout;
                    auto & latest_access = task_image.latest_access;

                    DAXA_ONLY_IF_TASK_LIST_DEBUG(std::cout
                                                 << "    task access: image task_buffer_id: "
                                                 << task_image_id.index
                                                 << ",\n      previous access: "
                                                 << to_string(task_image.latest_access)
                                                 << ",\n      previous layout: "
                                                 << to_string(task_image.latest_layout)
                                                 << ",\n      new access: "
                                                 << to_string(new_access)
                                                 << ",\n      new layout: "
                                                 << to_string(new_layout)
                                                 << std::endl);

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
                        usize latest_access_task_index = task_image.latest_access_task_index;

                        usize barrier_task_index = {};

                        if (latest_access_task_index >= this->last_task_index_with_barrier)
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
                        std::get_if<ImplGenericTask>(&tasks[barrier_task_index])->barriers.push_back(TaskPipelineBarrier{
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
                            .image_id = task_image_id,
                            .image_slice = task_image.slice,
                        });

                        DAXA_ONLY_IF_TASK_LIST_DEBUG(std::cout
                                                     << "    inserted barrier at task index: "
                                                     << task_index
                                                     << ",\n      task image index: "
                                                     << task_image_id.index
                                                     << ",\n      awaited_pipeline_access: "
                                                     << to_string(std::get_if<ImplGenericTask>(&tasks[barrier_task_index])->barriers.back().awaited_pipeline_access)
                                                     << ",\n      waiting_pipeline_access: "
                                                     << to_string(std::get_if<ImplGenericTask>(&tasks[barrier_task_index])->barriers.back().waiting_pipeline_access)
                                                     << ",\n      before_layout: "
                                                     << to_string(std::get_if<ImplGenericTask>(&tasks[barrier_task_index])->barriers.back().before_layout)
                                                     << ",\n      after_layout: "
                                                     << to_string(std::get_if<ImplGenericTask>(&tasks[barrier_task_index])->barriers.back().after_layout)
                                                     << std::endl);

                        latest_layout = new_layout;
                        latest_access = new_access;
                    }
                    else
                    {
                        latest_access = latest_access | new_access;
                    }
                    task_image.latest_access_task_index = task_index;
                }
                DAXA_ONLY_IF_TASK_LIST_DEBUG(std::cout
                                             << "  }\n"
                                             << std::endl);
            }
        }
    }
} // namespace daxa
