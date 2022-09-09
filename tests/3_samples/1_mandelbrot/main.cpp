#include <0_common/window.hpp>
#include <thread>
#include <iostream>

#define DAXA_GLSL 1
#define DAXA_HLSL 0

#include "shaders/shared.inl"

#define APPNAME "Daxa Sample: Mandelbrot"
#define APPNAME_PREFIX(x) ("[" APPNAME "] " x)

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
                "tests/3_samples/1_mandelbrot/shaders",
                "include",
            },
            .opt_level = 2,
#if DAXA_GLSL
            .language = daxa::ShaderLanguage::GLSL,
#elif DAXA_HLSL
            .language = daxa::ShaderLanguage::HLSL,
#endif
        },
        .debug_name = APPNAME_PREFIX("pipeline_compiler"),
    });
    // clang-format off
    daxa::ComputePipeline compute_pipeline = pipeline_compiler.create_compute_pipeline({
#if DAXA_GLSL
        .shader_info = {.source = daxa::ShaderFile{"compute.glsl"}},
#elif DAXA_HLSL
        .shader_info = {.source = daxa::ShaderFile{"compute.hlsl"}},
#endif
        .push_constant_size = sizeof(ComputePush),
        .debug_name = APPNAME_PREFIX("compute_pipeline"),
    }).value();
    // clang-format on

    daxa::BufferId compute_input_buffer = device.create_buffer(daxa::BufferInfo{
        .size = sizeof(ComputeInput),
        .debug_name = APPNAME_PREFIX("compute_input_buffer"),
    });
    ComputeInput compute_input = {};

    daxa::ImageId render_image = device.create_image(daxa::ImageInfo{
        .format = daxa::Format::R8G8B8A8_UNORM,
        .size = {size_x, size_y, 1},
        .usage = daxa::ImageUsageFlagBits::SHADER_READ_WRITE | daxa::ImageUsageFlagBits::TRANSFER_SRC,
        .debug_name = APPNAME_PREFIX("render_image"),
    });

    daxa::BinarySemaphore acquire_semaphore = device.create_binary_semaphore({.debug_name = APPNAME_PREFIX("acquire_semaphore")});
    daxa::BinarySemaphore present_semaphore = device.create_binary_semaphore({.debug_name = APPNAME_PREFIX("present_semaphore")});

    Clock::time_point start = Clock::now();

    App() : AppWindow<App>(APPNAME) {}

    ~App()
    {
        device.wait_idle();
        device.collect_garbage();
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
            std::cout << new_pipeline.to_string() << std::endl;
            if (new_pipeline.is_ok())
            {
                compute_pipeline = new_pipeline.value();
            }
        }

        auto swapchain_image = swapchain.acquire_next_image(acquire_semaphore);

        auto cmd_list = device.create_command_list({
            .debug_name = APPNAME_PREFIX("cmd_list"),
        });

        auto now = Clock::now();
        auto elapsed = std::chrono::duration<float>(now - start).count();

        auto compute_input_staging_buffer = device.create_buffer({
            .memory_flags = daxa::MemoryFlagBits::HOST_ACCESS_RANDOM,
            .size = sizeof(ComputeInput),
            .debug_name = APPNAME_PREFIX("compute_input_staging_buffer"),
        });
        cmd_list.destroy_buffer_deferred(compute_input_staging_buffer);

        compute_input.time = elapsed;
        auto buffer_ptr = device.map_memory_as<ComputeInput>(compute_input_staging_buffer);
        *buffer_ptr = compute_input;
        device.unmap_memory(compute_input_staging_buffer);

        cmd_list.pipeline_barrier({
            .awaited_pipeline_access = daxa::AccessConsts::HOST_WRITE,
            .waiting_pipeline_access = daxa::AccessConsts::TRANSFER_READ,
        });

        cmd_list.copy_buffer_to_buffer({
            .src_buffer = compute_input_staging_buffer,
            .dst_buffer = compute_input_buffer,
            .size = sizeof(ComputeInput),
        });

        cmd_list.pipeline_barrier({
            .awaited_pipeline_access = daxa::AccessConsts::TRANSFER_WRITE,
            .waiting_pipeline_access = daxa::AccessConsts::COMPUTE_SHADER_READ,
        });

        cmd_list.pipeline_barrier_image_transition({
            .waiting_pipeline_access = daxa::AccessConsts::COMPUTE_SHADER_WRITE,
            .before_layout = daxa::ImageLayout::UNDEFINED,
            .after_layout = daxa::ImageLayout::GENERAL,
            .image_id = render_image,
        });

        cmd_list.set_pipeline(compute_pipeline);
        cmd_list.push_constant(ComputePush {
            .image_id = render_image.default_view(),
#if DAXA_GLSL
            .compute_input = this->device.buffer_reference(compute_input_buffer),
#elif DAXA_HLSL
                .input_buffer_id = compute_input_buffer,
#endif
            .frame_dim = {size_x, size_y},
        });
        cmd_list.dispatch((size_x + 7) / 8, (size_y + 7) / 8);

        cmd_list.pipeline_barrier({
            .waiting_pipeline_access = daxa::AccessConsts::COMPUTE_SHADER_WRITE,
        });

        cmd_list.pipeline_barrier_image_transition({
            .waiting_pipeline_access = daxa::AccessConsts::TRANSFER_WRITE,
            .before_layout = daxa::ImageLayout::UNDEFINED,
            .after_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
            .image_id = swapchain_image,
        });

        cmd_list.pipeline_barrier_image_transition({
            .awaited_pipeline_access = daxa::AccessConsts::COMPUTE_SHADER_WRITE,
            .waiting_pipeline_access = daxa::AccessConsts::TRANSFER_READ,
            .before_layout = daxa::ImageLayout::GENERAL,
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
            .awaited_pipeline_access = daxa::AccessConsts::TRANSFER_WRITE,
            .before_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
            .after_layout = daxa::ImageLayout::PRESENT_SRC,
            .image_id = swapchain_image,
        });

        cmd_list.pipeline_barrier_image_transition({
            .awaited_pipeline_access = daxa::AccessConsts::TRANSFER_READ,
            .before_layout = daxa::ImageLayout::TRANSFER_SRC_OPTIMAL,
            .after_layout = daxa::ImageLayout::GENERAL,
            .image_id = render_image,
        });

        cmd_list.complete();

        device.submit_commands({
            .command_lists = {std::move(cmd_list)},
            .wait_binary_semaphores = {acquire_semaphore},
            .signal_binary_semaphores = {present_semaphore},
        });

        device.present_frame({
            .wait_binary_semaphores = {present_semaphore},
            .swapchain = swapchain,
        });
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
            device.destroy_image(render_image);
            render_image = device.create_image({
                .format = daxa::Format::R8G8B8A8_UNORM,
                .size = {size_x, size_y, 1},
                .usage = daxa::ImageUsageFlagBits::SHADER_READ_WRITE | daxa::ImageUsageFlagBits::TRANSFER_SRC,
            });
            swapchain.resize();
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
