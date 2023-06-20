#include <daxa/daxa.hpp>

// Include the pipeline manager
#include <daxa/utils/pipeline_manager.hpp>
// We'll also include iostream, since we now use it
#include <iostream>

// We're going to use another optional feature of Daxa,
// called TaskList. We'll explain more below.
#include <daxa/utils/task_list.hpp>

#include <GLFW/glfw3.h>
#if defined(_WIN32)
#define GLFW_EXPOSE_NATIVE_WIN32
#elif defined(__linux__)
#define GLFW_EXPOSE_NATIVE_X11
#elif defined(__APPLE__)
#define GLFW_EXPOSE_NATIVE_COCOA
#endif
#include <GLFW/glfw3native.h>

auto get_native_handle(GLFWwindow * glfw_window_ptr) -> daxa::NativeWindowHandle
{
#if defined(_WIN32)
    return glfwGetWin32Window(glfw_window_ptr);
#elif defined(__linux__)
    return reinterpret_cast<daxa::NativeWindowHandle>(glfwGetX11Window(glfw_window_ptr));
#elif defined(__APPLE__)
    return glfwGetCocoaWindow(glfw_window_ptr);
#else
    return {};
#endif
}

auto get_native_platform(GLFWwindow * /*unused*/) -> daxa::NativeWindowPlatform
{
#if defined(_WIN32)
    return daxa::NativeWindowPlatform::WIN32_API;
#elif defined(__linux__)
    return daxa::NativeWindowPlatform::XLIB_API;
#elif defined(__APPLE__)
    return daxa::NativeWindowPlatform::COCOA_API;
#else
    return daxa::NativeWindowPlatform::UNKNOWN;
#endif
}

struct WindowInfo
{
    daxa::u32 width{}, height{};
    bool swapchain_out_of_date = false;
};

// We also create a couple files for the shader, the shared.inl and the main.glsl
#include "shared.inl"

void upload_vertex_data_task(daxa::Device & device, daxa::CommandList & cmd_list, daxa::BufferId buffer_id);
void draw_to_swapchain_task(daxa::Device & device, daxa::CommandList & cmd_list, std::shared_ptr<daxa::RasterPipeline> & pipeline, daxa::ImageId swapchain_image, daxa::BufferId buffer_id, daxa::u32 width, daxa::u32 height);

