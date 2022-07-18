#include <0_common/window.hpp>
#include <thread>

using namespace daxa::types;

struct ComputePush
{
    daxa::ImageId image_id;
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
        .debug_name = "Test1 Swapchain",
    });

    daxa::PipelineCompiler pipeline_compiler = device.create_pipeline_compiler({
        .root_paths = {
            "tests/3_samples/0_playground/shaders",
            "include",
        },
        .debug_name = "Test1 Pipeline Compiler",
    });
    // clang-format off
    daxa::ComputePipeline compute_pipeline = pipeline_compiler.create_compute_pipeline({
        .shader_info = {.source = daxa::ShaderFile{"compute.hlsl"}},
        .push_constant_size = sizeof(ComputePush),
        .debug_name = "Test1 Compute Pipeline",
    }).value();
    // clang-format on

    daxa::ImageId render_image = device.create_image(daxa::ImageInfo{
        .format = daxa::Format::R8G8B8A8_UNORM,
        .size = {size_x, size_y, 1},
        .usage = daxa::ImageUsageFlagBits::STORAGE | daxa::ImageUsageFlagBits::TRANSFER_SRC,
    });

    daxa::BufferId buffer = device.create_buffer({ .size = 64, });

    App()
    {
    }

    ~App()
    {
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
        auto swapchain_image = swapchain.acquire_next_image();

        auto binary_semaphore = device.create_binary_semaphore({
            .debug_name = "Test1 Present Semaphore",
        });
        auto cmd_list = device.create_command_list({
            .debug_name = "Test1 Command List",
        });

        cmd_list.bind_pipeline(compute_pipeline);
        cmd_list.push_constant(ComputePush{
            .image_id = render_image,
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
            .signal_binary_semaphores_on_completion = {binary_semaphore},
        });

        device.present_frame({
            .wait_on_binary = binary_semaphore,
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
