#include <0_common/window.hpp>
#include <0_common/player.hpp>
#include <0_common/noise.hpp>
#include <thread>
#include <iostream>

#include <daxa/utils/pipeline_manager.hpp>

#include <daxa/utils/task_list.hpp>

#include <daxa/utils/imgui.hpp>
#include <imgui_impl_glfw.h>

#include <daxa/utils/math_operators.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <glm/gtc/type_ptr.hpp>

#define APPNAME "Daxa Sample: FSR2"
#define APPNAME_PREFIX(x) ("[" APPNAME "] " x)

#include <daxa/utils/fsr2.hpp>
using UpscaleContext = daxa::Fsr2Context;

struct RasterInput
{
    f32mat4x4 view_mat;
    f32mat4x4 prev_view_mat;
    f32vec2 jitter;
    daxa::ImageId texture_array_id;
    daxa::SamplerId sampler_id;
};

struct RasterPush
{
    f32vec3 chunk_pos = {};
    u32 mode = {};
    daxa::BufferId input_buffer_id = {};
    daxa::BufferId face_buffer_id = {};
};

#include <0_common/voxels.hpp>

using namespace daxa::types;
using namespace daxa::math_operators;
using Clock = std::chrono::high_resolution_clock;

struct App : AppWindow<App>
{
    daxa::Context daxa_ctx = daxa::create_context({
        .enable_validation = false,
    });
    daxa::Device device = daxa_ctx.create_device({});

    daxa::Swapchain swapchain = device.create_swapchain({
        .native_window = get_native_handle(),
        .native_window_platform = get_native_platform(),
        .present_mode = daxa::PresentMode::DO_NOT_WAIT_FOR_VBLANK,
        .image_usage = daxa::ImageUsageFlagBits::TRANSFER_DST,
        .debug_name = APPNAME_PREFIX("swapchain"),
    });

    daxa::PipelineManager pipeline_manager = daxa::PipelineManager({
        .device = device,
        .shader_compile_options = {
            .root_paths = {
                "tests/0_common/shaders",
                "tests/3_samples/7_FSR2/shaders",
                "include",
            },
        },
        .debug_name = APPNAME_PREFIX("pipeline_manager"),
    });

    // clang-format off
    daxa::RasterPipeline raster_pipeline = pipeline_manager.add_raster_pipeline({
        .vertex_shader_info = {.source = daxa::ShaderFile{"draw.hlsl"}, .compile_options = {.entry_point = "vs_main"}},
        .fragment_shader_info = {.source = daxa::ShaderFile{"draw.hlsl"}, .compile_options = {.entry_point = "fs_main"}},
        .color_attachments = {
            {.format = daxa::Format::R16G16B16A16_SFLOAT, .blend = {.blend_enable = 1u, .src_color_blend_factor = daxa::BlendFactor::SRC_ALPHA, .dst_color_blend_factor = daxa::BlendFactor::ONE_MINUS_SRC_ALPHA}},
            {.format = daxa::Format::R16G16_SFLOAT, .blend = {.blend_enable = 1u, .src_color_blend_factor = daxa::BlendFactor::SRC_ALPHA, .dst_color_blend_factor = daxa::BlendFactor::ONE_MINUS_SRC_ALPHA}},
        },
        .depth_test = {
            .depth_attachment_format = daxa::Format::D32_SFLOAT,
            .enable_depth_test = true,
            .enable_depth_write = true,
        },
        .raster = {
            .face_culling = daxa::FaceCullFlagBits::BACK_BIT,
        },
        .push_constant_size = sizeof(RasterPush) * 2,
        .debug_name = APPNAME_PREFIX("raster_pipeline"),
    }).value();
    // clang-format on

    daxa::ImGuiRenderer imgui_renderer = create_imgui_renderer();
    auto create_imgui_renderer() -> daxa::ImGuiRenderer
    {
        ImGui::CreateContext();
        ImGui_ImplGlfw_InitForVulkan(glfw_window_ptr, true);
        return daxa::ImGuiRenderer({
            .device = device,
            .format = swapchain.get_format(),
        });
    }

    u64 cpu_framecount = 0;

    Clock::time_point start = Clock::now(), prev_time = start;
    f32 elapsed_s = 1.0f;

    RenderableVoxelWorld renderable_world{device};
    Player3D player = {
        .rot = {2.0f, 0.0f, 0.0f},
    };
    bool paused = true;

