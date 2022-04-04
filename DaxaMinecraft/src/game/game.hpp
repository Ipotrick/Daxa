#pragma once

#include <utils/window.hpp>
#include <game/player.hpp>
#include <game/graphics.hpp>
#include <utils/fps_printer.hpp>

#include <chrono>

static void HelpMarker(const char *desc) {
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

struct Game {
    using Clock = std::chrono::high_resolution_clock;
    Clock::time_point prev_frame_time;
    FpsPrinter<Clock> fps_printer;

    Window window;

    VkSurfaceKHR vulkan_surface = window.get_vksurface(daxa::instance->getVkInstance());
    RenderContext render_context{vulkan_surface, window.frame_dim};
    std::optional<daxa::ImGuiRenderer> imgui_renderer = std::nullopt;

    World world{render_context};
    Player3D player;

    bool paused = true;

    Game() {
        window.set_user_pointer<Game>(this);
        reset_player();

        ImGui::CreateContext();
        ImGui_ImplGlfw_InitForVulkan(window.window_ptr, true);
        imgui_renderer.emplace(render_context.device, render_context.queue, render_context.pipeline_compiler);
    }

    void reset_player() {
        player = Player3D{};
        player.pos = glm::vec3(World::DIM * Chunk::DIM) / 2.0f;
        player.pos.y = -100.0f;
        player.rot = {0.0f, -0.6f, 0.0f};
    }

    void update() {
        auto now = Clock::now();
        float dt = std::chrono::duration<float>(now - prev_frame_time).count();
        prev_frame_time = now;
        fps_printer.update(now);

        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::Begin("debug");
        ImGui::SliderFloat("Speed", &player.speed, 0.1f, 40.0f);
        HelpMarker("Speed to move (Blocks/s)");
        ImGui::SliderFloat("FOV", &player.camera.fov, 0.1f, 170.0f);
        HelpMarker("Vertical field of view (Degrees)");
        ImGui::SliderFloat("Sensitivity", &player.mouse_sens, 0.01f, 10.0f);
        HelpMarker("Mouse rotation speed (Radians/Pixels_moved/200)");
        if (ImGui::Button("Reset player"))
            reset_player();
        ImGui::End();
        ImGui::Render();

        window.update();
        player.update(dt);
        world.update(dt);
        redraw();
    }

    void redraw() {
        auto cmd_list = render_context.begin_frame(window.frame_dim);
        player.camera.resize(window.frame_dim.x, window.frame_dim.y);
        player.camera.set_pos(player.pos);
        player.camera.set_rot(player.rot.x, player.rot.y);
        auto vp_mat = player.camera.vrot_mat;
        world.draw(vp_mat, player, cmd_list, render_context.render_color_image);
        render_context.blit_to_swapchain(cmd_list);
        if (paused)
            imgui_renderer->recordCommands(ImGui::GetDrawData(), cmd_list, render_context.swapchain_image.getImageViewHandle());
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
    void on_mouse_scroll(const glm::dvec2 offset) {
        if (!paused) {
            player.camera.fov -= static_cast<float>(offset.y);
            if (player.camera.fov < 10.0f)
                player.camera.fov = 10.0f;
            if (player.camera.fov > 170.0f)
                player.camera.fov = 170.0f;
        }
    }
    void on_mouse_button(int button, int action) {
        if (!paused && action == GLFW_PRESS) {
            switch (button) {
            case GLFW_MOUSE_BUTTON_LEFT:
                // world.should_break = true;
                break;
            case GLFW_MOUSE_BUTTON_RIGHT:
                // world.should_place = true;
                break;
            default: break;
            }
        }
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
