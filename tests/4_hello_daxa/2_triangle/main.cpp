#include <daxa/daxa.hpp>

// Include the pipeline manager
#include <daxa/utils/pipeline_manager.hpp>
// We'll also include iostream, since we now use it
#include <iostream>

// We're going to use another optional feature of Daxa,
// called TaskGraph. We'll explain more below.
#include <daxa/utils/task_graph.hpp>

#include "shared.inl"

#include <GLFW/glfw3.h>
#if defined(_WIN32)
#define GLFW_EXPOSE_NATIVE_WIN32
#elif defined(__linux__)
#define GLFW_EXPOSE_NATIVE_X11
#endif
#include <GLFW/glfw3native.h>

auto get_native_handle(GLFWwindow * glfw_window_ptr) -> daxa::NativeWindowHandle
{
#if defined(_WIN32)
    return glfwGetWin32Window(glfw_window_ptr);
#elif defined(__linux__)
    return reinterpret_cast<daxa::NativeWindowHandle>(glfwGetX11Window(glfw_window_ptr));
#endif
}

auto get_native_platform(GLFWwindow * /*unused*/) -> daxa::NativeWindowPlatform
{
#if defined(_WIN32)
    return daxa::NativeWindowPlatform::WIN32_API;
#elif defined(__linux__)
    return daxa::NativeWindowPlatform::XLIB_API;
#endif
}

struct WindowInfo
{
    daxa::u32 width{}, height{};
    bool swapchain_out_of_date = false;
};

// We declare a static task in two parts.
// The first part is called the task "head".
// The second part is the actual task class we declare. The task struct inherits from a partial task declared by the head.
// This split might seems strange, especially that the heads are declared as macros.
// This has a good reason: we can have task heads within shader/host shared .inl files!
// The heads automatically declare a struct that is automatically filled by the task graph representing your attachments!
// This is can be very convenient.
// In order to make this properly work in c++ we had to wrap it all in preprocessor macros.
// The head for DrawToSwapchainTask is declared within the shared file.

// Check out the shader/c++ shared code in shared.inl

struct DrawToSwapchainTask : DrawToSwapchainH::Task
{
    AttachmentViews views = {};             // This field must be declared in every task that inherits from a head.
    daxa::RasterPipeline * pipeline = {};
    void callback(daxa::TaskInterface ti)
    {
        {
            // The task head declares a list of indices, one for each attachment.
            // They are inside a constant under the heads namespace:
            [[maybe_unused]] daxa::TaskImageAttachmentIndex image_attach_index = DrawToSwapchainH::AT.color_target;
        }
        {
            // For convenience, each task that derives from the heads base task
            // can also access it directly without the heads namespace:
            [[maybe_unused]] daxa::TaskImageAttachmentIndex image_attach_index = AT.color_target;
        }
        
        // The task interface provides a way to get the attachment info:
        auto image_info = ti.info(AT.color_target).value();
        auto image_id = ti.id(AT.color_target);
        auto image_view_id = ti.view(AT.color_target);
        auto image_layout = ti.layout(AT.color_target);

        // Same for buffers:
        auto buffer_info = ti.info(AT.vertices).value();
        auto buffer_id = ti.id(AT.vertices);
        auto buffer_host_address = ti.buffer_host_address(AT.vertices).value();
        auto buffer_device_address = ti.buffer_device_address(AT.vertices).value();

        // When starting a render pass via a rasterization pipeline, daxa "eats" a generic command recorder
        // and turns it into a RenderCommandRecorder.
        // Only the RenderCommandRecorder can record raster commands.
        // The RenderCommandRecorder can only record commands that are valid within a render pass.
        // This way daxa ensures typesafety for command recording.
        daxa::RenderCommandRecorder render_recorder = std::move(ti.recorder).begin_renderpass({
            .color_attachments = std::array{
                daxa::RenderAttachmentInfo{
                    .image_view = ti.view(AT.color_target),
                    .load_op = daxa::AttachmentLoadOp::CLEAR,
                    .clear_value = std::array<daxa::f32, 4>{0.1f, 0.0f, 0.5f, 1.0f},
                },
            },
            .render_area = {.width = image_info.size.x, .height = image_info.size.y},
        });
        // Here, we'll bind the pipeline to be used in the draw call below
        render_recorder.set_pipeline(*pipeline);

        // Very importantly, task graph packs up our attachment shader data into a byte blob.
        // We need to pass this blob to our shader somehow.
        // The typical way to do this is to assign the blob to the push constant.
        render_recorder.push_constant(MyPushConstant {
            .attachments = ti.attachment_shader_blob,
        });
        // and issue the draw call with the desired number of vertices.
        render_recorder.draw({.vertex_count = 3});

        // VERY IMPORTANT! A renderpass must be ended after finishing!
        // The ending of a render pass returns back the original command recorder.
        // Assign it back to the task interfaces command recorder.
        ti.recorder = std::move(render_recorder).end_renderpass();
    }
};

