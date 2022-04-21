#pragma once

#include <glm/glm.hpp>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <numbers>

struct Camera3D {
    float fov = 98.6f, aspect = 1.0f;
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

namespace input {
    struct Keybinds {
        i32 move_pz, move_nz;
        i32 move_px, move_nx;
        i32 move_py, move_ny;
        i32 toggle_pause;
        i32 toggle_sprint;
    };

    static constexpr Keybinds DEFAULT_KEYBINDS{
        .move_pz = GLFW_KEY_W,
        .move_nz = GLFW_KEY_S,
        .move_px = GLFW_KEY_A,
        .move_nx = GLFW_KEY_D,
        .move_py = GLFW_KEY_SPACE,
        .move_ny = GLFW_KEY_LEFT_CONTROL,
        .toggle_pause = GLFW_KEY_ESCAPE,
        .toggle_sprint = GLFW_KEY_LEFT_SHIFT,
    };
} // namespace input

struct Player3D {
    Camera3D camera;
    input::Keybinds keybinds;
    glm::vec3 pos{}, vel{}, rot{};
    float speed = 30.0f, mouse_sens = 0.1f;
    float sprint_speed = 8.0f;
    float sin_rot_x = 0, cos_rot_x = 1;

    struct MoveFlags {
        uint8_t px : 1, py : 1, pz : 1, nx : 1, ny : 1, nz : 1, sprint : 1;
    } move{};

    void update(float dt) {
        auto delta_pos = speed * dt;
        if (move.sprint)
            delta_pos *= sprint_speed;
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
        if (key == keybinds.move_pz)
            move.pz = action != 0;
        if (key == keybinds.move_nz)
            move.nz = action != 0;
        if (key == keybinds.move_px)
            move.px = action != 0;
        if (key == keybinds.move_nx)
            move.nx = action != 0;
        if (key == keybinds.move_py)
            move.py = action != 0;
        if (key == keybinds.move_ny)
            move.ny = action != 0;
        if (key == keybinds.toggle_sprint)
            move.sprint = action != 0;
    }
    void on_mouse_move(double delta_x, double delta_y) {
        rot.x += static_cast<float>(delta_x) * mouse_sens * 0.0001f * camera.fov;
        rot.y += static_cast<float>(delta_y) * mouse_sens * 0.0001f * camera.fov;
        sin_rot_x = std::sin(rot.x);
        cos_rot_x = std::cos(rot.x);
    }
};
