#pragma once
// #include <glad/glad.h>
// #define GLFW_INCLUDE_NONE
// #include <GLFW/glfw3.h>

template <typename GameT> struct Window {
    // GLFWwindow * ptr;
    int          size_x = 1024, size_y = 720;
    bool         paused = true;

    Window() {
        // glfwInit();
        // glfwWindowHint(GLFW_SAMPLES, 4);
        // ptr = glfwCreateWindow(size_x, size_y, "title", nullptr, nullptr);
        // glfwSetWindowUserPointer(ptr, this);
        // glfwSetKeyCallback(ptr, [](GLFWwindow * w, int key, int sc, int action, int mods) {
        //     GameT & game = *static_cast<GameT *>(glfwGetWindowUserPointer(w));
        //     game.on_key(key, sc, action, mods);
        // });
        // glfwSetCursorPosCallback(ptr, [](GLFWwindow * w, double x, double y) {
        //     GameT & game = *static_cast<GameT *>(glfwGetWindowUserPointer(w));
        //     game.on_mouse_move(x, y);
        // });
        // glfwSetWindowSizeCallback(ptr, [](GLFWwindow * w, int size_x, int size_y) {
        //     GameT & game = *static_cast<GameT *>(glfwGetWindowUserPointer(w));
        //     game.on_resize(size_x, size_y);
        // });
        // init_gl();
        toggle_pause();
    }
    void toggle_pause() {
        // glfwSetInputMode(ptr, GLFW_RAW_MOUSE_MOTION, paused);
        // glfwSetInputMode(ptr, GLFW_CURSOR, paused ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
        // glfwSetCursorPos(ptr, center_x, center_y);
        double center_x = static_cast<double>(size_x) / 2;
        double center_y = static_cast<double>(size_y) / 2;
        paused = !paused;
    }

    void init_gl() {
        // glfwMakeContextCurrent(ptr);
        // gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));
    }
};
