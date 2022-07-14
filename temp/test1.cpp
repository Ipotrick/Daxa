#include <iostream>

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <daxa/daxa.hpp>

int main()
{
    using namespace daxa::types;

    auto daxa_ctx = daxa::create_context({
        .enable_validation = true,
        .validation_callback = [](daxa::MsgSeverity s, daxa::MsgType, std::string_view msg)
        {
            switch (s)
            {
            // clang-format off
            case daxa::MsgSeverity::VERBOSE: std::cout << "[VERBOSE]: " << msg << std::endl; break;
            case daxa::MsgSeverity::INFO:    std::cout << "[INFO]:    " << msg << std::endl; break;
            case daxa::MsgSeverity::WARNING: std::cout << "[WARNING]: " << msg << std::endl; break;
            case daxa::MsgSeverity::FAILURE: std::cout << "[FAILURE]: " << msg << std::endl; break;
            // clang-format on
            default: std::cout << "[UNKNOWN]: " << msg << std::endl; break;
            }
        },
    });

    auto device = daxa_ctx.create_default_device();

    glfwInit();
    struct WindowData
    {
        u32 size_x, size_y;
    };
    WindowData window_data{.size_x = 800, .size_y = 600};
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow * glfw_window_ptr = glfwCreateWindow(static_cast<i32>(window_data.size_x), static_cast<i32>(window_data.size_y), "test1", nullptr, nullptr);
    glfwSetWindowUserPointer(glfw_window_ptr, &window_data);
    glfwSetWindowSizeCallback(
        glfw_window_ptr,
        [](GLFWwindow * glfw_window_ptr, i32 sx, i32 sy)
        {
            auto & window_data = *reinterpret_cast<WindowData *>(glfwGetWindowUserPointer(glfw_window_ptr));
            window_data.size_x = static_cast<u32>(sx);
            window_data.size_y = static_cast<u32>(sy);
        });
    auto swapchain = device.create_swapchain({
        .native_window_handle = glfwGetWin32Window(glfw_window_ptr),
        .width = window_data.size_x,
        .height = window_data.size_y,
        .image_usage = daxa::ImageUsageFlagBits::TRANSFER_DST,
        .debug_name = "Test1 Swapchain",
    });

    auto pipeline_compiler = device.create_pipeline_compiler({.root_paths = {"temp/shaders"},
                                                              .debug_name = "Test1 Pipeline Compiler"});
    auto compute_pipeline = pipeline_compiler.create_compute_pipeline({
        .shader_info = {.path_to_source = "temp/shaders/test1_comp.hlsl"},
        .debug_name = "Test1 Compute Pipeline",
    });

    while (true)
    {
        auto prev_window_data = window_data;
        glfwPollEvents();
        if (glfwWindowShouldClose(glfw_window_ptr))
        {
            break;
        }

        if (prev_window_data.size_x != window_data.size_x || prev_window_data.size_y != window_data.size_y)
        {
            swapchain.resize(window_data.size_x, window_data.size_y);
        }

        auto img = swapchain.acquire_next_image();

        auto binary_semaphore = device.create_binary_semaphore({});
        auto cmd_list = device.create_command_list({
            .debug_name = "Test1 Command List",
        });
        cmd_list.clear_image(
            {
                .dst_image = img,
                .dst_image_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
                .dst_slice = daxa::ImageMipArraySlice{
                    .image_aspect = daxa::ImageAspectFlags::COLOR,
                    .base_mip_level = 0,
                    .level_count = 1,
                    .base_array_layer = 0,
                    .layer_count = 1,
                },
                .clear_color = daxa::ClearColor{.f32_value = {0.30f, 0.10f, 1.00f, 1.00f}},
            });

        cmd_list.complete();

        device.submit_commands({
            .command_lists = {cmd_list},
            .signal_binary_on_completion = binary_semaphore,
        });

        device.present_frame({
            .wait_on_binary = binary_semaphore,
            .swapchain = swapchain,
        });
    }
}