auto main() -> int
{
    auto window_info = WindowInfo{.width = 800, .height = 600};
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    auto *glfw_window_ptr = glfwCreateWindow(
        static_cast<daxa::i32>(window_info.width),
        static_cast<daxa::i32>(window_info.height),
        "Daxa sample window name", nullptr, nullptr);
    glfwSetWindowUserPointer(glfw_window_ptr, &window_info);
    glfwSetWindowSizeCallback(
        glfw_window_ptr,
        [](GLFWwindow * glfw_window, int width, int height)
        {
            auto & window_info_ref = *reinterpret_cast<WindowInfo *>(glfwGetWindowUserPointer(glfw_window));
            window_info_ref.swapchain_out_of_date = true;
            window_info_ref.width = static_cast<daxa::u32>(width);
            window_info_ref.height = static_cast<daxa::u32>(height);
        });
    auto *native_window_handle = get_native_handle(glfw_window_ptr);
    auto native_window_platform = get_native_platform(glfw_window_ptr);

    daxa::Context context = daxa::create_context({});

    daxa::Device device = context.create_device({
        .selector = [](daxa::DeviceProperties const & device_props) -> daxa::i32
        {
            daxa::i32 score = 0;
            switch (device_props.device_type)
            {
            case daxa::DeviceType::DISCRETE_GPU: score += 10000; break;
            case daxa::DeviceType::VIRTUAL_GPU: score += 1000; break;
            case daxa::DeviceType::INTEGRATED_GPU: score += 100; break;
            default: break;
            }
            score += static_cast<daxa::i32>(device_props.limits.max_memory_allocation_count / 100000);
            return score;
        },
        .name = "my device",
    });

    daxa::Swapchain swapchain = device.create_swapchain({
        .native_window = native_window_handle,
        .native_window_platform = native_window_platform,
        .surface_format_selector = [](daxa::Format format)
        {
            switch (format)
            {
            case daxa::Format::R8G8B8A8_UINT: return 100;
            default: return daxa::default_format_score(format);
            }
        },
        .present_mode = daxa::PresentMode::MAILBOX,
        .image_usage = daxa::ImageUsageFlagBits::TRANSFER_DST,
        .name = "my swapchain",
    });

    // Create a raster pipeline
    // first by creating the pipeline manager
    auto pipeline_manager = daxa::PipelineManager({
        .device = device,
        .shader_compile_options = {
            .root_paths = {
                DAXA_SHADER_INCLUDE_DIR,
                "./tests/4_hello_daxa/2_triangle",
            },
            .language = daxa::ShaderLanguage::GLSL,
            .enable_debug_info = true,
        },
        .name = "my pipeline manager",
    });
    // Then just adding it to the pipeline manager
    std::shared_ptr<daxa::RasterPipeline> pipeline;
    {
        auto result = pipeline_manager.add_raster_pipeline({
            .vertex_shader_info = daxa::ShaderCompileInfo{.source = daxa::ShaderFile{"main.glsl"}},
            .fragment_shader_info = daxa::ShaderCompileInfo{.source = daxa::ShaderFile{"main.glsl"}},
            .color_attachments = {{.format = swapchain.get_format()}},
            .raster = {},
            .push_constant_size = sizeof(MyPushConstant),
            .name = "my pipeline",
        });
        if (result.is_err())
        {
            std::cerr << result.message() << std::endl;
            return -1;
        }
        pipeline = result.value();
    }

    // Since we want to draw a triangle, we'll want to have data
    // to use in the shaders describing it. To do this, we could hard
    // code the data into the shader, but we might as well showcase
    // how to create and upload a buffer

    // We'll start by "allocating" the memory on the GPU by creating
    // a buffer with the device.
    auto buffer_id = device.create_buffer({
        .size = sizeof(MyVertex) * 3,
        .name = "my vertex data",
    });
    // Obviously the vertex data is not yet on the GPU, and this buffer
    // is just empty. We will use conditional TaskList to upload it on
    // just the first frame. More on this soon!

    // While not entirely necessary, we're going to use TaskList, which
    // allows us to compile a list of GPU tasks and their dependencies
    // into a synchronized set of commands. This simplifies your code
    // by making different tasks completely self-contained, while also
    // generating the most optimal synchronization for the tasks you
    // describe.

    // TaskList can have permutations, which allow for runtime conditions
    // to trigger different outcomes. These are identified with indices,
    // so we'll define an enum representing all the condition indices
    // since we want to name them and make sure they're all unique.
    enum class TaskCondition
    {
        VERTICES_UPLOAD,
        COUNT,
    };

    std::array<bool, static_cast<daxa::usize>(TaskCondition::COUNT)> task_condition_states{};

    auto loop_task_list = daxa::TaskList({
        .device = device,
        .swapchain = swapchain,
        .permutation_condition_count = static_cast<daxa::usize>(TaskCondition::COUNT),
        .name = "my task list",
    });

    // When using TaskList, we must create "virtual" resources (we call
    // them task resources) whose usages are tracked, allowing for correct
    // synchronization for them.

    // The first we'll make is the swapchain image task resource
    auto task_swapchain_image = loop_task_list.create_task_image({.swapchain_image = true, .name = "my task swapchain image"});

    // We'll also declare a swapchain image, since we're going to be
    // adding and removing it from being tracked by the TaskList.
    auto swapchain_image = daxa::ImageId{};

    // We then give the real swapchain image to this virtual resource,
    // in order to allow the TaskList to perform the necessary image
    // layout transitions on it.
    loop_task_list.add_runtime_image(task_swapchain_image, swapchain_image);

    // We will also create a buffer task resource, for our MyVertex buffer buffer_id
    // We do something a little special here, which is that we set the initial access
    // of the buffer to be vertex shader read, and that's because we'll create a task
    // list which will upload the buffer
    auto task_buffer_id = loop_task_list.create_task_buffer({.pre_task_list_slice_states = daxa::AccessConsts::VERTEX_SHADER_READ, .name = "my task buffer"});
    // adding this as a real resource just allows us to query it from the Task
    // runtime later, since there's no buffer-specific commands (unlike image
    // layout transitions) that need to happen when synchronizing buffer usages.
    loop_task_list.add_runtime_buffer(task_buffer_id, buffer_id);

    // Now we can record our tasks!

    // We'll first make a task to update the buffer. This doesn't need to be done
    // every frame, so we'll put it inside a task conditional!
    loop_task_list.conditional({
        .condition_index = static_cast<daxa::u32>(TaskCondition::VERTICES_UPLOAD),
        .when_true = [&loop_task_list, &task_buffer_id, &task_condition_states]()
        {
            loop_task_list.add_task({
                .used_buffers = {
                    // Since this task is going to copy a staging buffer to the
                    // actual buffer, we'll say that this task uses the buffer
                    // with a transfer write operation!
                    {task_buffer_id, daxa::TaskBufferAccess::TRANSFER_WRITE},
                },
                .task = [&task_buffer_id, &task_condition_states](daxa::TaskRuntimeInterface task_runtime)
                {
                    auto cmd_list = task_runtime.get_command_list();

                    // see upload_vertex_data_task(...) for more info on the
                    // uploading of the data itself.

                    // We'll call upload_vertex_data_task(...) with the buffer ID,
                    // which we can query from the task runtime.

                    upload_vertex_data_task(task_runtime.get_device(), cmd_list, task_runtime.get_buffers(task_buffer_id)[0]);

                    // Now we can reset this task condition, since it should
                    // only ever be executed when requested!
                    task_condition_states[static_cast<daxa::usize>(TaskCondition::VERTICES_UPLOAD)] = false;
                },
                .name = "my upload task",
            });
        },
    });

    // And a task to draw to the screen
    loop_task_list.add_task({
        .used_buffers = {
            // Now, since we're reading the buffer inside the vertex shader,
            // we'll say that we use it as such.
            {task_buffer_id, daxa::TaskBufferAccess::VERTEX_SHADER_READ_ONLY},
        },
        .used_images = {
            // And, we'll denote that we use the swapchain image as a color
            // attachment. At the end of the line, we default-construct the
            // ImageMipArraySlice, since all we need is the first mip and
            // first array slice.
            {task_swapchain_image, daxa::TaskImageAccess::COLOR_ATTACHMENT, daxa::ImageMipArraySlice{}},
        },
        .task = [task_swapchain_image, task_buffer_id, &pipeline, &window_info](daxa::TaskRuntimeInterface task_runtime)
        {
            auto cmd_list = task_runtime.get_command_list();
            // Here we can just get the buffer and image IDs from the runtime
            // and just pass them directly.
            draw_to_swapchain_task(
                task_runtime.get_device(), cmd_list, pipeline,
                task_runtime.get_images(task_swapchain_image)[0],
                task_runtime.get_buffers(task_buffer_id)[0],
                window_info.width, window_info.height);
        },
        .name = "my draw task",
    });

    // We now need to tell the task list that these commands will be submitted,
    // and that we have no additional information to provide. This exists in
    // order to allow more advanced Vulkan users to do much more complicated
    // things, that we don't care about, and that you can create a whole app
    // without ever touching.
    loop_task_list.submit({});

    // And tell the task list to do the present step.
    loop_task_list.present({});
    // Finally, we complete the task list, which essentially compiles the
    // dependency graph between tasks, and inserts the most optimal synchronization!
    loop_task_list.complete({});

    // We'll set our task condition states to make sure we use the permutation
    // where we upload the vertex data to the GPU. This will get set to false
    // when the task is run, so it will only upload the data once!
    task_condition_states[static_cast<daxa::usize>(TaskCondition::VERTICES_UPLOAD)] = true;

    while (true)
    {
        glfwPollEvents();
        if (glfwWindowShouldClose(glfw_window_ptr) != 0)
        {
            break;
        }

        if (window_info.swapchain_out_of_date)
        {
            swapchain.resize();
            window_info.swapchain_out_of_date = false;
        }

        // Using TaskList completely changes what our render loop looks like.
        // Now, all that we need to do is remove the previous swapchain image
        // from being the resource to transition by the task swapchain image,
        loop_task_list.remove_runtime_image(task_swapchain_image, swapchain_image);
        // acquire the next image as usual,
        swapchain_image = swapchain.acquire_next_image();
        // and add that new image into the Task Runtime to be handled.
        loop_task_list.add_runtime_image(task_swapchain_image, swapchain_image);
        if (swapchain_image.is_empty())
        {
            continue;
        }

        // So, now all we need to do is execute our task list!
        loop_task_list.execute({.permutation_condition_values = task_condition_states});
    }

    device.wait_idle();
    device.collect_garbage();
    device.destroy_buffer(buffer_id);
}

