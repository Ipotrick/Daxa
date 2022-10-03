#include <0_common/window.hpp>
#include <0_common/player.hpp>
#include <0_common/hlsl_util.hpp>
#include <thread>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define APPNAME "Daxa Sample: Raster"
#define APPNAME_PREFIX(x) ("[" APPNAME "] " x)

using namespace daxa::types;
#include "shaders/shared.inl"

#include <0_common/voxels.hpp>

using namespace daxa::types;
using Clock = std::chrono::high_resolution_clock;

struct App : AppWindow<App>
{
    daxa::Context daxa_ctx = daxa::create_context({
        .enable_validation = true,
    });
    daxa::Device device = daxa_ctx.create_device({
        .debug_name = APPNAME_PREFIX("device"),
    });
    daxa::Swapchain swapchain = device.create_swapchain({
        .native_window = get_native_handle(),
        .native_window_platform = get_native_platform(),
        .present_mode = daxa::PresentMode::DO_NOT_WAIT_FOR_VBLANK,
        .image_usage = daxa::ImageUsageFlagBits::TRANSFER_DST,
        .debug_name = APPNAME_PREFIX("swapchain"),
    });
    daxa::ImageId depth_image = device.create_image({
        .format = daxa::Format::D24_UNORM_S8_UINT,
        .aspect = daxa::ImageAspectFlagBits::DEPTH | daxa::ImageAspectFlagBits::STENCIL,
        .size = {size_x, size_y, 1},
        .usage = daxa::ImageUsageFlagBits::DEPTH_STENCIL_ATTACHMENT,
    });
    daxa::PipelineCompiler pipeline_compiler = device.create_pipeline_compiler({
        .shader_compile_options = {
            .root_paths = {
                "tests/0_common/shaders",
                "tests/3_samples/4_raster/shaders",
                "include",
            },
        },
        .debug_name = APPNAME_PREFIX("pipeline_compiler"),
    });
    // clang-format off
    daxa::RasterPipeline fill_raster_pipeline = pipeline_compiler.create_raster_pipeline({
        .vertex_shader_info = {.source = daxa::ShaderFile{"draw.hlsl"}, .compile_options = {.entry_point = "vs_main"}},
        .fragment_shader_info = {.source = daxa::ShaderFile{"draw.hlsl"}, .compile_options = {.entry_point = "fs_main"}},
        .color_attachments = {{.format = swapchain.get_format(), .blend = {.blend_enable = true, .src_color_blend_factor = daxa::BlendFactor::SRC_ALPHA, .dst_color_blend_factor = daxa::BlendFactor::ONE_MINUS_SRC_ALPHA}}},
        .depth_test = {
            .depth_attachment_format = daxa::Format::D24_UNORM_S8_UINT,
            .enable_depth_test = true,
            .enable_depth_write = true,
        },
        .raster = {
            .polygon_mode = daxa::PolygonMode::FILL,
            .face_culling = daxa::FaceCullFlagBits::BACK_BIT,
        },
        .push_constant_size = sizeof(DrawPush),
        .debug_name = APPNAME_PREFIX("raster_pipeline (fill)"),
    }).value();
    daxa::RasterPipeline line_raster_pipeline = pipeline_compiler.create_raster_pipeline({
        .vertex_shader_info = {.source = daxa::ShaderFile{"line.hlsl"}, .compile_options = {.entry_point = "vs_main"}},
        .fragment_shader_info = {.source = daxa::ShaderFile{"line.hlsl"}, .compile_options = {.entry_point = "fs_main"}},
        .color_attachments = {{.format = swapchain.get_format(), .blend = {.blend_enable = true, .src_color_blend_factor = daxa::BlendFactor::SRC_ALPHA, .dst_color_blend_factor = daxa::BlendFactor::ONE_MINUS_SRC_ALPHA}}},
        .depth_test = {
            .depth_attachment_format = daxa::Format::D24_UNORM_S8_UINT,
            .enable_depth_test = true,
            .enable_depth_write = true,
        },
        .raster = {
            .polygon_mode = daxa::PolygonMode::LINE,
            .face_culling = daxa::FaceCullFlagBits::NONE,
            .depth_bias_enable = true,
            .depth_bias_constant_factor = -0.1f,
            .depth_bias_slope_factor = -0.8f,
            .line_width = 1.0f,
        },
        .push_constant_size = sizeof(DrawPush),
        .debug_name = APPNAME_PREFIX("raster_pipeline (line)"),
    }).value();
    // daxa::RasterPipeline *raster_pipeline = &fill_raster_pipeline;
    // clang-format on
    static inline constexpr u64 FRAMES_IN_FLIGHT = 1;
    daxa::TimelineSemaphore gpu_framecount_timeline_sema = device.create_timeline_semaphore(daxa::TimelineSemaphoreInfo{
        .initial_value = 0,
        .debug_name = APPNAME_PREFIX("gpu_framecount_timeline_sema"),
    });
    u64 cpu_framecount = FRAMES_IN_FLIGHT - 1;
    Clock::time_point start = Clock::now(), prev_time = start;
    RenderableVoxelWorld renderable_world{device};
    Player3D player = {
        .rot = {2.0f, 0.0f, 0.0f},
    };
    bool paused = true;