    RasterInput raster_input{};
    daxa::BufferId raster_input_buffer = device.create_buffer({
        .size = sizeof(RasterInput),
        .debug_name = APPNAME_PREFIX("raster_input_buffer"),
    });
    daxa::BufferId staging_raster_input_buffer = device.create_buffer({
        .memory_flags = daxa::MemoryFlagBits::HOST_ACCESS_RANDOM,
        .size = sizeof(RasterInput),
        .debug_name = APPNAME_PREFIX("staging_raster_input_buffer"),
    });
    daxa::TaskBufferId task_raster_input_buffer;
    daxa::TaskBufferId task_staging_raster_input_buffer;

    UpscaleContext upscale_context = UpscaleContext{{.device = device}};
    f32 render_scl = 1.0f;
    daxa::ImageId swapchain_image{};
    daxa::ImageId color_image{}, display_image{}, motion_vectors_image{}, depth_image{};
    u32 render_size_x{}, render_size_y{};
    f32vec2 jitter = {0.0f, 0.0f};
    daxa::TaskImageId task_swapchain_image;
    daxa::TaskImageId task_color_image, task_display_image, task_motion_vectors_image, task_depth_image;
    daxa::CommandSubmitInfo submit_info = {};
    daxa::TaskList loop_task_list = record_loop_task_list();
    bool fsr_enabled = false;

    void create_render_images()
    {
        render_size_x = std::max<u32>(1, static_cast<u32>(static_cast<f32>(size_x) * render_scl));
        render_size_y = std::max<u32>(1, static_cast<u32>(static_cast<f32>(size_y) * render_scl));

        color_image = device.create_image({
            .format = daxa::Format::R16G16B16A16_SFLOAT,
            .aspect = daxa::ImageAspectFlagBits::COLOR,
            .size = {render_size_x, render_size_y, 1},
            .usage = daxa::ImageUsageFlagBits::COLOR_ATTACHMENT | daxa::ImageUsageFlagBits::SHADER_READ_ONLY | daxa::ImageUsageFlagBits::SHADER_READ_WRITE | daxa::ImageUsageFlagBits::TRANSFER_SRC,
            .debug_name = APPNAME_PREFIX("color_image"),
        });
        display_image = device.create_image({
            .format = daxa::Format::R16G16B16A16_SFLOAT,
            .aspect = daxa::ImageAspectFlagBits::COLOR,
            .size = {size_x, size_y, 1},
            .usage = daxa::ImageUsageFlagBits::COLOR_ATTACHMENT | daxa::ImageUsageFlagBits::SHADER_READ_ONLY | daxa::ImageUsageFlagBits::SHADER_READ_WRITE | daxa::ImageUsageFlagBits::TRANSFER_SRC | daxa::ImageUsageFlagBits::TRANSFER_DST,
            .debug_name = APPNAME_PREFIX("display_image"),
        });
        motion_vectors_image = device.create_image({
            .format = daxa::Format::R16G16_SFLOAT,
            .aspect = daxa::ImageAspectFlagBits::COLOR,
            .size = {render_size_x, render_size_y, 1},
            .usage = daxa::ImageUsageFlagBits::COLOR_ATTACHMENT | daxa::ImageUsageFlagBits::SHADER_READ_ONLY | daxa::ImageUsageFlagBits::SHADER_READ_WRITE,
            .debug_name = APPNAME_PREFIX("motion_vectors_image"),
        });
        depth_image = device.create_image({
            .format = daxa::Format::D32_SFLOAT,
            .aspect = daxa::ImageAspectFlagBits::DEPTH,
            .size = {render_size_x, render_size_y, 1},
            .usage = daxa::ImageUsageFlagBits::DEPTH_STENCIL_ATTACHMENT | daxa::ImageUsageFlagBits::SHADER_READ_ONLY,
            .debug_name = APPNAME_PREFIX("depth_image"),
        });

        upscale_context.resize({
            .render_size_x = render_size_x,
            .render_size_y = render_size_y,
            .display_size_x = size_x,
            .display_size_y = size_y,
        });
    }
    void destroy_render_images()
    {
        device.destroy_image(color_image);
        device.destroy_image(display_image);
        device.destroy_image(motion_vectors_image);
        device.destroy_image(depth_image);
    }

    App() : AppWindow<App>(APPNAME)
    {
        create_render_images();
    }

