#include <iostream>
#include <thread>

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <daxa/daxa.hpp>

using namespace daxa::types;

template <typename App>
struct Window
{
    GLFWwindow * glfw_window_ptr;
    u32 size_x = 800, size_y = 600;
    bool minimized = false;

    Window()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfw_window_ptr = glfwCreateWindow(static_cast<i32>(size_x), static_cast<i32>(size_y), "test1", nullptr, nullptr);
        glfwSetWindowUserPointer(glfw_window_ptr, this);
        glfwSetWindowSizeCallback(
            glfw_window_ptr,
            [](GLFWwindow * window_ptr, i32 sx, i32 sy)
            {
                auto & app = *reinterpret_cast<App *>(glfwGetWindowUserPointer(window_ptr));
                app.on_resize(static_cast<u32>(sx), static_cast<u32>(sy));
            });
    }
};

struct App : Window<App>
{
    daxa::Context daxa_ctx = daxa::create_context({
        .enable_validation = true,
        .validation_callback = [](daxa::MsgSeverity s, daxa::MsgType, std::string_view msg)
        {
            switch (s)
            {
            // clang-format off
            case daxa::MsgSeverity::VERBOSE: /*std::cout << "[VERBOSE]: " << msg << std::endl;*/ break;
            case daxa::MsgSeverity::INFO:    /*std::cout << "[INFO]:    " << msg << std::endl;*/ break;
            case daxa::MsgSeverity::WARNING: std::cout << "[WARNING]: " << msg << std::endl; DAXA_DBG_ASSERT_TRUE_M(false, ""); break;
            case daxa::MsgSeverity::FAILURE: std::cout << "[FAILURE]: " << msg << std::endl; DAXA_DBG_ASSERT_TRUE_M(false, ""); break;
            // clang-format on
            default: std::cout << "[UNKNOWN]: " << msg << std::endl; break;
            }
        },
    });

    daxa::Device device = daxa_ctx.create_default_device();

    daxa::Swapchain swapchain = device.create_swapchain({
        .native_window_handle = glfwGetWin32Window(glfw_window_ptr),
        .width = size_x,
        .height = size_y,
        .image_usage = daxa::ImageUsageFlagBits::TRANSFER_DST,
        .debug_name = "Test1 Swapchain",
    });

    daxa::PipelineCompiler pipeline_compiler = device.create_pipeline_compiler({
        .root_paths = {"temp/shaders"},
        .debug_name = "Test1 Pipeline Compiler",
    });
    daxa::ComputePipeline compute_pipeline = pipeline_compiler.create_compute_pipeline({
        .shader_info = {.path_to_source = "temp/shaders/test1_comp.hlsl"},
        .debug_name = "Test1 Compute Pipeline",
    });

    App()
    {
    }

    bool update()
    {
        glfwPollEvents();
        if (glfwWindowShouldClose(glfw_window_ptr))
        {
            return true;
        }

        if (!minimized)
        {
            draw();
        }
        else
        {
            using namespace std::literals;
            std::this_thread::sleep_for(1ms);
        }

        return false;
    }

    void draw()
    {
        auto img = swapchain.acquire_next_image();

        auto binary_semaphore = device.create_binary_semaphore({});
        auto cmd_list = device.create_command_list({
            .debug_name = "Test1 Command List",
        });

        cmd_list.pipeline_barrier_image_transition({
            .waiting_pipeline_access = daxa::PipelineStageAccessFlagBits::TRANSFER_WRITE,
            .before_layout = daxa::ImageLayout::UNDEFINED,
            .after_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
            .image_id = img,
            .image_slice = {
                .image_aspect = daxa::ImageAspectFlagBits::COLOR,
                .level_count = 1,
                .layer_count = 1,
            },
        });

        cmd_list.clear_image({
            .dst_image_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
            .clear_color = daxa::ClearColor{.f32_value = {0.30f, 0.10f, 1.00f, 1.00f}},
            .dst_image = img,
            .dst_slice = daxa::ImageMipArraySlice{
                .image_aspect = daxa::ImageAspectFlagBits::COLOR,
                .base_mip_level = 0,
                .level_count = 1,
                .base_array_layer = 0,
                .layer_count = 1,
            },
        });

        cmd_list.pipeline_barrier_image_transition({
            .awaited_pipeline_access = daxa::PipelineStageAccessFlagBits::TRANSFER_WRITE,
            .before_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
            .after_layout = daxa::ImageLayout::PRESENT_SRC,
            .image_id = img,
            .image_slice = {
                .image_aspect = daxa::ImageAspectFlagBits::COLOR,
                .level_count = 1,
                .layer_count = 1,
            },
        });

        cmd_list.complete();

        device.submit_commands({
            .command_lists = {cmd_list},
            .signal_binary_semaphores_on_completion = {binary_semaphore},
        });

        device.present_frame({
            .wait_on_binary = binary_semaphore,
            .swapchain = swapchain,
        });

        device.collect_garbage();
    }

    void on_resize(u32 sx, u32 sy)
    {
        size_x = sx;
        size_y = sy;
        minimized = (sx == 0 || sy == 0);

        if (!minimized)
        {
            swapchain.resize(size_x, size_y);
            draw();
        }
    }
};

int main()
{
    App app = {};
    while (true)
    {
        if (app.update())
            break;
    }
}