    daxa::BinarySemaphore acquire_semaphore = device.create_binary_semaphore({.debug_name = APPNAME_PREFIX("acquire_semaphore")});
    daxa::BinarySemaphore present_semaphore = device.create_binary_semaphore({.debug_name = APPNAME_PREFIX("present_semaphore")});

    App() : AppWindow<App>(APPNAME) {}

    ~App()
    {
        device.wait_idle();
        device.collect_garbage();
        device.destroy_image(depth_image);
    }

    bool update()
    {
        glfwPollEvents();
        if (glfwWindowShouldClose(glfw_window_ptr))
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
        auto elapsed_s = std::chrono::duration<f32>(now - prev_time).count();
        prev_time = now;

        player.camera.resize(static_cast<i32>(size_x), static_cast<i32>(size_y));
        player.camera.set_pos(player.pos);
        player.camera.set_rot(player.rot.x, player.rot.y);
        player.update(elapsed_s);

        if (pipeline_compiler.check_if_sources_changed(fill_raster_pipeline))
        {
            auto new_pipeline = pipeline_compiler.recreate_raster_pipeline(fill_raster_pipeline);
            std::cout << new_pipeline.to_string() << std::endl;
            if (new_pipeline.is_ok())
            {
                fill_raster_pipeline = new_pipeline.value();
            }
        }

        if (pipeline_compiler.check_if_sources_changed(line_raster_pipeline))
        {
            auto new_pipeline = pipeline_compiler.recreate_raster_pipeline(line_raster_pipeline);
            std::cout << new_pipeline.to_string() << std::endl;
            if (new_pipeline.is_ok())
            {
                line_raster_pipeline = new_pipeline.value();
            }
        }

        auto swapchain_image = swapchain.acquire_next_image(acquire_semaphore);

        auto cmd_list = device.create_command_list({
            .debug_name = APPNAME_PREFIX("cmd_list"),
        });

        cmd_list.pipeline_barrier_image_transition({
            .waiting_pipeline_access = daxa::AccessConsts::TRANSFER_WRITE,
            .before_layout = daxa::ImageLayout::UNDEFINED,
            .after_layout = daxa::ImageLayout::ATTACHMENT_OPTIMAL,
            .image_id = swapchain_image,
        });

        cmd_list.pipeline_barrier_image_transition({
            .waiting_pipeline_access = daxa::AccessConsts::TRANSFER_WRITE,
            .before_layout = daxa::ImageLayout::UNDEFINED,
            .after_layout = daxa::ImageLayout::ATTACHMENT_OPTIMAL,
            .image_slice = {.image_aspect = daxa::ImageAspectFlagBits::DEPTH | daxa::ImageAspectFlagBits::STENCIL},
            .image_id = depth_image,
        });

        cmd_list.begin_renderpass({
            .color_attachments = {{
                .image_view = swapchain_image.default_view(),
                .load_op = daxa::AttachmentLoadOp::CLEAR,
                .clear_value = std::array<f32, 4>{0.2f, 0.4f, 1.0f, 1.0f},
            }},
            .depth_attachment = {{
                .image_view = depth_image.default_view(),
                .load_op = daxa::AttachmentLoadOp::CLEAR,
                .clear_value = daxa::DepthValue{1.0f, 0},
            }},
            .render_area = {.x = 0, .y = 0, .width = size_x, .height = size_y},
        });

        auto mat = player.camera.get_vp();
        auto push = DrawPush{
            .view_mat = *reinterpret_cast<f32mat4x4 *>(&mat),
            .chunk_pos = {},
            .face_buffer_id = {},
            .texture_array_id = renderable_world.atlas_texture_array.default_view(),
            .sampler_id = renderable_world.atlas_sampler,
            .mode = {},
        };

        cmd_list.set_pipeline(fill_raster_pipeline);
        renderable_world.draw(cmd_list, push);

        cmd_list.set_pipeline(line_raster_pipeline);
        renderable_world.draw(cmd_list, push);

        cmd_list.end_renderpass();

        cmd_list.pipeline_barrier_image_transition({
            .awaited_pipeline_access = daxa::AccessConsts::TRANSFER_WRITE,
            .before_layout = daxa::ImageLayout::ATTACHMENT_OPTIMAL,
            .after_layout = daxa::ImageLayout::PRESENT_SRC,
            .image_id = swapchain_image,
        });

        cmd_list.complete();

        ++cpu_framecount;
        device.submit_commands({
            .command_lists = {std::move(cmd_list)},
            .wait_binary_semaphores = {acquire_semaphore},
            .signal_binary_semaphores = {present_semaphore},
            .signal_timeline_semaphores = {{gpu_framecount_timeline_sema, cpu_framecount}},
        });

        device.present_frame({
            .wait_binary_semaphores = {present_semaphore},
            .swapchain = swapchain,
        });

        gpu_framecount_timeline_sema.wait_for_value(cpu_framecount - 1);
    }

