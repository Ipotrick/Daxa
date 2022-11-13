#include <0_common/window.hpp>
#include <thread>
#include <iostream>
#include <cmath>

#include "daxa/utils/task_list.hpp"

#define APPNAME "Daxa Sample: Boids"
#define APPNAME_PREFIX(x) ("[" APPNAME "] " x)

using namespace daxa::types;
using Clock = std::chrono::high_resolution_clock;

#include "shared.inl"

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

    daxa::PipelineCompiler pipeline_compiler = device.create_pipeline_compiler({
        .shader_compile_options = {
            .root_paths = {
                "tests/3_samples/5_boids/shaders",
                "tests/3_samples/5_boids",
                "include",
            },
            .language = daxa::ShaderLanguage::GLSL,
        },
        .debug_name = APPNAME_PREFIX("pipeline_compiler"),
    });
    // clang-format off
    daxa::RasterPipeline draw_pipeline = pipeline_compiler.create_raster_pipeline({
        .vertex_shader_info = {.source = daxa::ShaderFile{"vert.glsl"}},
        .fragment_shader_info = {.source = daxa::ShaderFile{"frag.glsl"}},
        .color_attachments = {{.format = swapchain.get_format()}},
        .raster = {},
        .push_constant_size = sizeof(DrawPushConstant),
        .debug_name = APPNAME_PREFIX("draw_pipeline"),
    }).value();
    daxa::ComputePipeline update_boids_pipeline = pipeline_compiler.create_compute_pipeline({
        .shader_info = {.source = daxa::ShaderFile{"update_boids.glsl"}},
        .push_constant_size = sizeof(UpdateBoidsPushConstant),
        .debug_name = APPNAME_PREFIX("draw_pipeline"),
    }).value();
    // clang-format on

    u32 current_buffer_i = 1;

    f32 aspect = static_cast<f32>(size_x) / static_cast<f32>(size_y);

    using Clock = std::chrono::high_resolution_clock;
    Clock::time_point start = Clock::now();
    Clock::time_point prev_time = start;

    daxa::BufferId boid_buffer = device.create_buffer({
        .size = sizeof(Boids),
        .debug_name = APPNAME_PREFIX("boid_buffer"),
    });

    daxa::BufferId old_boid_buffer = device.create_buffer({
        .size = sizeof(Boids),
        .debug_name = APPNAME_PREFIX("old_boid_buffer"),
    });

    daxa::ImageId swapchain_image = {};
    daxa::TaskImageId task_swapchain_image = {};

    daxa::CommandSubmitInfo submit_info;

    daxa::TaskList task_list = record_tasks();

    App() : AppWindow<App>(APPNAME)
    {
        auto cmd_list = device.create_command_list({.debug_name = APPNAME_PREFIX("boid buffer init commands")});

        auto upload_buffer_id = device.create_buffer({
            .memory_flags = daxa::MemoryFlagBits::HOST_ACCESS_SEQUENTIAL_WRITE,
            .size = sizeof(Boids),
            .debug_name = APPNAME_PREFIX("voids buffer init staging buffer"),
        });
        cmd_list.destroy_buffer_deferred(upload_buffer_id);

        Boids * ptr = device.map_memory_as<Boids>(upload_buffer_id);

        for (usize i = 0; i < MAX_BOIDS; ++i)
        {
            ptr->boids[i].position.x = static_cast<f32>(rand() % ((FIELD_SIZE)*100)) / 100.0f;
            ptr->boids[i].position.y = static_cast<f32>(rand() % ((FIELD_SIZE)*100)) / 100.0f;
            f32 angle = static_cast<f32>(rand() % 3600) * 0.1f;
            ptr->boids[i].direction.x = std::cos(angle);
            ptr->boids[i].direction.y = std::sin(angle);
        }

        device.unmap_memory(upload_buffer_id);

        cmd_list.copy_buffer_to_buffer({
            .src_buffer = upload_buffer_id,
            .dst_buffer = boid_buffer,
            .size = sizeof(Boids),
        });

        cmd_list.copy_buffer_to_buffer({
            .src_buffer = upload_buffer_id,
            .dst_buffer = old_boid_buffer,
            .size = sizeof(Boids),
        });

        cmd_list.pipeline_barrier({
            .awaited_pipeline_access = daxa::AccessConsts::TRANSFER_WRITE,
            .waiting_pipeline_access = daxa::AccessConsts::COMPUTE_SHADER_READ_WRITE | daxa::AccessConsts::VERTEX_SHADER_READ,
        });
        cmd_list.complete();
        device.submit_commands({
            .command_lists = {cmd_list},
        });
    }

    ~App()
    {
        device.wait_idle();
        device.collect_garbage();
        device.destroy_buffer(boid_buffer);
        device.destroy_buffer(old_boid_buffer);
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

    void update_boids(daxa::CommandList & cmd_list, daxa::BufferId boid_buffer_id, daxa::BufferId old_boid_buffer_id)
    {
        cmd_list.set_pipeline(update_boids_pipeline);

        cmd_list.push_constant(UpdateBoidsPushConstant{
            .boids_buffer = device.get_device_address(boid_buffer_id),
            .old_boids_buffer = device.get_device_address(old_boid_buffer_id),
        });

        cmd_list.dispatch((MAX_BOIDS + 63) / 64, 1, 1);
    }

    void draw_boids(daxa::CommandList & cmd_list, daxa::ImageId render_target, daxa::BufferId boid_buffer_id, u32 sx, u32 sy)
    {
        cmd_list.set_pipeline(draw_pipeline);
        cmd_list.begin_renderpass({
            .color_attachments = {
                {
                    .image_view = render_target.default_view(),
                    .layout = daxa::ImageLayout::ATTACHMENT_OPTIMAL,
                    .load_op = daxa::AttachmentLoadOp::CLEAR,
                    .store_op = daxa::AttachmentStoreOp::STORE,
                    .clear_value = std::array<f32, 4>{1.0f, 1.0f, 1.0f, 1.0f},
                }},
            .render_area = {
                .width = size_x,
                .height = size_y,
            },
        });

        cmd_list.push_constant(DrawPushConstant{
            .boids_buffer = device.get_device_address(boid_buffer_id),
            .axis_scaling = {
                std::min(1.0f, static_cast<f32>(sy) / static_cast<f32>(sx)),
                std::min(1.0f, static_cast<f32>(sx) / static_cast<f32>(sy)),
            }});

        cmd_list.draw({.vertex_count = 3 * MAX_BOIDS});

        cmd_list.end_renderpass();
    }

    auto record_tasks() -> daxa::TaskList
    {
        daxa::TaskList new_task_list = daxa::TaskList({.device = device, .swapchain = swapchain, .debug_name = APPNAME_PREFIX("main task list")});

        auto task_boid_buffer = new_task_list.create_task_buffer({});
        new_task_list.add_runtime_buffer(task_boid_buffer, this->boid_buffer);
        auto task_old_boid_buffer = new_task_list.create_task_buffer({});
        new_task_list.add_runtime_buffer(task_old_boid_buffer, this->old_boid_buffer);

        task_swapchain_image = new_task_list.create_task_image({.swapchain_image = true, });
        new_task_list.add_runtime_image(task_swapchain_image, this->swapchain_image);

        new_task_list.add_task({
            .used_buffers = {
                {task_boid_buffer, daxa::TaskBufferAccess::COMPUTE_SHADER_READ_WRITE},
                {task_old_boid_buffer, daxa::TaskBufferAccess::COMPUTE_SHADER_READ_ONLY},
            },
            .task = [=, this](daxa::TaskRuntime const & runtime)
            {
                BufferId boid_buffer_id = runtime.get_buffers(task_boid_buffer)[0];
                BufferId old_boid_buffer_id = runtime.get_buffers(task_old_boid_buffer)[0];
                daxa::CommandList cmd_list = runtime.get_command_list();
                this->update_boids(cmd_list, boid_buffer_id, old_boid_buffer_id);
            },
            .debug_name = "update boids",
        });

        new_task_list.add_task({
            .used_buffers = {
                {task_boid_buffer, daxa::TaskBufferAccess::VERTEX_SHADER_READ_ONLY},
            },
            .used_images = {
                {task_swapchain_image, daxa::TaskImageAccess::COLOR_ATTACHMENT, daxa::ImageMipArraySlice{}},
            },
            .task = [=, this](daxa::TaskRuntime const & runtime)
            {
                ImageId render_target_id = runtime.get_images(task_swapchain_image)[0];
                BufferId boid_buffer_id = runtime.get_buffers(task_boid_buffer)[0];
                daxa::CommandList cmd_list = runtime.get_command_list();
                this->draw_boids(cmd_list, render_target_id, boid_buffer_id, this->size_x, this->size_y);
            },
            .debug_name = "draw boids",
        });
        new_task_list.submit(&submit_info);
        new_task_list.present({});
        new_task_list.complete();

        return new_task_list;
    }

    void draw()
    {
        auto now = Clock::now();
        while (std::chrono::duration<f32>(now - prev_time).count() < SIMULATION_DELTA_TIME_S)
        {
            now = Clock::now();
        }
        prev_time = now;

        if (pipeline_compiler.check_if_sources_changed(draw_pipeline))
        {
            auto new_pipeline = pipeline_compiler.recreate_raster_pipeline(draw_pipeline);
            std::cout << new_pipeline.to_string() << std::endl;
            if (new_pipeline.is_ok())
            {
                draw_pipeline = new_pipeline.value();
            }
        }
        if (pipeline_compiler.check_if_sources_changed(update_boids_pipeline))
        {
            auto new_pipeline = pipeline_compiler.recreate_compute_pipeline(update_boids_pipeline);
            std::cout << new_pipeline.to_string() << std::endl;
            if (new_pipeline.is_ok())
            {
                update_boids_pipeline = new_pipeline.value();
            }
        }

        task_list.remove_runtime_image(task_swapchain_image, swapchain_image);
        swapchain_image = swapchain.acquire_next_image();
        task_list.add_runtime_image(task_swapchain_image, swapchain_image);
        if (swapchain_image.is_empty())
        {
            return;
        }
        task_list.execute();
    }

    void on_mouse_move(f32, f32) {}
    void on_mouse_button(i32, i32) {}
    void on_key(i32, i32) {}

    void on_resize(u32 sx, u32 sy)
    {
        minimized = (sx == 0 || sy == 0);
        if (!minimized)
        {
            swapchain.resize();
            size_x = swapchain.get_surface_extent().x;
            size_y = swapchain.get_surface_extent().y;
            aspect = static_cast<f32>(size_x) / static_cast<f32>(size_y);
            draw();
        }
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
