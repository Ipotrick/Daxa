#pragma once

#define GLM_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <numbers>

#include <daxa/types.hpp>
using namespace daxa::types;

struct Camera3D
{
    f32 fov = 98.6f, aspect = 1.0f;
    f32 near_clip = 0.01f, far_clip = 1000.0f;
    glm::mat4 proj_mat{};
    glm::mat4 vtrn_mat{};
    glm::mat4 vrot_mat{};

    void resize(i32 size_x, i32 size_y)
    {
        aspect = static_cast<f32>(size_x) / static_cast<f32>(size_y);
        proj_mat = glm::perspective(glm::radians(fov), aspect, near_clip, far_clip);
    }
    void set_pos(f32vec3 pos)
    {
        vtrn_mat = glm::translate(glm::mat4(1), glm::vec3(pos.x, pos.y, pos.z));
    }
    void set_rot(f32 x, f32 y)
    {
        vrot_mat = glm::rotate(glm::rotate(glm::mat4(1), y, {1, 0, 0}), x, {0, 1, 0});
    }
    glm::mat4 get_vp()
    {
        return proj_mat * vrot_mat * vtrn_mat;
    }
};

namespace input
{
    struct Keybinds
    {
        i32 move_pz, move_nz;
        i32 move_px, move_nx;
        i32 move_py, move_ny;
        i32 toggle_pause;
        i32 toggle_sprint;
    };

    static inline constexpr Keybinds DEFAULT_KEYBINDS{
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

struct Player3D
{
    Camera3D camera{};
    input::Keybinds keybinds = input::DEFAULT_KEYBINDS;
    f32vec3 pos{0, 0, 0}, vel{}, rot{};
    f32 speed = 30.0f, mouse_sens = 0.1f;
    f32 sprint_speed = 8.0f;
    f32 sin_rot_x = 0, cos_rot_x = 1;

    struct MoveFlags
    {
        uint8_t px : 1, py : 1, pz : 1, nx : 1, ny : 1, nz : 1, sprint : 1;
    } move{};

    void update(f32 dt)
    {
        auto delta_pos = speed * dt;
        if (move.sprint)
            delta_pos *= sprint_speed;
        if (move.px)
            pos.z += sin_rot_x * delta_pos, pos.x += cos_rot_x * delta_pos;
        if (move.nx)
            pos.z -= sin_rot_x * delta_pos, pos.x -= cos_rot_x * delta_pos;
        if (move.pz)
            pos.x -= sin_rot_x * delta_pos, pos.z += cos_rot_x * delta_pos;
        if (move.nz)
            pos.x += sin_rot_x * delta_pos, pos.z -= cos_rot_x * delta_pos;
        if (move.py)
            pos.y += delta_pos;
        if (move.ny)
            pos.y -= delta_pos;

        constexpr auto MAX_ROT = std::numbers::pi_v<f32> / 2;
        if (rot.y > MAX_ROT)
            rot.y = MAX_ROT;
        if (rot.y < -MAX_ROT)
            rot.y = -MAX_ROT;
    }
    void on_key(i32 key, i32 action)
    {
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
    void on_mouse_move(f32 delta_x, f32 delta_y)
    {
        rot.x += delta_x * mouse_sens * 0.0001f * camera.fov;
        rot.y += delta_y * mouse_sens * 0.0001f * camera.fov;
        sin_rot_x = std::sin(rot.x);
        cos_rot_x = std::cos(rot.x);
    }
};