    void on_mouse_move(f32 x, f32 y)
    {
        if (!paused)
        {
            f32 center_x = static_cast<f32>(size_x / 2);
            f32 center_y = static_cast<f32>(size_y / 2);
            auto offset = f32vec2{x - center_x, center_y - y};
            player.on_mouse_move(offset.x, offset.y);
            set_mouse_pos(center_x, center_y);
        }
    }

    void on_mouse_button(i32, i32) {}

    void on_key(i32 key, i32 action)
    {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        {
            toggle_pause();
        }

        // if (key == GLFW_KEY_L && action == GLFW_PRESS)
        // {
        //     if (raster_pipeline == &fill_raster_pipeline)
        //         raster_pipeline = &line_raster_pipeline;
        //     else
        //         raster_pipeline = &fill_raster_pipeline;
        // }

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
            size_x = swapchain.info().width;
            size_y = swapchain.info().height;
            device.destroy_image(depth_image);
            depth_image = device.create_image({
                .format = daxa::Format::D24_UNORM_S8_UINT,
                .aspect = daxa::ImageAspectFlagBits::DEPTH | daxa::ImageAspectFlagBits::STENCIL,
                .size = {size_x, size_y, 1},
                .usage = daxa::ImageUsageFlagBits::DEPTH_STENCIL_ATTACHMENT,
            });
            draw();
        }
    }

    void toggle_pause()
    {
        set_mouse_capture(paused);
        paused = !paused;
    }
};

int main()
{
    App app = {};
    while (true)
    {
        if (app.update())
            break;
    }
}