void upload_vertex_data_task(
    daxa::Device & device,
    daxa::CommandList & cmd_list,
    daxa::BufferId buffer_id)
{
    // This is the data we'll send to the GPU
    auto data = std::array{
        MyVertex{.position = {-0.5f, +0.5f, 0.0f}, .color = {1.0f, 0.0f, 0.0f}},
        MyVertex{.position = {+0.5f, +0.5f, 0.0f}, .color = {0.0f, 1.0f, 0.0f}},
        MyVertex{.position = {+0.0f, -0.5f, 0.0f}, .color = {0.0f, 0.0f, 1.0f}},
    };

    // In order to send the data to the GPU, we can create a
    // staging buffer, which has host access, so that we can then
    // issue a command to copy from this buffer to the dedicated
    // GPU memory.
    auto staging_buffer_id = device.create_buffer({
        .memory_flags = daxa::MemoryFlagBits::HOST_ACCESS_RANDOM,
        .size = sizeof(data),
        .name = "my staging buffer",
    });
    // We can also ask the command list to destroy this temporary buffer,
    // since we don't care about it living, but we DO need it to survive
    // through its usage on the GPU (which won't happen until after these
    // commands are submitted), so we tell the command list to destroy it
    // in a deferred fashion.
    cmd_list.destroy_buffer_deferred(staging_buffer_id);
    // Instead of doing this manually, we could use one of Daxa's
    // other useful utilities, "Mem", but it's simple enough for now.

    // We then get the memory mapped pointer of the staging buffer, and
    // write the data directly to it.
    auto * buffer_ptr = device.get_host_address_as<std::array<MyVertex, 3>>(staging_buffer_id);
    *buffer_ptr = data;

    // And finally, we can just copy the data from the staging buffer
    // to the actual buffer.
    cmd_list.copy_buffer_to_buffer({
        .src_buffer = staging_buffer_id,
        .dst_buffer = buffer_id,
        .size = sizeof(data),
    });
}

void draw_to_swapchain_task(
    daxa::Device & device,
    daxa::CommandList & cmd_list,
    std::shared_ptr<daxa::RasterPipeline> & pipeline,
    daxa::ImageId swapchain_image,
    daxa::BufferId buffer_id,
    daxa::u32 width, daxa::u32 height)
{
    cmd_list.begin_renderpass({
        .color_attachments = {
            {
                .image_view = swapchain_image.default_view(),
                .load_op = daxa::AttachmentLoadOp::CLEAR,
                .clear_value = std::array<daxa::f32, 4>{0.1f, 0.0f, 0.5f, 1.0f},
            },
        },
        .render_area = {.x = 0, .y = 0, .width = width, .height = height},
    });
    // Here, we'll bind the pipeline to be used in the draw call below
    cmd_list.set_pipeline(*pipeline);
    // Set the push constant specifically for the following draw call...
    cmd_list.push_constant(MyPushConstant{
        .my_vertex_ptr = device.get_device_address(buffer_id),
    });
    // and issue the draw call with the desired number of vertices.
    cmd_list.draw({.vertex_count = 3});
    cmd_list.end_renderpass();
}
