#pragma once
#include <SDL2/SDL_keycode.h>

namespace input::keybinds {
    static constexpr auto MOVE_PZ = SDLK_w;
    static constexpr auto MOVE_NZ = SDLK_s;
    static constexpr auto MOVE_PX = SDLK_a;
    static constexpr auto MOVE_NX = SDLK_d;

    static constexpr auto MOVE_PY = SDLK_SPACE;
    static constexpr auto MOVE_NY = SDLK_LSHIFT;

    static constexpr auto TOGGLE_PAUSE = SDLK_ESCAPE;
} // namespace input::keybinds