auto main() -> int
{
    auto window_info = WindowInfo{.width = 800, .height = 600};
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    auto * glfw_window_ptr = glfwCreateWindow(
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
    auto * native_window_handle = get_native_handle(glfw_window_ptr);
    auto native_window_platform = get_native_platform(glfw_window_ptr);

    daxa::Instance instance = daxa::create_instance({});

    // Let instance auto select a device
    daxa::Device device = instance.create_device_2(instance.choose_device({}, {}));

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
        .root_paths = {
            DAXA_SHADER_INCLUDE_DIR,
            "./tests/4_hello_daxa/2_triangle",
        },
        .default_language = daxa::ShaderLanguage::GLSL,
        .default_enable_debug_info = true,
        .name = "my pipeline manager",
    });
    // Then just adding it to the pipeline manager
    std::shared_ptr<daxa::RasterPipeline> pipeline;
    {
        auto result = pipeline_manager.add_raster_pipeline2({
            .vertex_shader_info = daxa::ShaderCompileInfo2{.source = daxa::ShaderFile{"main.glsl"}},
            .fragment_shader_info = daxa::ShaderCompileInfo2{.source = daxa::ShaderFile{"main.glsl"}},
            .color_attachments = {{.format = swapchain.get_format()}},
            .raster = {},
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
        .allocate_info = daxa::MemoryFlagBits::HOST_ACCESS_SEQUENTIAL_WRITE, // Allows us to write VRAM using a host pointer.
        .name = "my vertex data",
    });

    // To upload the vertex data, we query the buffers pointer and write the vertices directly.
    std::array<MyVertex, 3>* vert_buf_ptr = device.buffer_host_address_as<std::array<MyVertex, 3>>(buffer_id).value();
    *vert_buf_ptr = std::array{
        MyVertex{.position = {-0.5f, +0.5f, 0.0f}, .color = {1.0f, 0.0f, 0.0f}},
        MyVertex{.position = {+0.5f, +0.5f, 0.0f}, .color = {0.0f, 1.0f, 0.0f}},
        MyVertex{.position = {+0.0f, -0.5f, 0.0f}, .color = {0.0f, 0.0f, 1.0f}},
    };

    // When using TaskGraph, we must create "virtual" resources (we call
    // them task resources) whose usages are tracked, allowing for correct
    // synchronization for them.

    // The first we'll make is the swapchain image task resource
    // We could immediately give this task image a image id.
    // But in the case of the swapchain images we need to reacquire a new image every frame.
    auto task_swapchain_image = daxa::TaskImage{{.swapchain_image = true, .name = "swapchain image"}};

    // We will also create a buffer task resource, for our MyVertex buffer buffer_id
    // We do something a little special here, which is that we set the initial access
    // of the buffer to be vertex shader read, and that's because we'll create a task
    // list which will upload the buffer
    auto task_vertex_buffer = daxa::TaskBuffer({
        .initial_buffers = {.buffers = std::array{buffer_id}},
        .name = "task vertex buffer",
    });

    auto loop_task_graph = daxa::TaskGraph({
        .device = device,
        .swapchain = swapchain,
        .name = "loop",
    });

    // We need to explicitly declare all uses of persistent task resources!
    // This not a technical limitation but an intentional choice.
    // Manually marking used resources makes it possible to detect errors in your graph recording.
    loop_task_graph.use_persistent_buffer(task_vertex_buffer);
    loop_task_graph.use_persistent_image(task_swapchain_image);

    // And a task to draw to the screen
    loop_task_graph.add_task(DrawToSwapchainTask{
        .views = DrawToSwapchainTask::Views{
            .color_target = task_swapchain_image,
            .vertices = task_vertex_buffer,
        },
        .pipeline = pipeline.get(),
    });

    // We now need to tell the task graph that these commands will be submitted,
    // and that we have no additional information to provide. This exists in
    // order to allow more advanced Vulkan users to do much more complicated
    // things, that we don't care about, and that you can create a whole app
    // without ever touching.
    loop_task_graph.submit({});

    // And tell the task graph to do the present step.
    loop_task_graph.present({});
    // Finally, we complete the task graph, which essentially compiles the
    // dependency graph between tasks, and inserts the most optimal synchronization!
    loop_task_graph.complete({});

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

        // acquire the next image as usual,
        auto swapchain_image = swapchain.acquire_next_image();
        if (swapchain_image.is_empty())
        {
            continue;
        }
        // We update the image id of the task swapchain image.
        task_swapchain_image.set_images({.images = std::span{&swapchain_image, 1}});

        // So, now all we need to do is execute our task graph!
        loop_task_graph.execute({});

        // The device performs all memory reclaiming in the collect_garbage call.
        // Usually its the best to call it once at the end of each frame.
        device.collect_garbage();
    }

    device.destroy_buffer(buffer_id);
    // No need to call wait idle or collect garbage here,
    // as the device will automatically call these when its lifetime ends.
    // device.wait_idle();
    // device.collect_garbage();
}