#define DAXA_SHADERLANG DAXA_SHADERLANG_HLSL
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
            pipeline_manager.add_virtual_file({
                .name = "custom file!!",
                .contents = R"(
                    #pragma once
                    #define MY_TOGGLE 1
                )",
            });
        }
        else
        {
            pipeline_manager.add_virtual_file({
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
    daxa::TaskBuffer task_gpu_input_buffer{{.initial_buffers = {.buffers = std::array{gpu_input_buffer}}, .name = "input_buffer"}};

    daxa::ImageId render_image = device.create_image(daxa::ImageInfo{
        .format = daxa::Format::R8G8B8A8_UNORM,
        .size = {size_x, size_y, 1},
        .usage = daxa::ImageUsageFlagBits::SHADER_SAMPLED | daxa::ImageUsageFlagBits::SHADER_STORAGE | daxa::ImageUsageFlagBits::TRANSFER_SRC,
        .name = "render_image",
    });
    daxa::TaskImage task_render_image{{.initial_images = {.images = std::array{render_image}}, .name = "render_image"}};
    daxa::SamplerId sampler = device.create_sampler({.name = "sampler"});

    daxa::TimelineQueryPool timeline_query_pool = device.create_timeline_query_pool({
        .query_count = 2,
        .name = "timeline_query",
    });

    daxa::TaskGraph loop_task_graph = record_loop_task_graph();

    ~App()
    {
        device.wait_idle();
        device.collect_garbage();
        device.destroy_sampler(sampler);
        device.destroy_buffer(gpu_input_buffer);
        device.destroy_image(render_image);
    }

    void ui_update()
    {
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::Begin("Settings");

        ImGui::Image(
            imgui_renderer.create_texture_id({
                .image_view_id = render_image.default_view(),
                .sampler_id = sampler,
            }),
            ImVec2(200, 200));

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
        if (auto reload_err = daxa::get_if<daxa::PipelineReloadError>(&reloaded_result))
            std::cout << "Failed to reload " << reload_err->message << '\n';
        if (daxa::get_if<daxa::PipelineReloadSuccess>(&reloaded_result))
            std::cout << "Successfully reloaded!\n";

        ui_update();

        auto swapchain_image = swapchain.acquire_next_image();
        task_swapchain_image.set_images({.images = std::array{swapchain_image}});
        if (swapchain_image.is_empty())
        {
            return;
        }
        loop_task_graph.execute({});
        device.collect_garbage();

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
                .usage = daxa::ImageUsageFlagBits::SHADER_SAMPLED | daxa::ImageUsageFlagBits::SHADER_STORAGE | daxa::ImageUsageFlagBits::TRANSFER_SRC,
            });
            task_render_image.set_images({.images = std::array{render_image}});
            base_on_update();
        }
    }

    void record_tasks(daxa::TaskGraph & new_task_graph)
    {
        using namespace daxa::task_resource_uses;

        new_task_graph.use_persistent_image(task_render_image);
        new_task_graph.use_persistent_buffer(task_gpu_input_buffer);

        using namespace daxa::task_resource_uses;
        imgui_task_uses.push_back(ImageFragmentShaderSampled<>{task_render_image});

        new_task_graph.add_task({
            .uses = {
                BufferHostTransferWrite{task_gpu_input_buffer},
            },
            .task = [this](daxa::TaskInterface runtime)
            {
                auto& recorder = runtime.get_recorder();
                recorder.reset_timestamps({
                    .query_pool = timeline_query_pool,
                    .start_index = 0,
                    .count = timeline_query_pool.info().query_count,
                });
                recorder.write_timestamp({
                    .query_pool = timeline_query_pool,
                    .pipeline_stage = daxa::PipelineStageFlagBits::BOTTOM_OF_PIPE,
                    .query_index = 0,
                });
                auto staging_gpu_input_buffer = device.create_buffer({
                    .size = sizeof(GpuInput),
                    .allocate_info = daxa::MemoryFlagBits::HOST_ACCESS_RANDOM,
                    .name = ("staging_gpu_input_buffer"),
                });
                recorder.destroy_buffer_deferred(staging_gpu_input_buffer);
                auto * buffer_ptr = device.get_host_address_as<GpuInput>(staging_gpu_input_buffer).value();
                *buffer_ptr = gpu_input;
                recorder.copy_buffer_to_buffer({
                    .src_buffer = staging_gpu_input_buffer,
                    .dst_buffer = gpu_input_buffer,
                    .size = sizeof(GpuInput),
                });
            },
            .name = ("Upload Input"),
        });
        new_task_graph.add_task({
            .uses = {
                BufferComputeShaderRead{task_gpu_input_buffer},
                ImageComputeShaderStorageWriteOnly<>{task_render_image},
            },
            .task = [this](daxa::TaskInterface runtime)
            {
                auto& recorder = runtime.get_recorder();
                recorder.set_pipeline(*compute_pipeline);
                recorder.push_constant(ComputePush{
                    .image_id = render_image.default_view(),
                    .input_buffer_id = gpu_input_buffer,
                    .frame_dim = {size_x, size_y},
                });
                recorder.dispatch({(size_x + 7) / 8, (size_y + 7) / 8});
            },
            .name = ("Draw (Compute)"),
        });
        new_task_graph.add_task({
            .uses = {
                ImageTransferRead<>{task_render_image},
                ImageTransferWrite<>{task_swapchain_image},
            },
            .task = [this](daxa::TaskInterface ti)
            {
                auto& recorder = ti.get_recorder();
                recorder.blit_image_to_image({
                    .src_image = ti.uses[task_render_image].image(),
                    .src_image_layout = daxa::ImageLayout::TRANSFER_SRC_OPTIMAL,
                    .dst_image = ti.uses[task_swapchain_image].image(),
                    .dst_image_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
                    .src_offsets = {{{0, 0, 0}, {static_cast<i32>(size_x), static_cast<i32>(size_y), 1}}},
                    .dst_offsets = {{{0, 0, 0}, {static_cast<i32>(size_x), static_cast<i32>(size_y), 1}}},
                });

                recorder.write_timestamp({
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
