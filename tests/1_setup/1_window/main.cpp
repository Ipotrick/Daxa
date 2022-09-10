#include <0_common/window.hpp>
#include <thread>

struct App : AppWindow<App>
{
    App() : AppWindow<App>("Setup: Test GLFW Window") {}
    ~App() {}

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
    }

    void on_resize(u32 sx, u32 sy)
    {
        minimized = (sx == 0 || sy == 0);
        if (!minimized)
        {
            size_x = sx;
            size_y = sy;
            draw();
        }
    }

    void on_key(i32, i32) {}
    void on_mouse_move(f32, f32) {}
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
