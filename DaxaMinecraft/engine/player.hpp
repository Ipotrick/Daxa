#pragma once
#include <cstdint>
#include <cmath>
#include <glm/glm.hpp>

// #define GLFW_INCLUDE_NONE
// #include <GLFW/glfw3.h>

struct Player {
    union MoveFlags {
        struct {
            uint8_t px : 1, py : 1, pz : 1, nx : 1, ny : 1, nz : 1;
        };
        uint8_t raw = 0;
    } move;
    glm::vec3 pos{0, 0, 0}, vel{0, 0, 0}, rot{0, 0, 0};
    float     sin_rot_x = 0, cos_rot_x = 1;
    float     speed = 20.0f, mouse_sens = 0.001f;
    float     fov     = 70.0f;
    float     elapsed = 0.0f;

    void update(double delta_time) {
        elapsed += delta_time;
        auto delta_pos = speed * static_cast<float>(delta_time);
        if (move.px) pos.z += sin_rot_x * delta_pos, pos.x += cos_rot_x * delta_pos;
        if (move.nx) pos.z -= sin_rot_x * delta_pos, pos.x -= cos_rot_x * delta_pos;
        if (move.pz) pos.x -= sin_rot_x * delta_pos, pos.z += cos_rot_x * delta_pos;
        if (move.nz) pos.x += sin_rot_x * delta_pos, pos.z -= cos_rot_x * delta_pos;
        if (move.py) pos.y += delta_pos;
        if (move.ny) pos.y -= delta_pos;
        // float sin_elapsed = sin(elapsed);
        // pos.y = sin_elapsed * 1;
    }
    void on_key(int key, int action) {
        switch (key) {
        // case GLFW_KEY_W: move.pz = action != GLFW_RELEASE; break;
        // case GLFW_KEY_S: move.nz = action != GLFW_RELEASE; break;
        // case GLFW_KEY_A: move.px = action != GLFW_RELEASE; break;
        // case GLFW_KEY_D: move.nx = action != GLFW_RELEASE; break;
        // case GLFW_KEY_SPACE: move.ny = action != GLFW_RELEASE; break;
        // case GLFW_KEY_LEFT_SHIFT: move.py = action != GLFW_RELEASE; break;
        }
    }
    void on_mouse_move(double delta_x, double delta_y) {
        rot.x += static_cast<float>(delta_x) * mouse_sens;
        rot.y += static_cast<float>(delta_y) * mouse_sens;
        sin_rot_x = std::sin(rot.x);
        cos_rot_x = std::cos(rot.x);
    }
};