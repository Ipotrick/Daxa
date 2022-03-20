#pragma once

#include <Daxa.hpp>

#include "input.hpp"

#if defined(USING_SDL2)
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#elif defined(USING_GLFW)
#include <GLFW/glfw3native.h>
#endif

template <typename UserT>
struct GraphicsWindow {
  private:
    struct DaxaContext {
        DaxaContext() { daxa::initialize(); }
        ~DaxaContext() { daxa::cleanup(); }
    };

#if defined(USING_SDL2)
    template <typename UserT>
    struct WindowImpl {
        SDL_Window *sdl_window_ptr;
        int sdl_window_id;

        void create(int32_t sx, int32_t sy, const char *title) {
            SDL_SetHint(SDL_HINT_MOUSE_RELATIVE_MODE_WARP, "1");
            sdl_window_ptr = SDL_CreateWindow(
                // clang-format off
            title,
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            sx, sy,
            SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE
                // clang-format on
            );
            sdl_window_id = SDL_GetWindowID(sdl_window_ptr);
            SDL_AddEventWatch(
                [](void *userdata, SDL_Event *event) -> int {
                    auto user_ptr = (UserT *)userdata;
                    auto window_ptr = (GraphicsWindow *)userdata;
                    switch (event->type) {
                    case SDL_QUIT: window_ptr->_should_close = true; break;
                    case SDL_WINDOWEVENT_CLOSE: window_ptr->_should_close = true; break;
                    case SDL_WINDOWEVENT:
                        switch (event->window.event) {
                        case SDL_WINDOWEVENT_RESIZED:
                            window_ptr->size_x = event->window.data1;
                            window_ptr->size_y = event->window.data2;
                            user_ptr->on_window_resize(event->window.data1,
                                                       event->window.data2);
                            break;
                        }
                        break;
                    case SDL_MOUSEMOTION:
                        user_ptr->on_mouse_move(event->motion.x, event->motion.y);
                        break;
                    case SDL_KEYDOWN:
                    case SDL_KEYUP:
                        user_ptr->on_key(event->key.keysym.sym, event->key.state);
                        break;
                    }
                    return 0;
                },
                this);
        }

        auto vulkan_surface() {
            using namespace daxa;
            VkSurfaceKHR vulkan_surface;
            auto ret = SDL_Vulkan_CreateSurface(
                sdl_window_ptr, gpu::instance->getVkInstance(), &vulkan_surface);
            DAXA_ASSERT_M(ret == SDL_TRUE, "could not create window surface");
            return vulkan_surface;
        }

        void poll_events() {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
            }
        }

        void set_mouse_pos(i32 x, i32 y) { SDL_WarpMouseInWindow(sdl_window_ptr, x, y); }
    };
#elif defined(USING_GLFW)
    struct WindowImpl {
        GLFWwindow *glfw_window_ptr;

        void create(int32_t sx, int32_t sy, const char *title) {
            glfwInit();
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            glfw_window_ptr = glfwCreateWindow(sx, sy, title, nullptr, nullptr);
            glfwSetWindowUserPointer(glfw_window_ptr, this);
            glfwSetWindowSizeCallback(
                glfw_window_ptr, [](GLFWwindow *glfw_window_ptr, int x, int y) {
                    auto *userdata = glfwGetWindowUserPointer(glfw_window_ptr);
                    auto user_ptr = (UserT *)userdata;
                    auto window_ptr = (GraphicsWindow *)userdata;
                    window_ptr->size_x = x;
                    window_ptr->size_y = y;
                    user_ptr->on_window_resize(x, y);
                });
            glfwSetKeyCallback(glfw_window_ptr, [](GLFWwindow *glfw_window_ptr, int key, int scancode, int action, int mods) {
                auto *userdata = glfwGetWindowUserPointer(glfw_window_ptr);
                auto user_ptr = (UserT *)userdata;
                // auto   window_ptr = (GraphicsWindow *)userdata;
                user_ptr->on_key(key, action);
            });
            glfwSetCursorPosCallback(
                glfw_window_ptr, [](GLFWwindow *glfw_window_ptr, double x, double y) {
                    auto *userdata = glfwGetWindowUserPointer(glfw_window_ptr);
                    auto user_ptr = (UserT *)userdata;
                    // auto   window_ptr = (GraphicsWindow *)userdata;
                    user_ptr->on_mouse_move(x, y);
                });
        }

        void destroy() {
            //
        }

        auto vulkan_surface() {
            using namespace daxa;
            VkSurfaceKHR vulkan_surface;
            VkResult err = glfwCreateWindowSurface(
                gpu::instance->getVkInstance(), glfw_window_ptr, NULL, &vulkan_surface);
            DAXA_ASSERT_M(err == VK_SUCCESS, "could not create window surface");
            return vulkan_surface;
        }

        void poll_events() { glfwPollEvents(); }

        void set_mouse_pos(i32 x, i32 y) { glfwSetCursorPos(glfw_window_ptr, x, y); }
    };
#endif

    WindowImpl impl;
    DaxaContext daxa_ctx{};
    VkSurfaceKHR vulkan_surface;
    bool _should_close = false;
    int32_t size_x, size_y;

  public:
    bool paused = true;

    GraphicsWindow(int32_t sx = 1024, int32_t sy = 720,
                   std::string_view title = "title") {
        size_x = sx, size_y = sy;
        impl.create(sx, sy, title.data());
        vulkan_surface = impl.vulkan_surface();
        DAXA_ASSERT_M(vulkan_surface, "FAIL");
        toggle_pause();
    }
    ~GraphicsWindow() {
        using namespace daxa;
        vkDestroySurfaceKHR(gpu::instance->getVkInstance(), vulkan_surface, nullptr);
        impl.destroy();
    }

    void poll_events() {
        impl.poll_events();
        if (glfwWindowShouldClose(impl.glfw_window_ptr))
            _should_close = true;
    }

    bool should_close() const { return _should_close; }

    void toggle_pause() {
        paused = !paused;
        std::cout << "Setting Paused to " << paused << "\n";

        glfwSetInputMode(impl.glfw_window_ptr, GLFW_CURSOR,
                         paused ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);

        // SDL_CaptureMouse(paused ? SDL_FALSE : SDL_TRUE);
        // SDL_SetRelativeMouseMode(paused ? SDL_FALSE : SDL_TRUE);

        double center_x = static_cast<double>(size_x / 2);
        double center_y = static_cast<double>(size_y / 2);

        set_mouse_pos(center_x, center_y);
    }

    void set_mouse_pos(i32 x, i32 y) { impl.set_mouse_pos(x, y); }

    auto get_window_surface() const { return vulkan_surface; }
    auto get_window_sx() const { return size_x; }
    auto get_window_sy() const { return size_y; }
};