    ~App()
    {
        device.wait_idle();
        device.collect_garbage();
        ImGui_ImplGlfw_Shutdown();
        destroy_render_images();
        device.destroy_buffer(raster_input_buffer);
        device.destroy_buffer(staging_raster_input_buffer);
    }

    auto update() -> bool
    {
        glfwPollEvents();
        if (glfwWindowShouldClose(glfw_window_ptr) != 0)
        {
            return true;
        }

        if (!minimized)
        {
            draw();
        }
        else
        {
            using namespace std::literals;
            std::this_thread::sleep_for(1ms);
        }

        return false;
    }

    void draw()
    {
        auto now = Clock::now();
        elapsed_s = std::chrono::duration<f32>(now - prev_time).count();
        prev_time = now;

        ui_update();

        player.camera.resize(static_cast<i32>(size_x), static_cast<i32>(size_y));
        player.camera.set_pos(player.pos);
        player.camera.set_rot(player.rot.x, player.rot.y);
        player.update(elapsed_s);

        auto reloaded_result = pipeline_manager.reload_all();
        if (reloaded_result.is_err())
        {
            std::cout << reloaded_result.to_string() << std::endl;
        }

        swapchain_image = swapchain.acquire_next_image();
        if (swapchain_image.is_empty())
        {
            return;
        }
        loop_task_list.execute();
    }

    void on_mouse_move(f32 x, f32 y)
    {
        if (!paused)
        {
            f32 const center_x = static_cast<f32>(size_x / 2);
            f32 const center_y = static_cast<f32>(size_y / 2);
            auto offset = f32vec2{x - center_x, center_y - y};
            player.on_mouse_move(offset.x, offset.y);
            set_mouse_pos(center_x, center_y);
        }
    }

    void on_mouse_button(i32 /*unused*/, i32 /*unused*/) {}

    void on_key(i32 key, i32 action)
    {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        {
            toggle_pause();
        }

        if (!paused)
        {
            player.on_key(key, action);
        }
    }

    void on_resize(u32 sx, u32 sy)
    {
        minimized = (sx == 0 || sy == 0);
        if (!minimized)
        {
            swapchain.resize();
            size_x = swapchain.get_surface_extent().x;
            size_y = swapchain.get_surface_extent().y;
            destroy_render_images();
            create_render_images();
            draw();
        }
    }

    void toggle_pause()
    {
        set_mouse_capture(paused);
        paused = !paused;
    }

    void ui_update()
    {
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::Begin("Debug");
        f32 new_scl = render_scl;
        ImGui::SliderFloat("Render Scl", &new_scl, 1.0f / static_cast<f32>(std::min(size_x, size_y)), 1.0f);
        if (new_scl != render_scl)
        {
            render_scl = new_scl;
            destroy_render_images();
            create_render_images();
        }
        if (ImGui::Button("Clear Console"))
        {
            [[maybe_unused]] i32 const system_ret = system("CLS");
        }
        ImGui::Checkbox("Enable FSR", &fsr_enabled);
        ImGui::End();
        ImGui::Render();
    }

