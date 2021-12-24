#include <iostream>
#include <chrono>

#define GABE_USING_GLFW 0
#define GABE_USING_SDL2 1

#define GLM_DEPTH_ZERO_TO_ONE

#include <engine/player.hpp>
#include <engine/camera.hpp>
#include <engine/window/sdl2/graphics_window.hpp>

#include <engine/world/world.hpp>

struct Game : GraphicsWindow<Game> {
    RenderContext render_ctx;

    Player player;
    Camera camera;

    World world;

    Game()
        : render_ctx(get_window_surface(), get_window_sx(), get_window_sy()),
          world(render_ctx) {
        player.pos = {0, -10, 0};
        camera.resize(get_window_sx(), get_window_sy());
    }

    void update(double elapsed_seconds) {
        poll_events();
        player.update(elapsed_seconds);
        camera.set_pos(player.pos);
        camera.set_rot(player.rot.x, player.rot.y);
    }
    void present() {
        world.try_reload_shaders(render_ctx);

        auto vp       = camera.update();
        auto cmd_list = render_ctx.begin_frame(get_window_sx(), get_window_sy());

        world.update(cmd_list, {.viewproj_mat = vp, .cam_pos = player.pos});

        render_ctx.begin_rendering(cmd_list);
        world.draw(cmd_list);
        render_ctx.end_rendering(cmd_list);

        render_ctx.end_frame(cmd_list);
    }

    // window callbacks

    void on_key(int32_t key_id, int32_t action) {
        if (action && key_id == input::keybinds::TOGGLE_PAUSE) toggle_pause();
        if (!paused) player.on_key(key_id, action);
    }
    void on_mouse_move(double x, double y) {
        if (!paused) {
            double center_x = static_cast<double>(get_window_sx()) / 2;
            double center_y = static_cast<double>(get_window_sy()) / 2;
            auto   offset   = glm::dvec2{x - center_x, center_y - y};
            player.on_mouse_move(offset.x, offset.y);
            set_mouse_pos(center_x, center_y);
        }
    }
    void on_window_resize(int32_t size_x, int32_t size_y) {
        camera.resize(get_window_sx(), get_window_sy());
        present();
    }
};

int main() {
    Game game;

    using Clock    = std::chrono::steady_clock;
    auto prev_time = Clock::now();

    while (true) {
        auto now             = Clock::now();
        auto elapsed         = now - prev_time;
        prev_time            = now;
        auto elapsed_seconds = std::chrono::duration<double>(elapsed).count();

        game.update(elapsed_seconds);
        if (game.should_close()) break;

        game.present();
    }
}
