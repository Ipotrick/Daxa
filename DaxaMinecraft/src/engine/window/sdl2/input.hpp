#pragma once

#define USING_GLFW
// #define USING_SDL2

#if defined(USING_SDL2)
#include <SDL2/SDL_keycode.h>
#elif defined(USING_GLFW)
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#endif

namespace input::keybinds {
#if defined(USING_SDL2)
    static constexpr auto MOVE_PZ = SDLK_w;
    static constexpr auto MOVE_NZ = SDLK_s;
    static constexpr auto MOVE_PX = SDLK_a;
    static constexpr auto MOVE_NX = SDLK_d;

    static constexpr auto MOVE_PY = SDLK_SPACE;
    static constexpr auto MOVE_NY = SDLK_LSHIFT;

    static constexpr auto TOGGLE_PAUSE = SDLK_ESCAPE;
#elif defined(USING_GLFW)
    static constexpr auto MOVE_PZ = GLFW_KEY_W;
    static constexpr auto MOVE_NZ = GLFW_KEY_S;
    static constexpr auto MOVE_PX = GLFW_KEY_A;
    static constexpr auto MOVE_NX = GLFW_KEY_D;

    static constexpr auto MOVE_PY = GLFW_KEY_SPACE;
    static constexpr auto MOVE_NY = GLFW_KEY_LEFT_SHIFT;

    static constexpr auto TOGGLE_PAUSE = GLFW_KEY_ESCAPE;
#endif
} // namespace input::keybinds
