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
        .debug_name = "TaskList Swapchain",
    });

    daxa::PipelineCompiler pipeline_compiler = device.create_pipeline_compiler({
        .root_paths = {
            "tests/2_daxa_api/6_task_list/shaders",
            "include",
        },
        .debug_name = "TaskList Compiler",
    });
    // clang-format off
    daxa::RasterPipeline raster_pipeline = pipeline_compiler.create_raster_pipeline({
        .vertex_shader_info = {.source = daxa::ShaderFile{"vert.hlsl"}},
        .fragment_shader_info = {.source = daxa::ShaderFile{"frag.hlsl"}},
        .color_attachments = {{.format = swapchain.get_format()}},
        .raster = {},
        .push_constant_size = sizeof(RasterPush),
        .debug_name = "TaskList Pipeline",
    }).value();
    // clang-format on

    daxa::BufferId vertex_buffer = device.create_buffer(daxa::BufferInfo{
        .size = sizeof(Vertex) * 3,
        .debug_name = "TaskList Vertex buffer",
    });

    daxa::BinarySemaphore binary_semaphore = device.create_binary_semaphore({
        .debug_name = "TaskList Present Semaphore",
    });

    static inline constexpr u64 FRAMES_IN_FLIGHT = 1;
    daxa::TimelineSemaphore gpu_framecount_timeline_sema = device.create_timeline_semaphore(daxa::TimelineSemaphoreInfo{
        .initial_value = 0,
        .debug_name = "TaskList gpu framecount Timeline Semaphore",
    });
    u64 cpu_framecount = FRAMES_IN_FLIGHT - 1;

    daxa::TaskList task_list = daxa::TaskList({
        .device = device,
        .debug_name = "TaskList Task List",
    });

    bool should_resize = false;

    App()
    {
        daxa::TaskBufferId t_vertex_buffer = task_list.create_task_buffer({
            .fetch_callback = [this](daxa::TaskInterface &)
            { return vertex_buffer; },
        });
        daxa::TaskImageId t_swapchain_image = task_list.create_task_image({
            .fetch_callback = [this](daxa::TaskInterface &)
            { return swapchain.acquire_next_image(); },
        });

        task_list.add_task({
            .resources = {
                .buffers = {
                    {t_vertex_buffer, daxa::TaskBufferAccess::TRANSFER_WRITE},
                },
            },
            .task = [&](daxa::TaskInterface & task_interface)
            {
                // Id Fetch can be ommited, as it is known ahead of time.
                // daxa::BufferId vertex_buffer = interface.get_buffer(t_vertex_buffer);

                auto vertex_staging_buffer = task_interface.get_device().create_buffer({
                    .memory_flags = daxa::MemoryFlagBits::HOST_ACCESS_RANDOM | daxa::MemoryFlagBits::STRATEGY_MIN_TIME,
                    .size = sizeof(Vertex) * 3,
                    .debug_name = "TaskList Vertex Staging buffer",
                });
                task_interface.get_command_list().destroy_buffer_deferred(vertex_staging_buffer);

                auto buffer_ptr = reinterpret_cast<Vertex *>(task_interface.get_device().map_memory(vertex_staging_buffer));
                buffer_ptr[0] = Vertex{-0.5f, +0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f};
                buffer_ptr[1] = Vertex{+0.5f, +0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f};
                buffer_ptr[2] = Vertex{+0.0f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f};
                task_interface.get_device().unmap_memory(vertex_staging_buffer);

                task_interface.get_command_list().copy_buffer_to_buffer({
                    .src_buffer = vertex_staging_buffer,
                    .dst_buffer = vertex_buffer,
                    .size = sizeof(Vertex) * 3,
                });
            },
        });

        task_list.add_render_task({
            .render_info = {
                .color_attachments = {{.image = t_swapchain_image}},
                .render_area = {.x = 0, .y = 0, .width = size_x, .height = size_y},
            },
            .task = [this](daxa::TaskInterface & task_interface)
            {
                // daxa::BufferId vertex_buffer = task_interface.get_buffer(t_vertex_buffer);

                task_interface.get_command_list().set_pipeline(raster_pipeline);
                task_interface.get_command_list().push_constant(RasterPush{
                    .vertex_buffer_id = vertex_buffer,
                });
                task_interface.get_command_list().draw({.vertex_count = 3});
            },
        });

        task_list.add_present(swapchain);

        task_list.compile();
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

        task_list.execute();
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