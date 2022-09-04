#include <0_common/window.hpp>
#include <thread>
#include <iostream>

#include "daxa/utils/task_list.hpp"

#define GLM_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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
        .width = size_x,
        .height = size_y,
        .surface_format_selector = [](daxa::Format format)
        {
            switch (format)
            {
            case daxa::Format::R8G8B8A8_UINT: return 100;
            default: return daxa::default_format_score(format);
            }
        },
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

    static inline constexpr u64 FRAMES_IN_FLIGHT = 1;
    daxa::TimelineSemaphore gpu_framecount_timeline_sema = device.create_timeline_semaphore(daxa::TimelineSemaphoreInfo{
        .initial_value = 0,
        .debug_name = APPNAME_PREFIX("gpu_framecount_timeline_sema"),
    });
    u64 cpu_framecount = FRAMES_IN_FLIGHT - 1;

    bool should_resize = false;
    u32 current_buffer_i = 1;

    f32 aspect = static_cast<f32>(size_x) / static_cast<f32>(size_y);

    using Clock = std::chrono::high_resolution_clock;
    Clock::time_point start = Clock::now(), prev_time = start;

    daxa::TaskList task_list = record_tasks();

    daxa::BufferId boid_buffer = device.create_buffer({
        .size = sizeof(BoidBuffer),
        .debug_name = APPNAME_PREFIX("boid_buffer"),
    });

    daxa::BufferId old_boid_buffer = device.create_buffer({
        .size = sizeof(BoidBuffer),
        .debug_name = APPNAME_PREFIX("old_boid_buffer"),
    });

    daxa::ImageId swapchain_image = {};
    daxa::TaskImageId task_swapchain_image = {};

    f32 elapsed_s = 1.0f;

    App() : AppWindow<App>(APPNAME)
    {
        auto cmd_list = device.create_command_list({ .debug_name = APPNAME_PREFIX("boid buffer init commands") });

        auto upload_buffer_id = device.create_buffer({
            .size = sizeof(BoidBuffer),
            .memory_flags = daxa::MemoryFlagBits::HOST_ACCESS_SEQUENTIAL_WRITE,
            .debug_name = APPNAME_PREFIX("voids buffer init staging buffer"),
        });
        cmd_list.destroy_buffer_deferred(upload_buffer_id);

        BoidBuffer* ptr = device.map_memory_as<BoidBuffer>(upload_buffer_id);

        for (usize i = 0; i < MAX_BOIDS; ++i)
        {
            ptr->boids[i].position.x = ((rand() % 1000) * (1.0f / 500.0f)) - 1.0f;
            ptr->boids[i].position.y = ((rand() % 1000) * (1.0f / 500.0f)) - 1.0f;
            ptr->boids[i].direction.x = 0.0f;
            ptr->boids[i].direction.y = 1.0f;
            ptr->boids[i].speed = 1.0f;
        }

        device.unmap_memory(upload_buffer_id);

        cmd_list.copy_buffer_to_buffer({
            .src_buffer = upload_buffer_id,
            .dst_buffer = boid_buffer,
            .size = sizeof(BoidBuffer),
        });

        cmd_list.copy_buffer_to_buffer({
            .src_buffer = upload_buffer_id,
            .dst_buffer = old_boid_buffer,
            .size = sizeof(BoidBuffer),
        });

        cmd_list.pipeline_barrier({
            .awaited_pipeline_access = daxa::AccessConsts::TRANSFER_WRITE,
            .waiting_pipeline_access = daxa::AccessConsts::COMPUTE_SHADER_READ_WRITE | daxa::AccessConsts::VERTEX_SHADER_READ,
        });
        cmd_list.complete();
        device.submit_commands({
            .command_lists = { cmd_list },
        });
    }

    ~App()
    {
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
            .boid_buffer_id = boid_buffer_id,
            .old_boid_buffer_id = old_boid_buffer_id,
            .delta_time = elapsed_s,
        });

        cmd_list.dispatch((MAX_BOIDS + 63) / 64, 1, 1);
    }

    void draw_boids(daxa::CommandList & cmd_list, daxa::ImageId render_target, daxa::BufferId boid_buffer_id, u32 size_x, u32 size_y)
    {
        cmd_list.set_pipeline(draw_pipeline);
        cmd_list.begin_renderpass({
            .color_attachments = {
                {
                    .image_view = render_target.default_view(),
                    .layout = daxa::ImageLayout::ATTACHMENT_OPTIMAL,
                    .load_op = daxa::AttachmentLoadOp::CLEAR,
                    .store_op = daxa::AttachmentStoreOp::STORE,
                    .clear_value = std::array<f32, 4>{ 1.0f, 1.0f, 1.0f, 1.0f },
                }
            },
            .render_area = {
                .width = size_x,
                .height = size_y,
            },
        });

        cmd_list.push_constant(DrawPushConstant{
            .boid_buffer_id = boid_buffer_id,
        });

        cmd_list.draw({.vertex_count = 3 * MAX_BOIDS });

        cmd_list.end_renderpass();
    }

    auto record_tasks() -> daxa::TaskList
    {
        daxa::TaskList task_list = daxa::TaskList({.device = device, .debug_name = APPNAME_PREFIX("main task list")});

        auto task_boid_buffer = task_list.create_task_buffer({
            .fetch_callback = [=]()
            { return boid_buffer; },
            .debug_name = "task boid buffer",
        });

        auto task_old_boid_buffer = task_list.create_task_buffer({
            .fetch_callback = [=]()
            { return old_boid_buffer; },
            .debug_name = "task old boid buffer",
        });

        task_swapchain_image = task_list.create_task_image({
            .fetch_callback = [=]()
            { return swapchain_image; },
            .debug_name = "task swapchain image",
        });

        task_list.add_task({
            .used_buffers = {
                { task_boid_buffer, daxa::TaskBufferAccess::COMPUTE_SHADER_READ_WRITE },
                { task_old_boid_buffer, daxa::TaskBufferAccess::COMPUTE_SHADER_READ_ONLY },
            },
            .task = [=](daxa::TaskInterface & interf)
            {
                BufferId boid_buffer_id = interf.get_buffer(task_boid_buffer);
                BufferId old_boid_buffer_id = interf.get_buffer(task_old_boid_buffer);
                daxa::CommandList cmd_list = interf.get_command_list();
                this->update_boids(cmd_list, boid_buffer_id, old_boid_buffer_id);
            },
            .debug_name = "update boids",
        });

        task_list.add_task({
            .used_buffers = {
                { task_boid_buffer, daxa::TaskBufferAccess::VERTEX_SHADER_READ_ONLY },
            },
            .used_images = {
                { task_swapchain_image, daxa::TaskImageAccess::COLOR_ATTACHMENT },
            },
            .task = [=](daxa::TaskInterface & interf)
            {
                ImageId render_target_id = interf.get_image(task_swapchain_image);
                BufferId boid_buffer_id = interf.get_buffer(task_boid_buffer);
                daxa::CommandList cmd_list = interf.get_command_list();
                this->draw_boids(cmd_list, render_target_id, boid_buffer_id, this->size_x, this->size_y);
            },
            .debug_name = "draw boids",
        });

        task_list.compile();

        return task_list;
    }

    void draw()
    {
        auto now = Clock::now();
        elapsed_s = std::chrono::duration<f32>(now - prev_time).count();
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

        if (should_resize)
        {
            do_resize();
        }

        swapchain_image = swapchain.acquire_next_image();

        std::swap(old_boid_buffer, boid_buffer);

        task_list.execute();
        
        auto commands = task_list.command_lists();

        auto cmd_list = device.create_command_list({ .debug_name = "frame finish command list" });

        cmd_list.pipeline_barrier_image_transition({
            .awaited_pipeline_access = task_list.last_access(task_swapchain_image),
            .before_layout = task_list.last_layout(task_swapchain_image),
            .after_layout = daxa::ImageLayout::PRESENT_SRC,
            .image_id = swapchain_image,
            .image_slice = device.info_image_view(swapchain_image.default_view()).slice,
        });

        cmd_list.complete();

        commands.push_back(cmd_list);

        auto binary_semaphore = device.create_binary_semaphore({.debug_name = "present semaphore"});

        ++cpu_framecount;
        device.submit_commands({
            .command_lists = commands,
            .signal_binary_semaphores = {binary_semaphore},
            .signal_timeline_semaphores = {{gpu_framecount_timeline_sema, cpu_framecount}},
        });

        device.present_frame({
            .wait_binary_semaphores = {binary_semaphore},
            .swapchain = swapchain,
        });

        gpu_framecount_timeline_sema.wait_for_value(cpu_framecount - 1);

        current_buffer_i = !current_buffer_i;
    }

    void on_mouse_move(f32, f32)
    {
    }

    void on_key(int, int)
    {
    }

    void on_resize(u32 sx, u32 sy)
    {
        size_x = sx;
        size_y = sy;
        minimized = (sx == 0 || sy == 0);

        if (!minimized)
        {
            should_resize = true;
            do_resize();
        }
    }

    void do_resize()
    {
        should_resize = false;
        swapchain.resize(size_x, size_y);
        aspect = static_cast<f32>(size_x) / static_cast<f32>(size_y);
        draw();
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
