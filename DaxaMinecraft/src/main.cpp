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

    ~Game() {
        // as the world also contains daxa::gpu things, we must wait before we cann
        // destroy them
        render_ctx.queue->waitIdle();
        render_ctx.queue->checkForFinishedSubmits();
    }

    void update(double elapsed_seconds) {
        poll_events();

        if (world.chunks_invalidated)
            world.generate_chunks(render_ctx);

        auto ppos = player.pos;
        player.update(elapsed_seconds);
        auto pos_diff = player.pos - ppos;

        auto *containing_chunk = world.get_containing_chunk(player.pos);
        if (containing_chunk) {
            auto *containing_block = containing_chunk->get_containing_block(player.pos);
            if (containing_block) {
                if (!containing_block->is_not_drawn()) {
                    player.pos -= pos_diff;
                }
            } else {
                // std::cout << "No bloqe player(" << player.pos.x << ", " << player.pos.y << ", " << player.pos.z << "), chunk(" << containing_chunk->pos.x << ", " << containing_chunk->pos.y << ", " << containing_chunk->pos.z << ")\n";
            }
        } else {
            // std::cout << "No chunk ";
        }

        camera.set_pos(player.pos);
        camera.set_rot(player.rot.x, player.rot.y);
    }
    void present() {
        world.try_reload_shaders(render_ctx);

        auto vp = camera.update();
        auto cmd_list = render_ctx.begin_frame(get_window_sx(), get_window_sy());

        world.update(cmd_list, vp);

        render_ctx.begin_rendering(cmd_list);
        world.draw(cmd_list);
        render_ctx.end_rendering(cmd_list);

        render_ctx.end_frame(cmd_list);
    }

    // window callbacks

    void on_key(int32_t key_id, int32_t action) {
        if (action && key_id == input::keybinds::TOGGLE_PAUSE)
            toggle_pause();
        if (!paused)
            player.on_key(key_id, action);
    }
    void on_mouse_move(double x, double y) {
        if (!paused) {
            double center_x = static_cast<double>(get_window_sx() / 2);
            double center_y = static_cast<double>(get_window_sy() / 2);
            auto offset = glm::dvec2{x - center_x, center_y - y};
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

    using Clock = std::chrono::steady_clock;
    auto prev_time = Clock::now();

    while (true) {
        auto now = Clock::now();
        auto elapsed = now - prev_time;
        prev_time = now;
        auto elapsed_seconds = std::chrono::duration<double>(elapsed).count();

        game.update(elapsed_seconds);
        if (game.should_close())
            break;

        game.present();
    }
}
