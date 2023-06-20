#define DAXA_SHADERLANG DAXA_SHADERLANG_GLSL
#define APPNAME "Daxa Sample: Mandelbrot"
#include <0_common/base_app.hpp>

using namespace daxa::types;
#include "shaders/shared.inl"

struct App : BaseApp<App>
{
    bool my_toggle = true;
    void update_virtual_shader()
    {
        if (my_toggle)
        {
            pipeline_manager.add_virtual_include_file({
                .name = "custom file!!",
                .contents = R"(
                    #pragma once
                    #define MY_TOGGLE 1
                )",
            });
        }
        else
        {
            pipeline_manager.add_virtual_include_file({
                .name = "custom file!!",
                .contents = R"(
                    #pragma once
                    #define MY_TOGGLE 0
                )",
            });
        }
    }

    // clang-format off
    std::shared_ptr<daxa::ComputePipeline> compute_pipeline = [this]() {
        update_virtual_shader();
        return pipeline_manager.add_compute_pipeline({
#if DAXA_SHADERLANG == DAXA_SHADERLANG_GLSL
            .shader_info = {.source = daxa::ShaderFile{"compute.glsl"}},
#elif DAXA_SHADERLANG == DAXA_SHADERLANG_HLSL
            .shader_info = {.source = daxa::ShaderFile{"compute.hlsl"}},
#endif
            .push_constant_size = sizeof(ComputePush),
            .name = "compute_pipeline",
        }).value();
    }();
    // clang-format on

    daxa::BufferId gpu_input_buffer = device.create_buffer(daxa::BufferInfo{
        .size = sizeof(GpuInput),
        .name = "gpu_input_buffer",
    });
    GpuInput gpu_input = {};
    daxa::TaskBuffer task_gpu_input_buffer{{.initial_buffers={.buffers=std::array{gpu_input_buffer}}, .name = "input_buffer"}};

    daxa::ImageId render_image = device.create_image(daxa::ImageInfo{
        .format = daxa::Format::R8G8B8A8_UNORM,
        .size = {size_x, size_y, 1},
        .usage = daxa::ImageUsageFlagBits::SHADER_READ_WRITE | daxa::ImageUsageFlagBits::TRANSFER_SRC,
        .name = "render_image",
    });
    daxa::TaskImage task_render_image{{.initial_images={.images=std::array{render_image}}, .name = "render_image"}};

    daxa::TimelineQueryPool timeline_query_pool = device.create_timeline_query_pool({
        .query_count = 2,
        .name = "timeline_query",
    });

    daxa::TaskList loop_task_list = record_loop_task_list();

    ~App()
    {
        device.wait_idle();
        device.collect_garbage();
        device.destroy_buffer(gpu_input_buffer);
        device.destroy_image(render_image);
    }

    void ui_update()
    {
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::Begin("Settings");
        if (ImGui::Checkbox("MY_TOGGLE", &my_toggle))
        {
            update_virtual_shader();
        }
        ImGui::End();
        ImGui::Render();
    }
    void on_update()
    {
        gpu_input.time = time;
        gpu_input.delta_time = delta_time;

        auto reloaded_result = pipeline_manager.reload_all();
        if (reloaded_result.has_value())
        {
            std::cout << reloaded_result.value().to_string() << std::endl;
        }

        ui_update();

        auto swapchain_image = swapchain.acquire_next_image();
        task_swapchain_image.set_images({.images=std::array{swapchain_image}});
        if (swapchain_image.is_empty())
        {
            return;
        }
        loop_task_list.execute({});

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
            render_image = device.create_image({
                .format = daxa::Format::R8G8B8A8_UNORM,
                .size = {size_x, size_y, 1},
                .usage = daxa::ImageUsageFlagBits::SHADER_READ_WRITE | daxa::ImageUsageFlagBits::TRANSFER_SRC,
            });
            task_render_image.set_images({.images=std::array{render_image}});
            base_on_update();
        }
    }

    void record_tasks(daxa::TaskList & new_task_list)
    {
        using namespace daxa::task_resource_uses;

        new_task_list.use_persistent_image(task_render_image);
        new_task_list.use_persistent_buffer(task_gpu_input_buffer);

        new_task_list.add_task({
            .uses = {
                BufferHostTransferWrite{task_gpu_input_buffer},
            },
            .task = [this](daxa::TaskInterface runtime)
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
                    .size = sizeof(GpuInput),
                    .allocate_info = daxa::MemoryFlagBits::HOST_ACCESS_RANDOM,
                    .name = APPNAME_PREFIX("staging_gpu_input_buffer"),
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
            .name = APPNAME_PREFIX("Upload Input"),
        });
        new_task_list.add_task({
            .uses = {
                BufferComputeShaderRead{task_gpu_input_buffer},
                ImageComputeShaderWrite<>{task_render_image},
            },
            .task = [this](daxa::TaskInterface runtime)
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
            .name = APPNAME_PREFIX("Draw (Compute)"),
        });
        new_task_list.add_task({
            .uses = {
                ImageTransferRead<>{task_render_image},
                ImageTransferWrite<>{task_swapchain_image},
            },
            .task = [this](daxa::TaskInterface ti)
            {
                auto cmd_list = ti.get_command_list();
                cmd_list.blit_image_to_image({
                    .src_image = ti.uses[task_render_image].image(),
                    .src_image_layout = daxa::ImageLayout::TRANSFER_SRC_OPTIMAL,
                    .dst_image = ti.uses[task_swapchain_image].image(),
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
            .name = "Blit (render to swapchain)",
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
