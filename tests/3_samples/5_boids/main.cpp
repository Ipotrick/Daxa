#include <0_common/window.hpp>
#include <thread>

using namespace daxa::types;
using Clock = std::chrono::high_resolution_clock;

struct BoidState
{
    f32 x = {};
    f32 y = {};
    f32 dir_x = 1.0f;
    f32 dir_y = {};
    f32 speed = {};
    u32 _pad[1] = {};
};

struct RasterPush
{
    daxa::BufferId boids_buffer_id;
};

struct UpdatePush
{
    daxa::BufferId prev_boids_buffer_id;
    daxa::BufferId boids_buffer_id;
    f32 delta_time;
};

struct App : AppWindow<App>
{
    daxa::Context daxa_ctx = daxa::create_context({
        .enable_validation = true,
    });
    daxa::Device device = daxa_ctx.create_default_device();

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
        .debug_name = "Boids Swapchain",
    });

    daxa::PipelineCompiler pipeline_compiler = device.create_pipeline_compiler({
        .root_paths = {
            "tests/3_samples/5_boids/shaders",
            "include",
        },
        .debug_name = "Boids Compiler",
    });
    // clang-format off
    daxa::RasterPipeline raster_pipeline = pipeline_compiler.create_raster_pipeline({
        .vertex_shader_info = {.source = daxa::ShaderFile{"vert.hlsl"}},
        .fragment_shader_info = {.source = daxa::ShaderFile{"frag.hlsl"}},
        .color_attachments = {{.format = swapchain.get_format()}},
        .raster = {},
        .push_constant_size = sizeof(RasterPush),
        .debug_name = "Boids Raster Pipeline",
    }).value();
    daxa::ComputePipeline update_pipeline = pipeline_compiler.create_compute_pipeline({
        .shader_info = {.source = daxa::ShaderFile{"update_boids.hlsl"}},
        .push_constant_size = sizeof(UpdatePush),
        .debug_name = "Boids Update Pipeline",
    }).value();
    // clang-format on

    static inline constexpr usize MAX_BOIDS = 1000;

    daxa::BufferId boids_buffers[2] = {
        device.create_buffer(daxa::BufferInfo{
            .size = sizeof(BoidState) * MAX_BOIDS,
            .debug_name = "Boids buffer 0",
        }),
        device.create_buffer(daxa::BufferInfo{
            .size = sizeof(BoidState) * MAX_BOIDS,
            .debug_name = "Boids buffer 1",
        }),
    };

    daxa::BinarySemaphore binary_semaphore = device.create_binary_semaphore({
        .debug_name = "Boids Present Semaphore",
    });

    static inline constexpr u64 FRAMES_IN_FLIGHT = 1;
    daxa::TimelineSemaphore gpu_framecount_timeline_sema = device.create_timeline_semaphore(daxa::TimelineSemaphoreInfo{
        .initial_value = 0,
        .debug_name = "Boids gpu framecount Timeline Semaphore",
    });
    u64 cpu_framecount = FRAMES_IN_FLIGHT - 1;

    bool should_resize = false;
    u32 current_buffer_i = 1;

    using Clock = std::chrono::high_resolution_clock;
    Clock::time_point start = Clock::now(), prev_time = start;

    App() : AppWindow<App>("Samples: Boids")
    {
        auto cmd_list = device.create_command_list({
            .debug_name = "Boids Init Commandlist",
        });

        auto init_boids_buffer = device.create_buffer({
            .memory_flags = daxa::MemoryFlagBits::HOST_ACCESS_RANDOM,
            .size = sizeof(BoidState) * 1000,
            .debug_name = "Boids Vertex Staging buffer",
        });
        cmd_list.destroy_buffer_deferred(init_boids_buffer);

        auto buffer_ptr = reinterpret_cast<BoidState *>(device.map_memory(init_boids_buffer));
        for (usize i = 0; i < MAX_BOIDS; ++i)
        {
            f32 angle = static_cast<f32>(static_cast<u32>(rand()) % MAX_BOIDS) / MAX_BOIDS * 3.14159f * 2.0f;
            buffer_ptr[i] = BoidState{
                .x = static_cast<f32>(static_cast<u32>(rand()) % MAX_BOIDS) / MAX_BOIDS - 0.5f,
                .y = static_cast<f32>(static_cast<u32>(rand()) % MAX_BOIDS) / MAX_BOIDS - 0.5f,
                .dir_x = std::cos(angle),
                .dir_y = std::sin(angle),
                .speed = 0.0f,
            };
        }
        device.unmap_memory(init_boids_buffer);

        cmd_list.pipeline_barrier({
            .awaited_pipeline_access = daxa::AccessConsts::HOST_WRITE,
            .waiting_pipeline_access = daxa::AccessConsts::TRANSFER_READ,
        });

        cmd_list.copy_buffer_to_buffer({
            .src_buffer = init_boids_buffer,
            .dst_buffer = boids_buffers[0],
            .size = sizeof(BoidState) * MAX_BOIDS,
        });

        cmd_list.pipeline_barrier({
            .awaited_pipeline_access = daxa::AccessConsts::TRANSFER_WRITE,
            .waiting_pipeline_access = daxa::AccessConsts::VERTEX_SHADER_READ,
        });

        cmd_list.complete();

        device.submit_commands({
            .command_lists = {cmd_list},
        });
    }

    ~App()
    {
        device.destroy_buffer(boids_buffers[0]);
        device.destroy_buffer(boids_buffers[1]);
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

        if (pipeline_compiler.check_if_sources_changed(raster_pipeline))
        {
            auto new_pipeline = pipeline_compiler.recreate_raster_pipeline(raster_pipeline);
            if (new_pipeline.is_ok())
            {
                raster_pipeline = new_pipeline.value();
            }
        }
        if (pipeline_compiler.check_if_sources_changed(update_pipeline))
        {
            auto new_pipeline = pipeline_compiler.recreate_compute_pipeline(update_pipeline);
            if (new_pipeline.is_ok())
            {
                update_pipeline = new_pipeline.value();
            }
        }

        if (should_resize)
        {
            do_resize();
        }

        auto swapchain_image = swapchain.acquire_next_image();

        auto cmd_list = device.create_command_list({
            .debug_name = "Boids Command List",
        });

        cmd_list.set_pipeline(update_pipeline);
        cmd_list.push_constant(UpdatePush{
            .prev_boids_buffer_id = boids_buffers[!current_buffer_i],
            .boids_buffer_id = boids_buffers[current_buffer_i],
            .delta_time = elapsed_s,
        });
        cmd_list.dispatch((MAX_BOIDS + 63) / 64);

        cmd_list.pipeline_barrier({
            .awaited_pipeline_access = daxa::AccessConsts::COMPUTE_SHADER_WRITE,
            .waiting_pipeline_access = daxa::AccessConsts::VERTEX_SHADER_READ,
        });

        cmd_list.pipeline_barrier_image_transition({
            .waiting_pipeline_access = daxa::AccessConsts::COLOR_ATTACHMENT_OUTPUT_WRITE,
            .before_layout = daxa::ImageLayout::UNDEFINED,
            .after_layout = daxa::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,
            .image_id = swapchain_image,
        });

        cmd_list.begin_renderpass({
            .color_attachments = {{.image_view = swapchain_image.default_view()}},
            .render_area = {.x = 0, .y = 0, .width = size_x, .height = size_y},
        });
        cmd_list.set_pipeline(raster_pipeline);
        cmd_list.push_constant(RasterPush{
            .boids_buffer_id = boids_buffers[current_buffer_i],
        });
        cmd_list.draw({.vertex_count = MAX_BOIDS * 3});
        cmd_list.end_renderpass();

        cmd_list.pipeline_barrier_image_transition({
            .awaited_pipeline_access = daxa::AccessConsts::TRANSFER_WRITE,
            .before_layout = daxa::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,
            .after_layout = daxa::ImageLayout::PRESENT_SRC,
            .image_id = swapchain_image,
        });

        cmd_list.complete();

        ++cpu_framecount;
        device.submit_commands({
            .command_lists = {std::move(cmd_list)},
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
