#define DAXA_SHADERLANG DAXA_SHADERLANG_GLSL
#define APPNAME "Daxa Sample: Mandelbrot"
#include <0_common/base_app.hpp>

using namespace daxa::types;
#include "shaders/shared.inl"

struct App : BaseApp<App>
{
    // clang-format off
    std::shared_ptr<daxa::ComputePipeline> compute_pipeline = pipeline_manager.add_compute_pipeline({
#if DAXA_SHADERLANG == DAXA_SHADERLANG_GLSL
        .shader_info = {.source = daxa::ShaderFile{"compute.glsl"}},
#elif DAXA_SHADERLANG == DAXA_SHADERLANG_HLSL
        .shader_info = {.source = daxa::ShaderFile{"compute.hlsl"}},
#endif
        .push_constant_size = sizeof(ComputePush),
        .debug_name = APPNAME_PREFIX("compute_pipeline"),
    }).value();
    // clang-format on

    daxa::BufferId gpu_input_buffer = device.create_buffer(daxa::BufferInfo{
        .size = sizeof(GpuInput),
        .debug_name = APPNAME_PREFIX("gpu_input_buffer"),
    });
    GpuInput gpu_input = {};
    daxa::TaskBufferId task_gpu_input_buffer;

    daxa::ImageId render_image = device.create_image(daxa::ImageInfo{
        .format = daxa::Format::R8G8B8A8_UNORM,
        .size = {size_x, size_y, 1},
        .usage = daxa::ImageUsageFlagBits::SHADER_READ_WRITE | daxa::ImageUsageFlagBits::TRANSFER_SRC,
        .debug_name = APPNAME_PREFIX("render_image"),
    });
    daxa::TaskImageId task_render_image;

    daxa::TimelineQueryPool timeline_query_pool = device.create_timeline_query_pool({
        .query_count = 2,
        .debug_name = "timeline_query",
    });

    daxa::TaskList loop_task_list = record_loop_task_list();

    ~App()
    {
        device.wait_idle();
        device.collect_garbage();
        device.destroy_buffer(gpu_input_buffer);
        device.destroy_image(render_image);
    }

    static void ui_update()
    {
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::Render();
    }
    void on_update()
    {
        gpu_input.time = time;
        gpu_input.delta_time = delta_time;

        auto reloaded_result = pipeline_manager.reload_all();
        if (reloaded_result.is_err())
        {
            std::cout << reloaded_result.to_string() << std::endl;
        }

        ui_update();

        loop_task_list.remove_runtime_image(task_swapchain_image, swapchain_image);
        swapchain_image = swapchain.acquire_next_image();
        loop_task_list.add_runtime_image(task_swapchain_image, swapchain_image);
        if (swapchain_image.is_empty())
        {
            return;
        }
        loop_task_list.execute();

        auto query_results = timeline_query_pool.get_query_results(0, 2);
        if ((query_results[1] != 0u) && (query_results[3] != 0u))
        {
            std::cout << "gpu execution took " << static_cast<f64>(query_results[2] - query_results[0]) / 1000000.0 << " ms" << std::endl;
        }
    }

    void on_mouse_move(f32 /*unused*/, f32 /*unused*/) {}
    void on_mouse_button(i32 /*unused*/, i32 /*unused*/) {}
    void on_key(i32 /*unused*/, i32 /*unused*/) {}
    void on_resize(u32 sx, u32 sy)
    {
        minimized = (sx == 0 || sy == 0);
        if (!minimized)
        {
            swapchain.resize();
            size_x = swapchain.get_surface_extent().x;
            size_y = swapchain.get_surface_extent().y;
            device.destroy_image(render_image);
            loop_task_list.remove_runtime_image(task_render_image, render_image);
            render_image = device.create_image({
                .format = daxa::Format::R8G8B8A8_UNORM,
                .size = {size_x, size_y, 1},
                .usage = daxa::ImageUsageFlagBits::SHADER_READ_WRITE | daxa::ImageUsageFlagBits::TRANSFER_SRC,
            });
            loop_task_list.add_runtime_image(task_render_image, render_image);
            base_on_update();
        }
    }

