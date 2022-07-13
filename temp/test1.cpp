#include <iostream>

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <daxa/daxa.hpp>

int main()
{
    using namespace daxa::types;

    auto daxa_ctx = daxa::create_context({
        .enable_validation = false,
        .validation_callback = [](daxa::MsgSeverity s, daxa::MsgType, std::string_view msg)
        {
            switch (s)
            {
            // clang-format off
            // case daxa::MsgSeverity::VERBOSE: std::cout << "[VERBOSE]: " << msg << std::endl; break;
            // case daxa::MsgSeverity::INFO:    std::cout << "[INFO]:    " << msg << std::endl; break;
            case daxa::MsgSeverity::WARNING: std::cout << "[WARNING]: " << msg << std::endl; break;
            case daxa::MsgSeverity::FAILURE: std::cout << "[FAILURE]: " << msg << std::endl; break;
            // clang-format on
            default: break;
            }
        },
    });

    // auto device_selector = [](daxa::DeviceInfo const & device_info) -> daxa::types::i32
    // {
    //     if (device_info.device_type != daxa::DeviceType::DISCRETE_GPU)
    //     {
    //         return -1;
    //     }
    //     return device_info.limits.max_memory_allocation_count;
    // };
    // auto device = daxa_ctx.create_device(device_selector).value();

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
    });

    auto pipeline_compiler = device.create_pipeline_compiler({
        .root_paths = {"temp/shaders"},
    });
    auto compute_pipeline = pipeline_compiler.create_compute_pipeline({
        .shader_info = {.path_to_source = "temp/shaders/test1_comp.hlsl"},
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
    }
}
