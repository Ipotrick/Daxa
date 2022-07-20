#include <0_common/window.hpp>
#include <thread>

using namespace daxa::types;
using Clock = std::chrono::high_resolution_clock;

struct ComputeInput
{
    float time;
};

struct ComputePush
{
    daxa::ImageId image_id;
    daxa::BufferId input_buffer_id;
    u32 frame_dim_x, frame_dim_y;
};

struct App : AppWindow<App>
{
    daxa::Context daxa_ctx = daxa::create_context({});
    daxa::Device device = daxa_ctx.create_default_device();

    daxa::Swapchain swapchain = device.create_swapchain({
#if defined(_WIN32)
        .native_window = glfwGetWin32Window(glfw_window_ptr),
#elif defined(__linux__)
        .native_window = glfwGetX11Window(glfw_window_ptr),
#endif
        .width = size_x,
        .height = size_y,
        .surface_format_selector =
            [](daxa::Format format)
        {
            switch (format)
            {
            case daxa::Format::R8G8B8A8_UINT: return 100;
            default: return daxa::default_format_score(format);
            }
        },
        .present_mode = daxa::PresentMode::DOUBLE_BUFFER_WAIT_FOR_VBLANK,
        .image_usage = daxa::ImageUsageFlagBits::TRANSFER_DST,
        .debug_name = "Mandelbrot Swapchain",
    });

    daxa::PipelineCompiler pipeline_compiler = device.create_pipeline_compiler({
        .root_paths = {
            "tests/3_samples/1_mandelbrot/shaders",
            "include",
        },
        .debug_name = "Mandelbrot Pipeline Compiler",
    });
    // clang-format off
    daxa::ComputePipeline compute_pipeline = pipeline_compiler.create_compute_pipeline({
        .shader_info = {.source = daxa::ShaderFile{"compute.hlsl"}},
        .push_constant_size = sizeof(ComputePush),
        .debug_name = "Mandelbrot Compute Pipeline",
    }).value();
    // clang-format on

    daxa::BufferId compute_input_buffer = device.create_buffer(daxa::BufferInfo{
        .size = sizeof(ComputeInput),
        .debug_name = "Mandelbrot Compute Input buffer",
    });
    ComputeInput compute_input = {};

    daxa::ImageId render_image = device.create_image(daxa::ImageInfo{
        .format = daxa::Format::R8G8B8A8_UNORM,
        .size = {size_x, size_y, 1},
        .usage = daxa::ImageUsageFlagBits::STORAGE | daxa::ImageUsageFlagBits::TRANSFER_SRC,
        .debug_name = "Mandelbrot Render Image",
    });

    daxa::BinarySemaphore binary_semaphore = device.create_binary_semaphore({
        .debug_name = "Mandelbrot Present Semaphore",
    });

    Clock::time_point start = Clock::now();

    App()
    {
    }