    auto record_loop_task_list() -> daxa::TaskList
    {
        daxa::TaskList new_task_list = daxa::TaskList({
            .device = device,
            .swapchain = swapchain,
            .debug_name = APPNAME_PREFIX("task_list"),
        });
        task_swapchain_image = new_task_list.create_task_image({.swapchain_image = true});
        task_color_image = new_task_list.create_task_image({});
        task_display_image = new_task_list.create_task_image({});
        task_motion_vectors_image = new_task_list.create_task_image({});
        task_depth_image = new_task_list.create_task_image({});

        new_task_list.add_runtime_image(task_swapchain_image, swapchain_image);
        new_task_list.add_runtime_image(task_color_image, color_image);
        new_task_list.add_runtime_image(task_display_image, display_image);
        new_task_list.add_runtime_image(task_motion_vectors_image, motion_vectors_image);
        new_task_list.add_runtime_image(task_depth_image, depth_image);

        task_raster_input_buffer = new_task_list.create_task_buffer({});
        task_staging_raster_input_buffer = new_task_list.create_task_buffer({});

        new_task_list.add_runtime_buffer(task_raster_input_buffer, raster_input_buffer);
        new_task_list.add_runtime_buffer(task_staging_raster_input_buffer, staging_raster_input_buffer);

        new_task_list.add_task({
            .used_buffers = {
                {task_staging_raster_input_buffer, daxa::TaskBufferAccess::HOST_TRANSFER_WRITE},
            },
            .task = [this](daxa::TaskRuntime /* runtime */)
            {
                this->raster_input.prev_view_mat = this->raster_input.view_mat;

                auto prev_jitter = jitter;
                jitter = upscale_context.get_jitter(cpu_framecount);
                auto jitter_vec = f32vec2{
                    jitter.x * 2.0f / static_cast<f32>(render_size_x),
                    jitter.y * 2.0f / static_cast<f32>(render_size_y),
                };
                auto view_mat = player.camera.get_vp();
                view_mat = glm::translate(glm::identity<glm::mat4>(), glm::vec3(jitter_vec.x, jitter_vec.y, 0.0f)) * view_mat;
                this->raster_input.view_mat = mat_from_span<f32, 4, 4>(std::span<f32, 4 * 4>{glm::value_ptr(view_mat), 4 * 4});
                this->raster_input.jitter = (jitter - prev_jitter) * f32vec2{2.0f / static_cast<f32>(render_size_x), 2.0f / static_cast<f32>(render_size_y)};
                this->raster_input.texture_array_id = renderable_world.atlas_texture_array;
                this->raster_input.sampler_id = renderable_world.atlas_sampler;

                auto * buffer_ptr = device.get_host_address_as<RasterInput>(staging_raster_input_buffer);
                *buffer_ptr = this->raster_input;
            },
            .debug_name = APPNAME_PREFIX("Input MemMap"),
        });
        new_task_list.add_task({
            .used_buffers = {
                {task_raster_input_buffer, daxa::TaskBufferAccess::TRANSFER_WRITE},
                {task_staging_raster_input_buffer, daxa::TaskBufferAccess::TRANSFER_READ},
            },
            .task = [this](daxa::TaskRuntime runtime)
            {
                auto cmd_list = runtime.get_command_list();
                cmd_list.copy_buffer_to_buffer({
                    .src_buffer = staging_raster_input_buffer,
                    .dst_buffer = raster_input_buffer,
                    .size = sizeof(RasterInput),
                });
            },
            .debug_name = APPNAME_PREFIX("Input Transfer"),
        });

        new_task_list.add_task({
            .used_buffers = {
                {task_raster_input_buffer, daxa::TaskBufferAccess::VERTEX_SHADER_READ_ONLY},
            },
            .used_images = {
                {task_color_image, daxa::TaskImageAccess::COLOR_ATTACHMENT, daxa::ImageMipArraySlice{}},
                {task_motion_vectors_image, daxa::TaskImageAccess::COLOR_ATTACHMENT, daxa::ImageMipArraySlice{}},
                {task_depth_image, daxa::TaskImageAccess::DEPTH_ATTACHMENT, daxa::ImageMipArraySlice{}},
            },
            .task = [this](daxa::TaskRuntime runtime)
            {
                auto cmd_list = runtime.get_command_list();
                cmd_list.begin_renderpass({
                    .color_attachments = {
                        {
                            .image_view = color_image.default_view(),
                            .load_op = daxa::AttachmentLoadOp::CLEAR,
                            .clear_value = std::array<f32, 4>{0.2f, 0.4f, 1.0f, 1.0f},
                        },
                        {
                            .image_view = motion_vectors_image.default_view(),
                            .load_op = daxa::AttachmentLoadOp::CLEAR,
                            .clear_value = std::array<f32, 4>{0.0f, 0.0f, 0.0f, 0.0f},
                        },
                    },
                    .depth_attachment = {{
                        .image_view = depth_image.default_view(),
                        .load_op = daxa::AttachmentLoadOp::CLEAR,
                        .clear_value = daxa::DepthValue{1.0f, 0},
                    }},
                    .render_area = {.x = 0, .y = 0, .width = render_size_x, .height = render_size_y},
                });
                cmd_list.set_pipeline(raster_pipeline);
                auto push = RasterPush{.input_buffer_id = raster_input_buffer};
                renderable_world.draw(cmd_list, push);
                cmd_list.end_renderpass();
            },
            .debug_name = APPNAME_PREFIX("Draw Task"),
        });

        new_task_list.add_task({
            .used_images = {
                {task_color_image, daxa::TaskImageAccess::TRANSFER_READ, daxa::ImageMipArraySlice{}},
                {task_display_image, daxa::TaskImageAccess::TRANSFER_WRITE, daxa::ImageMipArraySlice{}},
            },
            .task = [this](daxa::TaskRuntime runtime)
            {
                if (!fsr_enabled)
                {
                    auto cmd_list = runtime.get_command_list();
                    cmd_list.blit_image_to_image({
                        .src_image = color_image,
                        .src_image_layout = daxa::ImageLayout::TRANSFER_SRC_OPTIMAL,
                        .dst_image = display_image,
                        .dst_image_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
                        .src_slice = {.image_aspect = daxa::ImageAspectFlagBits::COLOR},
                        .src_offsets = {{{0, 0, 0}, {static_cast<i32>(render_size_x), static_cast<i32>(render_size_y), 1}}},
                        .dst_slice = {.image_aspect = daxa::ImageAspectFlagBits::COLOR},
                        .dst_offsets = {{{0, 0, 0}, {static_cast<i32>(size_x), static_cast<i32>(size_y), 1}}},
                    });
                }
            },
            .debug_name = APPNAME_PREFIX("Blit Task (render to display)"),
        });

        new_task_list.add_task({
            .used_images = {
                {task_color_image, daxa::TaskImageAccess::SHADER_READ_ONLY, daxa::ImageMipArraySlice{}},
                {task_motion_vectors_image, daxa::TaskImageAccess::SHADER_READ_ONLY, daxa::ImageMipArraySlice{}},
                {task_depth_image, daxa::TaskImageAccess::SHADER_READ_ONLY, daxa::ImageMipArraySlice{}},
                {task_display_image, daxa::TaskImageAccess::SHADER_WRITE_ONLY, daxa::ImageMipArraySlice{}},
            },
            .task = [this](daxa::TaskRuntime runtime)
            {
                if (fsr_enabled)
                {
                    auto cmd_list = runtime.get_command_list();
                    upscale_context.upscale(
                        cmd_list,
                        {
                            .color = color_image,
                            .depth = depth_image,
                            .motion_vectors = motion_vectors_image,
                            .output = display_image,
                            .should_reset = false,
                            .delta_time = elapsed_s,
                            .jitter = jitter,
                            .should_sharpen = false,
                            .sharpening = 0.0f,
                            .camera_info = {
                                .near_plane = player.camera.near_clip,
                                .far_plane = player.camera.far_clip,
                                .vertical_fov = glm::radians(player.camera.fov),
                            },
                        });
                }
            },
            .debug_name = APPNAME_PREFIX("Upscale Task"),
        });

        new_task_list.add_task({
            .used_images = {
                {task_display_image, daxa::TaskImageAccess::TRANSFER_READ, daxa::ImageMipArraySlice{}},
                {task_swapchain_image, daxa::TaskImageAccess::TRANSFER_WRITE, daxa::ImageMipArraySlice{}},
            },
            .task = [this](daxa::TaskRuntime runtime)
            {
                auto cmd_list = runtime.get_command_list();
                cmd_list.blit_image_to_image({
                    .src_image = display_image,
                    .src_image_layout = daxa::ImageLayout::TRANSFER_SRC_OPTIMAL,
                    .dst_image = swapchain_image,
                    .dst_image_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
                    .src_slice = {.image_aspect = daxa::ImageAspectFlagBits::COLOR},
                    .src_offsets = {{{0, 0, 0}, {static_cast<i32>(size_x), static_cast<i32>(size_y), 1}}},
                    .dst_slice = {.image_aspect = daxa::ImageAspectFlagBits::COLOR},
                    .dst_offsets = {{{0, 0, 0}, {static_cast<i32>(size_x), static_cast<i32>(size_y), 1}}},
                });
            },
            .debug_name = APPNAME_PREFIX("Blit Task (display to swapchain)"),
        });

        new_task_list.add_task({
            .used_images = {
                {task_swapchain_image, daxa::TaskImageAccess::COLOR_ATTACHMENT, daxa::ImageMipArraySlice{}},
            },
            .task = [this](daxa::TaskRuntime runtime)
            {
                auto cmd_list = runtime.get_command_list();
                imgui_renderer.record_commands(ImGui::GetDrawData(), cmd_list, swapchain_image, size_x, size_y);
            },
            .debug_name = APPNAME_PREFIX("ImGui Task"),
        });
        new_task_list.submit(&submit_info);
        new_task_list.present({});
        new_task_list.complete();

        return new_task_list;
    }
};

auto main() -> int
{
    App app = {};
    while (true)
    {
        if (app.update())
        {
            break;
        }
    }
}
