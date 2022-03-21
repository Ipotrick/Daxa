#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
struct Camera {
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
        vrot_mat = glm::rotate(glm::rotate(glm::mat4(1), y, {1, 0, 0}), x, {0, 1, 0});
    }
    glm::mat4 update() { return proj_mat * vrot_mat * vtrn_mat; }
};
