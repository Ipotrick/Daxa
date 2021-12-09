#include <engine/window.hpp>
#include <engine/camera.hpp>
#include <engine/player.hpp>
#include <engine/world/world.hpp>

#include <array>
#include <vector>
#include <string>
#include <string_view>
#include <iostream>
#include <chrono>

using Clock = std::chrono::steady_clock;

struct Game {
    Window<Game> window;
    Clock::time_point prev_time;

    Player player;
    Camera camera;
    World  world;

    Game() {
        camera.resize(window.size_x, window.size_y);
        // frame = create_framebuffer(window.size_x, window.size_y);
        prev_time = Clock::now();
    }

    void on_key(int key, int scancode, int action, int mods) {
        // switch (key) {
        // case GLFW_KEY_ESCAPE:
        //     if (action == GLFW_PRESS) window.toggle_pause();
        //     break;
        // }
        if (!window.paused) player.on_key(key, action);
    }
    void on_mouse_move(double x, double y) {
        if (!window.paused) {
            double center_x = static_cast<double>(window.size_x) / 2;
            double center_y = static_cast<double>(window.size_y) / 2;
            player.on_mouse_move(x - center_x, y - center_y);
            // glfwSetCursorPos(window.ptr, center_x, center_y);
        }
    }
    void on_resize(int size_x, int size_y) {
        window.size_x = size_x, window.size_y = size_y;
        camera.resize(window.size_x, window.size_y);
        // recreate_framebuffer(window.size_x, window.size_y, frame);
        present();
    }
    void present() {
        auto now = Clock::now();
        auto delta_time = std::chrono::duration<double>(now - prev_time).count();
        prev_time = now;
        player.update(delta_time);

        // frame.bind();
        // glViewport(0, 0, window.size_x, window.size_y);
        // glClearColor(0.4, 0.5f, 1.0f, 1.0f);
        // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        camera.set_pos(player.pos);
        camera.set_rot(player.rot.x, player.rot.y);
        glm::mat4 viewproj_mat = camera.update();
        world.draw(viewproj_mat);

        // frame.unbind();
        // glViewport(0, 0, window.size_x, window.size_y);
        // glClearColor(0.0, 0.0f, 0.0f, 1.0f);
        // glClear(GL_COLOR_BUFFER_BIT);
        // quad.draw(frame);

        // glfwSwapBuffers(window.ptr);
    }
};

int main() {
    float values[3] = {0, 3, 1};
    std::cout << values[1000] << "\n";

    return 0;
    
    Game game;
    while (true) {
        // glfwPollEvents();
        // if (glfwWindowShouldClose(game.window.ptr)) break;
        game.present();
    }
}
