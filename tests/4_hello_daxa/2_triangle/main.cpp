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
        .surface_format_selector = [](daxa::Format format, daxa::ColorSpace colorspace)
        {
            switch (format)
            {
            case daxa::Format::R8G8B8A8_UINT: return 100;
            default: return daxa::default_format_score(format, colorspace);
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
    daxa::BufferId vertex_buffer = device.create_buffer({
        .size = sizeof(MyVertex) * 3,
        .allocate_info = daxa::MemoryFlagBits::HOST_ACCESS_SEQUENTIAL_WRITE, // Will allocate buffer in host visible vram
        .name = "vertex buffer",
    });

    // To upload the vertex data, we query the buffers pointer and write the vertices directly.
    MyVertex * vert_buf_ptr = device.buffer_host_address_as<MyVertex>(vertex_buffer).value();
    vert_buf_ptr[0] = {.position = {-0.5f, +0.5f, 0.0f}, .color = {1.0f, 0.0f, 0.0f}};
    vert_buf_ptr[1] = {.position = {+0.5f, +0.5f, 0.0f}, .color = {0.0f, 1.0f, 0.0f}};
    vert_buf_ptr[2] = {.position = {+0.0f, -0.5f, 0.0f}, .color = {0.0f, 0.0f, 1.0f}};

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

        // acquire_next_image will wait until a frame in flight is available, then attempt to acquire a new swapchain image.
        // If the acquisition fails, it will return a null image id (is_empty() -> true).
        daxa::ImageId swapchain_image = swapchain.acquire_next_image();
        if (swapchain_image.is_empty())
        {
            continue;
        }

        // Record and submit frame gpu commands
        {
            daxa::CommandRecorder recorder = device.create_command_recorder({.name = "Main Loop Cmd Recorder"});

            // Daxa stores and maintains meta information about all created resources such as images: 
            // The result of most image/buffer device functions return an optional that will be nullopt in the case that the given id is invalid.
            daxa::ImageInfo swapchain_image_info = device.image_info(swapchain_image).value();

            // Daxa uses image layout GENERAL for all access, generally there is no need for image pipeline barriers in daxa.
            // The usage of layout general for everything is efficient on modern gpus.
            // Vulkan image layouts are mostly a thing for mobile gpus and older AMD gpus.
            // Modern gpus no longer care, so daxa abstracts layouts completely.
            // There are two transitions that must still be done:
            // * initialization of an image before its first usage via daxa::ImageLayoutOperation::TO_GENERAL
            // * conversion to a presentable format via daxa::ImageLayoutOperation::TO_PRESENT_SRC
            // This tutorial shows both these transitions.
            recorder.pipeline_image_barrier({
                .dst_access = daxa::AccessConsts::COLOR_ATTACHMENT_OUTPUT_READ_WRITE,
                .image_id = swapchain_image,
                .layout_operation = daxa::ImageLayoutOperation::TO_GENERAL,
            });

            // When starting a render pass via a rasterization pipeline, the cmd recorder type is changed to daxa::RenderCommandRecorder.
            // Only the RenderCommandRecorder can record raster commands.
            // The RenderCommandRecorder can only record commands that are valid within a render pass.
            // This way daxa ensures type safety for command recording.
            daxa::RenderCommandRecorder render_recorder = std::move(recorder).begin_renderpass({
                .color_attachments = std::array{
                    daxa::RenderAttachmentInfo{
                        .image_view = swapchain_image.default_view(),
                        .load_op = daxa::AttachmentLoadOp::CLEAR,
                        .clear_value = std::array<daxa::f32, 4>{0.1f, 0.0f, 0.5f, 1.0f},
                    },
                },
                .render_area = {.width = swapchain_image_info.size.x, .height = swapchain_image_info.size.y},
            });
            // Here, we'll bind the pipeline to be used in the draw call below
            render_recorder.set_pipeline(*pipeline);

            // The only way to send data to a shader in daxa is via push constants.
            // In this case, we need to send the buffer pointer to the vertices to the gpu via a push constant:            
            render_recorder.push_constant(MyPushConstant{ .vertices = device.device_address(vertex_buffer).value() });
            
            render_recorder.draw({.vertex_count = 3});

            // VERY IMPORTANT! A renderpass must be ended after finishing!
            // The ending of a render pass returns back the original command recorder.
            // Assign it back to the task interfaces command recorder.
            recorder = std::move(render_recorder).end_renderpass();

            recorder.pipeline_image_barrier({
                .src_access = daxa::AccessConsts::COLOR_ATTACHMENT_OUTPUT_READ_WRITE,
                .image_id = swapchain_image,
                .layout_operation = daxa::ImageLayoutOperation::TO_PRESENT_SRC,
            });

            daxa::ExecutableCommandList cmd_list = recorder.complete_current_commands();

            // For convenience, the swapchain maintains:
            // * frames in flight
            // * a set of acquire semaphores (one for each frame in flight)
            // * a set of present semaphores (one for each swapchain image)
            
            // There are a few usage rules around the swapchain images in vulkan and daxa:
            // * all submitted commands that access a acquired swapchain image, MUST wait on the current acquire semaphore
            // * the last submitted commands accessing the swapchain image to be presented MUST signal the current present semaphore
            // * the last submitted commands accessing the swapchain image MUST signal the current timeline pair
            // * the present MUST wait in the current present semaphore

            device.submit_commands({
                .command_lists = std::array{ cmd_list },
                .wait_binary_semaphores = std::array{ swapchain.current_acquire_semaphore() },
                .signal_binary_semaphores = std::array{ swapchain.current_present_semaphore() },
                .signal_timeline_semaphores = std::array{ swapchain.current_timeline_pair() },
            });

            device.present_frame({
                .swapchain = swapchain,
                .wait_binary_semaphores = std::array{ swapchain.current_present_semaphore() },
            });
        }

        // The device performs all memory reclaiming in the collect_garbage call.
        // Usually its the best to call it once at the end of each frame.
        device.collect_garbage();
    }

    device.destroy_buffer(vertex_buffer);
    // No need to call wait idle or collect garbage here,
    // as the device will automatically call these when its lifetime ends.
    // device.wait_idle();
    // device.collect_garbage();
}