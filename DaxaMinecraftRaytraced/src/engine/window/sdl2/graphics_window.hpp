#pragma once

#include <Daxa.hpp>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

template <typename UserT> struct GraphicsWindow {
  private:
    struct DaxaContext {
        DaxaContext() { daxa::initialize(); }
        ~DaxaContext() { daxa::cleanup(); }
    };

    DaxaContext  daxa_ctx{};
    SDL_Window * sdl_window_ptr;
    int          sdl_window_id;
    VkSurfaceKHR vulkan_surface;
    bool         _should_close = false;
    int32_t      size_x, size_y;

  public:
    bool paused = true;

    GraphicsWindow(int32_t sx = 1024, int32_t sy = 720,
                   std::string_view title = "title") {
        size_x = sx, size_y = sy;
        SDL_SetHint(SDL_HINT_MOUSE_RELATIVE_MODE_WARP, "1");
        sdl_window_ptr = SDL_CreateWindow(
            // clang-format off
            title.data(),
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            size_x, size_y,
            SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE
            // clang-format on
        );
        sdl_window_id = SDL_GetWindowID(sdl_window_ptr);

        using namespace daxa;
        auto ret = SDL_Vulkan_CreateSurface(
            sdl_window_ptr, gpu::instance->getVkInstance(), &vulkan_surface);
        DAXA_ASSERT_M(ret == SDL_TRUE, "could not create window surface");

        SDL_AddEventWatch(
            [](void * userdata, SDL_Event * event) -> int {
                auto user_ptr   = (UserT *)userdata;
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

        toggle_pause();
    }
    ~GraphicsWindow() {
        using namespace daxa;
        vkDestroySurfaceKHR(gpu::instance->getVkInstance(), vulkan_surface, nullptr);
        SDL_DestroyWindow(sdl_window_ptr);
    }

    void poll_events() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {}
    }

    bool should_close() const { return _should_close; }

    void toggle_pause() {
        paused = !paused;
        std::cout << "Setting Paused to " << paused << "\n";

        SDL_CaptureMouse(paused ? SDL_FALSE : SDL_TRUE);
        SDL_SetRelativeMouseMode(paused ? SDL_FALSE : SDL_TRUE);

        double center_x = static_cast<double>(size_x) / 2;
        double center_y = static_cast<double>(size_y) / 2;

        set_mouse_pos(center_x, center_y);
    }

    void set_mouse_pos(i32 x, i32 y) {
        SDL_WarpMouseInWindow(sdl_window_ptr, x, y);
    }

    auto get_window_surface() const { return vulkan_surface; }
    auto get_window_sx() const { return size_x; }
    auto get_window_sy() const { return size_y; }
};
