
#include <GLFW/glfw3.h>
#if defined(_WIN32)
#define GLFW_EXPOSE_NATIVE_WIN32
#elif defined(__linux__)
#define GLFW_EXPOSE_NATIVE_X11
#endif
#include <GLFW/glfw3native.h>

#include <daxa/daxa.hpp>
using namespace daxa::types;

template <typename App>
struct AppWindow
{
    GLFWwindow * glfw_window_ptr;
    u32 size_x = 800, size_y = 600;
    bool minimized = false;

    AppWindow()
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

    ~AppWindow()
    {
        glfwDestroyWindow(glfw_window_ptr);
        glfwTerminate();
    }

    auto get_native_handle() -> daxa::NativeWindowHandle
    {
#if defined(_WIN32)
        return glfwGetWin32Window(glfw_window_ptr);
#elif defined(__linux__)
        return glfwGetX11Window(glfw_window_ptr);
#endif
    }
};