    void record_tasks(daxa::TaskList & new_task_list)
    {
        task_render_image = new_task_list.create_task_image({.debug_name = APPNAME_PREFIX("task_render_image")});
        new_task_list.add_runtime_image(task_render_image, render_image);
        task_gpu_input_buffer = new_task_list.create_task_buffer({.debug_name = APPNAME_PREFIX("task_gpu_input_buffer")});
        new_task_list.add_runtime_buffer(task_gpu_input_buffer, gpu_input_buffer);

        new_task_list.add_task({
            .used_buffers = {
                {task_gpu_input_buffer, daxa::TaskBufferAccess::HOST_TRANSFER_WRITE},
            },
            .task = [this](daxa::TaskRuntime runtime)
            {
                auto cmd_list = runtime.get_command_list();
                cmd_list.reset_timestamps({
                    .query_pool = timeline_query_pool,
                    .start_index = 0,
                    .count = timeline_query_pool.info().query_count,
                });
                cmd_list.write_timestamp({
                    .query_pool = timeline_query_pool,
                    .pipeline_stage = daxa::PipelineStageFlagBits::BOTTOM_OF_PIPE,
                    .query_index = 0,
                });
                auto staging_gpu_input_buffer = device.create_buffer({
                    .memory_flags = daxa::MemoryFlagBits::HOST_ACCESS_RANDOM,
                    .size = sizeof(GpuInput),
                    .debug_name = APPNAME_PREFIX("staging_gpu_input_buffer"),
                });
                cmd_list.destroy_buffer_deferred(staging_gpu_input_buffer);
                auto * buffer_ptr = device.get_host_address_as<GpuInput>(staging_gpu_input_buffer);
                *buffer_ptr = gpu_input;
                cmd_list.copy_buffer_to_buffer({
                    .src_buffer = staging_gpu_input_buffer,
                    .dst_buffer = gpu_input_buffer,
                    .size = sizeof(GpuInput),
                });
            },
            .debug_name = APPNAME_PREFIX("Upload Input"),
        });
        new_task_list.add_task({
            .used_buffers = {
                {task_gpu_input_buffer, daxa::TaskBufferAccess::COMPUTE_SHADER_READ_ONLY},
            },
            .used_images = {
                {task_render_image, daxa::TaskImageAccess::COMPUTE_SHADER_WRITE_ONLY, daxa::ImageMipArraySlice{}},
            },
            .task = [this](daxa::TaskRuntime runtime)
            {
                auto cmd_list = runtime.get_command_list();
                cmd_list.set_pipeline(*compute_pipeline);
                cmd_list.push_constant(ComputePush {
                    .image_id = render_image.default_view(),
#if DAXA_SHADERLANG == DAXA_SHADERLANG_GLSL
                    .gpu_input = this->device.get_device_address(gpu_input_buffer),
#elif DAXA_SHADERLANG == DAXA_SHADERLANG_HLSL
                    .input_buffer_id = gpu_input_buffer,
#endif
                    .frame_dim = {size_x, size_y},
                });
                cmd_list.dispatch((size_x + 7) / 8, (size_y + 7) / 8);
            },
            .debug_name = APPNAME_PREFIX("Draw (Compute)"),
        });
        new_task_list.add_task({
            .used_images = {
                {task_render_image, daxa::TaskImageAccess::TRANSFER_READ, daxa::ImageMipArraySlice{}},
                {task_swapchain_image, daxa::TaskImageAccess::TRANSFER_WRITE, daxa::ImageMipArraySlice{}},
            },
            .task = [this](daxa::TaskRuntime runtime)
            {
                auto cmd_list = runtime.get_command_list();
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

                cmd_list.write_timestamp({
                    .query_pool = timeline_query_pool,
                    .pipeline_stage = daxa::PipelineStageFlagBits::BOTTOM_OF_PIPE,
                    .query_index = 1,
                });
            },
            .debug_name = APPNAME_PREFIX("Blit (render to swapchain)"),
        });
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
