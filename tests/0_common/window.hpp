#include <daxa/daxa.hpp>
using namespace daxa::types;

#include <GLFW/glfw3.h>
#if defined(_WIN32)
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#define GLFW_EXPOSE_NATIVE_WIN32
#elif defined(__linux__)
#define GLFW_EXPOSE_NATIVE_X11
#endif
#include <GLFW/glfw3native.h>

template <typename App>
struct AppWindow
{
    GLFWwindow * glfw_window_ptr;
    u32 size_x, size_y;
    bool minimized = false;

    AppWindow(char const * window_name, u32 sx = 800, u32 sy = 600) : size_x{sx}, size_y{sy}
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfw_window_ptr = glfwCreateWindow(static_cast<i32>(size_x), static_cast<i32>(size_y), window_name, nullptr, nullptr);
        glfwSetWindowUserPointer(glfw_window_ptr, this);
        glfwSetCursorPosCallback(
            glfw_window_ptr,
            [](GLFWwindow * window_ptr, f64 x, f64 y)
            {
                auto & app = *reinterpret_cast<App *>(glfwGetWindowUserPointer(window_ptr));
                app.on_mouse_move(static_cast<f32>(x), static_cast<f32>(y));
            });
        glfwSetMouseButtonCallback(
            glfw_window_ptr,
            [](GLFWwindow * window_ptr, i32 button, i32 action, i32)
            {
                auto & app = *reinterpret_cast<App *>(glfwGetWindowUserPointer(window_ptr));
                app.on_mouse_button(button, action);
            });
        glfwSetKeyCallback(
            glfw_window_ptr,
            [](GLFWwindow * window_ptr, i32 key, i32, i32 action, i32)
            {
                auto & app = *reinterpret_cast<App *>(glfwGetWindowUserPointer(window_ptr));
                app.on_key(key, action);
            });
        glfwSetFramebufferSizeCallback(
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

    inline void set_mouse_pos(f32 x, f32 y)
    {
        glfwSetCursorPos(glfw_window_ptr, static_cast<f64>(x), static_cast<f64>(y));
    }

    inline void set_mouse_capture(bool should_capture)
    {
        glfwSetCursorPos(glfw_window_ptr, static_cast<f64>(size_x / 2), static_cast<f64>(size_y / 2));
        glfwSetInputMode(glfw_window_ptr, GLFW_CURSOR, should_capture ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
        glfwSetInputMode(glfw_window_ptr, GLFW_RAW_MOUSE_MOTION, should_capture);
    }
};