    ~App()
    {
        device.destroy_buffer(compute_input_buffer);
        device.destroy_image(render_image);
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
        if (pipeline_compiler.check_if_sources_changed(compute_pipeline))
        {
            auto new_pipeline = pipeline_compiler.recreate_compute_pipeline(compute_pipeline);
            if (new_pipeline.isOk())
            {
                compute_pipeline = new_pipeline.value();
            }
        }

        auto swapchain_image = swapchain.acquire_next_image();

        auto cmd_list = device.create_command_list({
            .debug_name = "Mandelbrot Command List",
        });

        auto now = Clock::now();
        auto elapsed = std::chrono::duration<float>(now - start).count();

        auto compute_input_staging_buffer = device.create_buffer({
            .memory_flags = daxa::MemoryFlagBits::HOST_ACCESS_RANDOM,
            .size = sizeof(ComputeInput),
            .debug_name = "Mandelbrot Compute Input Staging buffer",
        });
        cmd_list.destroy_buffer_deferred(compute_input_staging_buffer);

        compute_input.time = elapsed;
        auto buffer_ptr = reinterpret_cast<ComputeInput *>(device.map_memory(compute_input_staging_buffer));
        *buffer_ptr = compute_input;
        device.unmap_memory(compute_input_staging_buffer);

        cmd_list.pipeline_barrier({
            .awaited_pipeline_access = daxa::PipelineStageAccessFlagBits::HOST_WRITE,
            .waiting_pipeline_access = daxa::PipelineStageAccessFlagBits::TRANSFER_READ,
        });

        cmd_list.copy_buffer_to_buffer({
            .src_buffer = compute_input_staging_buffer,
            .dst_buffer = compute_input_buffer,
            .size = sizeof(ComputeInput),
        });

        cmd_list.pipeline_barrier({
            .awaited_pipeline_access = daxa::PipelineStageAccessFlagBits::TRANSFER_WRITE,
            .waiting_pipeline_access = daxa::PipelineStageAccessFlagBits::COMPUTE_SHADER_READ,
        });

        cmd_list.bind_pipeline(compute_pipeline);
        cmd_list.push_constant(ComputePush{
            .image_id = render_image,
            .input_buffer_id = compute_input_buffer,
            .frame_dim_x = size_x,
            .frame_dim_y = size_y,
        });
        cmd_list.dispatch((size_x + 7) / 8, (size_y + 7) / 8);

        cmd_list.pipeline_barrier({
            .waiting_pipeline_access = daxa::PipelineStageAccessFlagBits::COMPUTE_SHADER_WRITE,
        });

        cmd_list.pipeline_barrier_image_transition({
            .waiting_pipeline_access = daxa::PipelineStageAccessFlagBits::TRANSFER_WRITE,
            .before_layout = daxa::ImageLayout::UNDEFINED,
            .after_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
            .image_id = swapchain_image,
        });

        cmd_list.pipeline_barrier_image_transition({
            .waiting_pipeline_access = daxa::PipelineStageAccessFlagBits::TRANSFER_READ,
            .before_layout = daxa::ImageLayout::UNDEFINED,
            .after_layout = daxa::ImageLayout::TRANSFER_SRC_OPTIMAL,
            .image_id = render_image,
        });

        cmd_list.blit_image_to_image({
            .src_image = render_image,
            .src_image_layout = daxa::ImageLayout::TRANSFER_SRC_OPTIMAL,
            .dst_image = swapchain_image,
            .dst_image_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
            .src_slice = {.image_aspect = daxa::ImageAspectFlagBits::COLOR},
            .src_offsets = {{{0, 0, 0}, {static_cast<i32>(size_x), static_cast<i32>(size_y), 1}}},
            .dst_slice = {.image_aspect = daxa::ImageAspectFlagBits::COLOR},
            .dst_offsets = {{{0, 0, 0}, {static_cast<i32>(size_x), static_cast<i32>(size_y), 1}}},
        });

        cmd_list.pipeline_barrier_image_transition({
            .awaited_pipeline_access = daxa::PipelineStageAccessFlagBits::TRANSFER_WRITE,
            .before_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
            .after_layout = daxa::ImageLayout::PRESENT_SRC,
            .image_id = swapchain_image,
        });

        cmd_list.pipeline_barrier_image_transition({
            .awaited_pipeline_access = daxa::PipelineStageAccessFlagBits::TRANSFER_READ,
            .before_layout = daxa::ImageLayout::TRANSFER_SRC_OPTIMAL,
            .after_layout = daxa::ImageLayout::GENERAL,
            .image_id = render_image,
        });

        cmd_list.complete();

        device.submit_commands({
            .command_lists = {std::move(cmd_list)},
            .signal_binary_semaphores = {binary_semaphore},
        });

        device.present_frame({
            .wait_binary_semaphores = {binary_semaphore},
            .swapchain = swapchain,
        });
    }

    void on_resize(u32 sx, u32 sy)
    {
        size_x = sx;
        size_y = sy;
        minimized = (sx == 0 || sy == 0);

        if (!minimized)
        {
            device.destroy_image(render_image);
            render_image = device.create_image({
                .format = daxa::Format::R8G8B8A8_UNORM,
                .size = {size_x, size_y, 1},
                .usage = daxa::ImageUsageFlagBits::STORAGE | daxa::ImageUsageFlagBits::TRANSFER_SRC,
            });
            swapchain.resize(size_x, size_y);
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
