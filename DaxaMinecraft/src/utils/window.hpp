#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <Daxa.hpp>

struct Window {
    GLFWwindow *window_ptr;
    glm::ivec2 frame_dim{800, 800};
    VkSurfaceKHR vulkan_surface;

    Window() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window_ptr = glfwCreateWindow(frame_dim.x, frame_dim.y, "title", nullptr, nullptr);
    }

    ~Window() {
        auto vk_instance = daxa::instance->getVkInstance();
        vkDestroySurfaceKHR(vk_instance, vulkan_surface, nullptr);
        glfwDestroyWindow(window_ptr);
        glfwTerminate();
    }

    bool should_close() {
        return glfwWindowShouldClose(window_ptr);
    }
    void update() {
        glfwPollEvents();
    }
    void swap_buffers() {
        glfwSwapBuffers(window_ptr);
    }
    VkSurfaceKHR get_vksurface(VkInstance vk_instance) {
        if (!vulkan_surface)
            glfwCreateWindowSurface(vk_instance, window_ptr, nullptr, &vulkan_surface);
        return vulkan_surface;
    }

    template <typename T>
    void set_user_pointer(T *user_ptr) {
        glfwSetWindowUserPointer(window_ptr, user_ptr);
        glfwSetWindowSizeCallback(window_ptr, [](GLFWwindow *glfw_window_ptr, int size_x, int size_y) -> void {
            auto user_ptr = static_cast<T *>(glfwGetWindowUserPointer(glfw_window_ptr));
            if (user_ptr) {
                user_ptr->get_window().frame_dim = {size_x, size_y};
                user_ptr->on_resize();
            }
        });
        glfwSetCursorPosCallback(window_ptr, [](GLFWwindow *glfw_window_ptr, double mouse_x, double mouse_y) -> void {
            auto user_ptr = static_cast<T *>(glfwGetWindowUserPointer(glfw_window_ptr));
            if (user_ptr) {
                user_ptr->on_mouse_move(glm::dvec2{mouse_x, mouse_y});
            }
        });
        glfwSetScrollCallback(window_ptr, [](GLFWwindow *glfw_window_ptr, double offset_x, double offset_y) -> void {
            auto user_ptr = static_cast<T *>(glfwGetWindowUserPointer(glfw_window_ptr));
            if (user_ptr) {
                user_ptr->on_mouse_scroll(glm::dvec2{offset_x, offset_y});
            }
        });
        glfwSetMouseButtonCallback(window_ptr, [](GLFWwindow *glfw_window_ptr, int button, int action, int) -> void {
            auto user_ptr = static_cast<T *>(glfwGetWindowUserPointer(glfw_window_ptr));
            if (user_ptr) {
                user_ptr->on_mouse_button(button, action);
            }
        });
        glfwSetKeyCallback(window_ptr, [](GLFWwindow *glfw_window_ptr, int key, int, int action, int) -> void {
            auto user_ptr = static_cast<T *>(glfwGetWindowUserPointer(glfw_window_ptr));
            if (user_ptr) {
                user_ptr->on_key(key, action);
            }
        });
    }

    inline void set_mouse_capture(bool should_capture) {
        glfwSetCursorPos(window_ptr, static_cast<double>(frame_dim.x / 2), static_cast<double>(frame_dim.y / 2));
        glfwSetInputMode(window_ptr, GLFW_CURSOR, should_capture ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
        glfwSetInputMode(window_ptr, GLFW_RAW_MOUSE_MOTION, should_capture);
    }
    inline void set_mouse_pos(const glm::vec2 p) {
        glfwSetCursorPos(window_ptr, static_cast<f64>(p.x), static_cast<f64>(p.y));
    }
};
