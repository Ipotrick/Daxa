#include <iostream>

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <daxa/daxa.hpp>

int main()
{
    auto daxa_ctx = daxa::create_context({
        .enable_validation = false,
        .validation_callback = [](daxa::MsgSeverity, daxa::MsgType, std::string_view msg)
        {
            std::cout << msg << std::endl;
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
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow * glfw_window_ptr = glfwCreateWindow(800, 600, "test1", nullptr, nullptr);
    device.create_swapchain(glfwGetWin32Window(glfw_window_ptr));

    while (true)
    {
        glfwPollEvents();
        if (glfwWindowShouldClose(glfw_window_ptr))
            break;

        //
    }
}
