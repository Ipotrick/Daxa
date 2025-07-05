#include <0_common/window.hpp>
#include <thread>
#include <chrono>

struct App : AppWindow<App>
{
    App() : AppWindow<App>("Setup: Test GLFW Window") {}
    ~App() = default;

    auto update() -> bool
    {
        glfwPollEvents();
        if (glfwWindowShouldClose(glfw_window_ptr) != 0)
        {
            return true;
        }
        if (!minimized)
        {
            // draw();
        }
        else
        {
            using namespace std::literals;
            std::this_thread::sleep_for(1ms);
        }
        return false;
    }

    // void draw()
    // {
    // }

    void on_resize(u32 sx, u32 sy)
    {
        minimized = (sx == 0 || sy == 0);
        if (!minimized)
        {
            size_x = sx;
            size_y = sy;
            // draw();
        }
    }

    void on_mouse_move(f32 /*unused*/, f32 /*unused*/) {}
    void on_mouse_button(i32 /*unused*/, i32 /*unused*/) {}
    void on_key(i32 /*unused*/, i32 /*unused*/) {}
};

auto main() -> int
{
    App app = {};
    while (true)
    {
        if (app.update())
        {
            break;
        }
    }
}
