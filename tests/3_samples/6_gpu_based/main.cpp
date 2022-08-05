#include <0_common/window.hpp>
#include <0_common/player.hpp>
#include <0_common/hlsl_util.hpp>
#include <thread>
#include <iostream>
#include <daxa/utils/task_list.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using namespace daxa::types;
using Clock = std::chrono::high_resolution_clock;

#include "shared.inl"

// helper utils

auto recreate_pipeline(daxa::PipelineCompiler & compiler, auto const & pipeline)
{
    if constexpr (std::is_same_v<std::decay_t<decltype(pipeline)>, daxa::ComputePipeline>)
    {
        return compiler.recreate_compute_pipeline(pipeline);
    }
    else
    {
        return compiler.recreate_raster_pipeline(pipeline);
    }
}
auto refresh_pipeline(daxa::PipelineCompiler & compiler, auto & pipeline) -> bool
{
    if (compiler.check_if_sources_changed(pipeline))
    {
        auto new_pipeline = recreate_pipeline(compiler, pipeline);
        if (new_pipeline.is_ok())
        {
            pipeline = new_pipeline.value();
            return true;
        }
        else
        {
            std::cout << new_pipeline.message() << std::endl;
        }
    }
    return false;
}

// end helper utils

struct App : AppWindow<App>
{
    daxa::Context daxa_ctx = daxa::create_context({.enable_validation = true});
    daxa::Device device = daxa_ctx.create_device({});
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
        .debug_name = "Playground Swapchain",
    });
    daxa::ImageId swapchain_image;
    daxa::TaskImageId task_swapchain_image;
    daxa::ImageId draw_depth_image = create_depth();
    daxa::TaskImageId task_draw_depth_image;
    daxa::PipelineCompiler pipeline_compiler = device.create_pipeline_compiler({
        .root_paths = {
            "tests/0_common/shaders",
            "tests/3_samples/6_gpu_based/shaders",
            "tests/3_samples/6_gpu_based/shaders/pipelines",
            "include",
        },
        .debug_name = "Playground Compiler",
    });
    daxa::BinarySemaphore binary_semaphore = device.create_binary_semaphore({
        .debug_name = "Playground Present Semaphore",
    });
    static inline constexpr u64 FRAMES_IN_FLIGHT = 1;
    daxa::TimelineSemaphore gpu_framecount_timeline_sema = device.create_timeline_semaphore(daxa::TimelineSemaphoreInfo{
        .initial_value = 0,
        .debug_name = "Playground gpu framecount Timeline Semaphore",
    });
    u64 cpu_framecount = FRAMES_IN_FLIGHT - 1;
    Player3D player = {
        .rot = {2.0f, 0.0f, 0.0f},
    };
    u32 camera_index = 0;
    Clock::time_point start = Clock::now(), prev_time = start, now;
    float time, delta_time;
    bool should_resize = false, paused = true;

    DrawRasterPush draw_push;

    // clang-format off
    daxa::RasterPipeline draw_raster_pipeline = pipeline_compiler.create_raster_pipeline({
        .vertex_shader_info = {.source = daxa::ShaderFile{"temp_vert.hlsl"}},
        .fragment_shader_info = {.source = daxa::ShaderFile{"temp_frag.hlsl"}},
        #if VISUALIZE_OVERDRAW
        .color_attachments = {{.format = swapchain.get_format(), .blend = {.blend_enable = true, .src_color_blend_factor = daxa::BlendFactor::ONE, .dst_color_blend_factor = daxa::BlendFactor::ONE}}},
        #else
        .color_attachments = {{.format = swapchain.get_format(), .blend = {.blend_enable = true, .src_color_blend_factor = daxa::BlendFactor::SRC_ALPHA, .dst_color_blend_factor = daxa::BlendFactor::ONE_MINUS_SRC_ALPHA}}},
        #endif
        .depth_test = {
            .depth_attachment_format = daxa::Format::D32_SFLOAT,
        #if VISUALIZE_OVERDRAW
            .enable_depth_test = false,
            .enable_depth_write = false,
        #else
            .enable_depth_test = true,
            .enable_depth_write = true,
        #endif
        },
        .raster = {
            .face_culling = daxa::FaceCullFlagBits::BACK_BIT,
        },
        .push_constant_size = sizeof(DrawRasterPush),
        .debug_name = "Playground Pipeline",
    }).value();
    daxa::ComputePipeline chunkgen_compute_pipeline = pipeline_compiler.create_compute_pipeline({
        .shader_info = {.source = daxa::ShaderFile{"chunkgen.hlsl"}},
        .push_constant_size = sizeof(ChunkgenComputePush),
        .debug_name = "Playground Chunkgen Compute Pipeline",
    }).value();
    daxa::ComputePipeline meshgen_compute_pipeline = pipeline_compiler.create_compute_pipeline({
        .shader_info = {.source = daxa::ShaderFile{"meshgen.hlsl"}},
        .push_constant_size = sizeof(MeshgenComputePush),
        .debug_name = "Playground Meshgen Compute Pipeline",
    }).value();
    // daxa::ComputePipeline meshgen_compute_pipeline = pipeline_compiler.create_compute_pipeline({
    //     .shader_info = {.source = daxa::ShaderFile{"meshgen.hlsl"}},
    //     .push_constant_size = sizeof(MeshgenComputePush),
    //     .debug_name = "Playground Meshgen Compute Pipeline",
    // }).value();
    // clang-format on

    daxa::BufferId voxel_world_buffer = device.create_buffer(daxa::BufferInfo{
        .size = sizeof(u32) * CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * CHUNK_COUNT_X * CHUNK_COUNT_Y * CHUNK_COUNT_Z,
        .debug_name = "Voxel World buffer",
    });
    daxa::TaskBufferId task_voxel_world_buffer;

    daxa::BufferId globals_buffer = device.create_buffer(daxa::BufferInfo{
        .size = sizeof(SharedGlobals),
        .debug_name = "Globals buffer",
    });
    daxa::TaskBufferId task_globals_buffer;

    daxa::BufferId indirect_draw_buffer = device.create_buffer(daxa::BufferInfo{
        .size = sizeof(IndirectDrawBuffer),
        .debug_name = "Indirect Draw buffer",
    });
    daxa::TaskBufferId task_indirect_draw_buffer;

    daxa::TaskList loop_task_list = record_loop_task_list();

    App() : AppWindow<App>("Samples: Gpu Driven")
    {
    }

    ~App()
    {
        device.destroy_buffer(voxel_world_buffer);
        device.destroy_buffer(globals_buffer);
        device.destroy_buffer(indirect_draw_buffer);
        device.destroy_image(draw_depth_image);
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
            now = Clock::now();
            delta_time = std::chrono::duration<f32>(now - prev_time).count();
            time = std::chrono::duration<f32>(now - start).count();
            prev_time = now;
            on_update();
        }
        else
        {
            using namespace std::literals;
            std::this_thread::sleep_for(1ms);
        }

        return false;
    }

    void on_update()
    {
        player.update(delta_time);
        refresh_pipeline(pipeline_compiler, draw_raster_pipeline);
        if (refresh_pipeline(pipeline_compiler, chunkgen_compute_pipeline))
        {
            // invalidate chunk block data
        }
        if (refresh_pipeline(pipeline_compiler, meshgen_compute_pipeline))
        {
            // invalidate chunk mesh data
        }
        swapchain_image = swapchain.acquire_next_image();
        loop_task_list.execute();
        auto command_lists = loop_task_list.command_lists();
        auto cmd_list = device.create_command_list({});
        cmd_list.pipeline_barrier_image_transition({
            .awaited_pipeline_access = loop_task_list.last_access(task_swapchain_image),
            .before_layout = loop_task_list.last_layout(task_swapchain_image),
            .after_layout = daxa::ImageLayout::PRESENT_SRC,
            .image_id = swapchain_image,
        });
        cmd_list.complete();
        command_lists.push_back(cmd_list);
        device.submit_commands({
            .command_lists = command_lists,
            .signal_binary_semaphores = {binary_semaphore},
        });
        device.present_frame({
            .wait_binary_semaphores = {binary_semaphore},
            .swapchain = swapchain,
        });
    }

    auto record_loop_task_list() -> daxa::TaskList
    {
        daxa::TaskList new_task_list = daxa::TaskList({
            .device = device,
            .debug_name = "TaskList task list",
        });
        task_swapchain_image = new_task_list.create_task_image({
            .fetch_callback = [this]()
            { return swapchain_image; },
            .debug_name = "TaskList Task Swapchain Image",
        });
        task_draw_depth_image = new_task_list.create_task_image({
            .fetch_callback = [this]()
            { return draw_depth_image; },
            .slice = {.image_aspect = daxa::ImageAspectFlagBits::DEPTH},
            .debug_name = "TaskList Task Draw Depth Image",
        });
        task_globals_buffer = new_task_list.create_task_buffer({
            .fetch_callback = [this]()
            { return globals_buffer; },
            .debug_name = "TaskList Task Globals Buffer",
        });
        task_indirect_draw_buffer = new_task_list.create_task_buffer({
            .fetch_callback = [this]()
            { return indirect_draw_buffer; },
            .debug_name = "TaskList Task Indirect Draw Buffer",
        });

        // new_task_list.start_conditional_scope({
        //     // .conditional = [this]() -> bool
        //     // {
        //     //     return true;
        //     // },
        // });
        // new_task_list.add_task({
        //     .resources = {
        //         .buffers = {
        //             {task_globals_buffer, daxa::TaskBufferAccess::SHADER_READ_WRITE},
        //         },
        //     },
        //     .task = [this](daxa::TaskInterface interf)
        //     {
        //         auto cmd_list = interf.get_command_list();
        //         cmd_list.set_pipeline(chunkgen_compute_pipeline);
        //         // draw_push.globals_buffer_id = globals_buffer;
        //         // cmd_list.push_constant(draw_push);
        //         // cmd_list.draw({.vertex_count = 3});
        //     },
        //     .debug_name = "TaskList Chungen Task",
        // });
        // new_task_list.end_conditional_scope();

        new_task_list.add_task({
            .resources = {
                .buffers = {
                    {task_globals_buffer, daxa::TaskBufferAccess::SHADER_READ_ONLY},
                    {task_indirect_draw_buffer, daxa::TaskBufferAccess::SHADER_READ_ONLY},
                },
                .images = {
                    {task_swapchain_image, daxa::TaskImageAccess::COLOR_ATTACHMENT},
                    {task_draw_depth_image, daxa::TaskImageAccess::DEPTH_ATTACHMENT},
                },
            },
            .task = [this](daxa::TaskInterface interf)
            {
                auto cmd_list = interf.get_command_list();
                cmd_list.begin_renderpass({
                    .color_attachments = {{
                        .image_view = swapchain_image.default_view(),
                        .load_op = daxa::AttachmentLoadOp::CLEAR,
                        .clear_value = std::array<f32, 4>{0.2f, 0.4f, 1.0f, 1.0f},
                        // .clear_value = std::array<f32, 4>{0.0f, 0.0f, 0.0f, 1.0f},
                    }},
                    .depth_attachment = {{
                        .image_view = draw_depth_image.default_view(),
                        .load_op = daxa::AttachmentLoadOp::CLEAR,
                        .clear_value = daxa::DepthValue{1.0f, 0},
                    }},
                    .render_area = {.x = 0, .y = 0, .width = size_x, .height = size_y},
                });
                cmd_list.set_pipeline(draw_raster_pipeline);
                draw_push.globals_buffer_id = globals_buffer;
                cmd_list.push_constant(draw_push);
                cmd_list.draw({.vertex_count = 3});
                cmd_list.end_renderpass();
            },
            .debug_name = "TaskList Draw Task",
        });
        new_task_list.compile();
        return new_task_list;
    }

    void on_mouse_move(f32 x, f32 y)
    {
        if (!paused)
        {
            f32 center_x = static_cast<f32>(size_x / 2);
            f32 center_y = static_cast<f32>(size_y / 2);
            auto offset = glm::vec2{x - center_x, center_y - y};
            player.on_mouse_move(static_cast<f64>(offset.x), static_cast<f64>(offset.y));
            set_mouse_pos(center_x, center_y);
        }
    }

    void on_key(int key, int action)
    {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        {
            toggle_pause();
        }

        if (!paused)
        {
            player.on_key(key, action);
            if (key == GLFW_KEY_F5 && action == GLFW_PRESS)
            {
                toggle_view();
            }
        }
    }

    void on_resize(u32 sx, u32 sy)
    {
        size_x = sx;
        size_y = sy;
        minimized = (sx == 0 || sy == 0);

        if (!minimized)
        {
            loop_task_list = record_loop_task_list();
            device.destroy_image(draw_depth_image);
            draw_depth_image = create_depth();
            swapchain.resize(size_x, size_y);
            on_update();
        }
    }

    auto create_depth() -> daxa::ImageId
    {
        return device.create_image({
            .format = daxa::Format::D32_SFLOAT,
            .aspect = daxa::ImageAspectFlagBits::DEPTH,
            .size = {size_x, size_y, 1},
            .usage = daxa::ImageUsageFlagBits::DEPTH_STENCIL_ATTACHMENT,
        });
    }
    void toggle_pause()
    {
        set_mouse_capture(paused);
        paused = !paused;
    }
    void toggle_view()
    {
        camera_index = (camera_index + 1) % 4;
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
