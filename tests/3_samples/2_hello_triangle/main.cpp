#include <0_common/window.hpp>
#include <thread>

using namespace daxa::types;
using Clock = std::chrono::high_resolution_clock;

struct Vertex
{
    f32 x, y, z, w;
    f32 r, g, b, a;
};

struct RasterPush
{
    daxa::BufferId vertex_buffer_id;
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
        .debug_name = "HelloTriangle Swapchain",
    });

    daxa::PipelineCompiler pipeline_compiler = device.create_pipeline_compiler({
        .root_paths = {
            "tests/3_samples/2_hello_triangle/shaders",
            "include",
        },
        .debug_name = "HelloTriangle Compiler",
    });
    // clang-format off
    daxa::RasterPipeline raster_pipeline = pipeline_compiler.create_raster_pipeline({
        .vertex_shader_info = {.source = daxa::ShaderFile{"vert.hlsl"}},
        .fragment_shader_info = {.source = daxa::ShaderFile{"frag.hlsl"}},
        .color_attachments = {{.format = swapchain.get_format()}},
        .raster = {},
        .push_constant_size = sizeof(RasterPush),
        .debug_name = "HelloTriangle Pipeline",
    }).value();
    // clang-format on

    daxa::BufferId vertex_buffer = device.create_buffer(daxa::BufferInfo{
        .size = sizeof(Vertex) * 3,
        .debug_name = "HelloTriangle Vertex buffer",
    });

    daxa::BinarySemaphore binary_semaphore = device.create_binary_semaphore({
        .debug_name = "HelloTriangle Present Semaphore",
    });

    static inline constexpr u64 FRAMES_IN_FLIGHT = 1;
    daxa::TimelineSemaphore gpu_framecount_timeline_sema = device.create_timeline_semaphore(daxa::TimelineSemaphoreInfo{
        .initial_value = 0,
        .debug_name = "HelloTriangle gpu framecount Timeline Semaphore",
    });
    u64 cpu_framecount = FRAMES_IN_FLIGHT - 1;

    // Clock::time_point start = Clock::now();

    bool should_resize = false;

    App()
    {
    }

    ~App()
    {
        device.destroy_buffer(vertex_buffer);
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
        if (pipeline_compiler.check_if_sources_changed(raster_pipeline))
        {
            auto new_pipeline = pipeline_compiler.recreate_raster_pipeline(raster_pipeline);
            if (new_pipeline.is_ok())
            {
                raster_pipeline = new_pipeline.value();
            }
        }

        if (should_resize)
        {
            do_resize();
        }

        auto swapchain_image = swapchain.acquire_next_image();

        auto cmd_list = device.create_command_list({
            .debug_name = "HelloTriangle Command List",
        });

        // auto now = Clock::now();
        // auto elapsed = std::chrono::duration<float>(now - start).count();

        auto vertex_staging_buffer = device.create_buffer({
            .memory_flags = daxa::MemoryFlagBits::HOST_ACCESS_RANDOM,
            .size = sizeof(Vertex) * 3,
            .debug_name = "HelloTriangle Vertex Staging buffer",
        });
        cmd_list.destroy_buffer_deferred(vertex_staging_buffer);

        auto buffer_ptr = reinterpret_cast<Vertex *>(device.map_memory(vertex_staging_buffer));
        *buffer_ptr = Vertex{-0.5f, +0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f};
        ++buffer_ptr;
        *buffer_ptr = Vertex{+0.5f, +0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f};
        ++buffer_ptr;
        *buffer_ptr = Vertex{+0.0f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f};
        ++buffer_ptr;
        device.unmap_memory(vertex_staging_buffer);

        cmd_list.pipeline_barrier({
            .awaited_pipeline_access = daxa::PipelineStageAccessFlagBits::HOST_WRITE,
            .waiting_pipeline_access = daxa::PipelineStageAccessFlagBits::TRANSFER_READ,
        });

        cmd_list.copy_buffer_to_buffer({
            .src_buffer = vertex_staging_buffer,
            .dst_buffer = vertex_buffer,
            .size = sizeof(Vertex) * 3,
        });

        cmd_list.pipeline_barrier_image_transition({
            .awaited_pipeline_access = daxa::PipelineStageAccessFlagBits::TRANSFER_WRITE,
            .waiting_pipeline_access = daxa::PipelineStageAccessFlagBits::VERTEX_SHADER_READ,
            .before_layout = daxa::ImageLayout::UNDEFINED,
            .after_layout = daxa::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,
            .image_id = swapchain_image,
        });

        cmd_list.begin_renderpass({
            .color_attachments = {{.image = swapchain_image}},
            .render_area = {.x = 0, .y = 0, .width = size_x, .height = size_y},
        });
        cmd_list.set_pipeline(raster_pipeline);
        cmd_list.push_constant(RasterPush{
            .vertex_buffer_id = vertex_buffer,
        });
        cmd_list.draw({.vertex_count = 3});
        cmd_list.end_renderpass();

        cmd_list.pipeline_barrier_image_transition({
            .awaited_pipeline_access = daxa::PipelineStageAccessFlagBits::TRANSFER_WRITE,
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
        // printf("ahead: %llu\n", cpu_framecount - gpu_framecount_timeline_sema.value());
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
