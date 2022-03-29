#pragma once

#include "math.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <numbers>

struct Camera3D {
    float fov = 70.0f, aspect = 1.0f;
    float near = 0.01f, far = 1000.0f;
    glm::mat4 proj_mat{};
    glm::mat4 vtrn_mat{};
    glm::mat4 vrot_mat{};

    void resize(int size_x, int size_y) {
        aspect = static_cast<float>(size_x) / static_cast<float>(size_y);
        proj_mat = glm::perspective(glm::radians(fov), aspect, near, far);
    }
    void set_pos(glm::vec3 pos) { vtrn_mat = glm::translate(glm::mat4(1), pos); }
    void set_rot(float x, float y) {
        vrot_mat = glm::rotate(glm::rotate(glm::mat4(1), x, {0, 1, 0}), y, {1, 0, 0});
    }
    glm::mat4 get_vp() { return proj_mat * vrot_mat * vtrn_mat; }
};

namespace input::keybinds {
    static constexpr auto MOVE_PZ = GLFW_KEY_W;
    static constexpr auto MOVE_NZ = GLFW_KEY_S;
    static constexpr auto MOVE_PX = GLFW_KEY_A;
    static constexpr auto MOVE_NX = GLFW_KEY_D;
    static constexpr auto MOVE_PY = GLFW_KEY_SPACE;
    static constexpr auto MOVE_NY = GLFW_KEY_LEFT_SHIFT;
    static constexpr auto TOGGLE_PAUSE = GLFW_KEY_ESCAPE;
} // namespace input::keybinds

struct Player3D {
    glm::vec3 pos{}, vel{}, rot{};
    float speed = 128.0f, mouse_sens = 0.005f;
    float sin_rot_x = 0, cos_rot_x = 1;

    union MoveFlags {
        struct {
            uint8_t px : 1, py : 1, pz : 1, nx : 1, ny : 1, nz : 1;
        };
        uint8_t raw = 0;
    } move;

    void update(float dt) {
        auto delta_pos = speed * dt;
        if (move.px)
            pos.z += sin_rot_x * delta_pos, pos.x -= cos_rot_x * delta_pos;
        if (move.nx)
            pos.z -= sin_rot_x * delta_pos, pos.x += cos_rot_x * delta_pos;
        if (move.pz)
            pos.x += sin_rot_x * delta_pos, pos.z += cos_rot_x * delta_pos;
        if (move.nz)
            pos.x -= sin_rot_x * delta_pos, pos.z -= cos_rot_x * delta_pos;
        if (move.py)
            pos.y -= delta_pos;
        if (move.ny)
            pos.y += delta_pos;

        constexpr auto MAX_ROT = std::numbers::pi_v<float> / 2;
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
        rot.y += static_cast<float>(delta_y) * mouse_sens;
        sin_rot_x = std::sin(rot.x);
        cos_rot_x = std::cos(rot.x);
    }
};
