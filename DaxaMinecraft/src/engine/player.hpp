#pragma once
#include <cstdint>
#include <cmath>
#include <glm/glm.hpp>

#include <engine/window/sdl2/input.hpp>

struct Player {
    union MoveFlags {
        struct {
            uint8_t px : 1, py : 1, pz : 1, nx : 1, ny : 1, nz : 1;
        };
        uint8_t raw = 0;
    } move;
    glm::vec3 pos{0, 0, 0}, vel{0, 0, 0}, rot{0, 0, 0};
    float sin_rot_x = 0, cos_rot_x = 1;
    float speed = 45.0f, mouse_sens = 0.001f;
    float fov = 70.0f;
    float elapsed = 0.0f;
    bool is_grounded = false;

    void update(float delta_time) {
        elapsed += delta_time;
        auto delta_pos = speed * delta_time;
        if (move.px)
            pos.z += sin_rot_x * delta_pos, pos.x += cos_rot_x * delta_pos;
        if (move.nx)
            pos.z -= sin_rot_x * delta_pos, pos.x -= cos_rot_x * delta_pos;
        if (move.pz)
            pos.x -= sin_rot_x * delta_pos, pos.z += cos_rot_x * delta_pos;
        if (move.nz)
            pos.x += sin_rot_x * delta_pos, pos.z -= cos_rot_x * delta_pos;
        if (move.py)
            pos.y -= delta_pos;
        if (move.ny)
            pos.y += delta_pos;

        constexpr auto MAX_ROT = 1.54f;
        if (rot.y > MAX_ROT)
            rot.y = MAX_ROT;
        if (rot.y < -MAX_ROT)
            rot.y = -MAX_ROT;
    }
    void on_key(int key, int action) {
        switch (key) {
        case input::keybinds::MOVE_PZ: move.pz = action != 0; break;
        case input::keybinds::MOVE_NZ: move.nz = action != 0; break;

        case input::keybinds::MOVE_PX: move.px = action != 0; break;
        case input::keybinds::MOVE_NX: move.nx = action != 0; break;

        case input::keybinds::MOVE_PY: move.py = action != 0; break;
        case input::keybinds::MOVE_NY: move.ny = action != 0; break;
        }
    }
    void on_mouse_move(double delta_x, double delta_y) {
        rot.x += static_cast<float>(delta_x) * mouse_sens;
        rot.y -= static_cast<float>(delta_y) * mouse_sens;
        sin_rot_x = std::sin(rot.x);
        cos_rot_x = std::cos(rot.x);
    }
};