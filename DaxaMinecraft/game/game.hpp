#pragma once

#include "window.hpp"
#include "player.hpp"
#include "graphics.hpp"
#include "../../../fps_printer.hpp"

#include <chrono>

struct Game {
    using Clock = std::chrono::high_resolution_clock;
    Clock::time_point prev_frame_time;
    FpsPrinter<Clock> fps_printer;

    Window window;

    VkSurfaceKHR vulkan_surface = window.get_vksurface(daxa::instance->getVkInstance());
    RenderContext render_context{vulkan_surface, window.frame_dim};

    World world{render_context};
    Player3D player;
    Camera3D camera;

    bool paused = true;

    Game() {
        window.set_user_pointer<Game>(this);
        player.pos = glm::vec3(World::DIM) * 64.0f / 2.0f;
        player.pos.y = 0;
    }

    void update() {
        auto now = Clock::now();
        float dt = std::chrono::duration<float>(now - prev_frame_time).count();
        prev_frame_time = now;
        fps_printer.update(now);
        window.update();
        player.update(dt);
        world.update(dt);
        redraw();
    }

    void redraw() {
        auto cmd_list = render_context.begin_frame(window.frame_dim);
        camera.resize(window.frame_dim.x, window.frame_dim.y);
        camera.set_pos(player.pos);
        camera.set_rot(player.rot.x, player.rot.y);
        auto vp_mat = camera.vrot_mat;
        world.draw(vp_mat, player, cmd_list, render_context.render_color_image);
        render_context.begin_rendering(cmd_list);
        render_context.end_rendering(cmd_list);
        render_context.blit_to_swapchain(cmd_list);
        render_context.end_frame(cmd_list);
    }

    Window &get_window() {
        return window;
    }

    void on_mouse_move(const glm::dvec2 m) {
        if (!paused) {
            double center_x = static_cast<double>(window.frame_dim.x / 2);
            double center_y = static_cast<double>(window.frame_dim.y / 2);
            auto offset = glm::dvec2{m.x - center_x, center_y - m.y};
            player.on_mouse_move(offset.x, offset.y);
            window.set_mouse_pos(glm::vec2(center_x, center_y));
        }
    }
    void on_mouse_scroll(const glm::dvec2) {
    }
    void on_mouse_button(int, int) {
    }
    void on_key(int key, int action) {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            toggle_pause();
        if (!paused) {
            player.on_key(key, action);
        }
    }
    void on_resize() {
        redraw();
    }

    void toggle_pause() {
        window.set_mouse_capture(paused);
        paused = !paused;
    }
};
