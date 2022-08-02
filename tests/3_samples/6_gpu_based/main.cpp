#include <0_common/window.hpp>
#include <0_common/player.hpp>
#include <thread>
#include <iostream>
#include <daxa/utils/task_list.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "util.hpp"

using namespace daxa::types;
using Clock = std::chrono::high_resolution_clock;

#include "config.inl"

// helper utils

auto refresh_pipeline(daxa::PipelineCompiler & compiler, auto & pipeline) -> bool
{
    if (compiler.check_if_sources_changed(pipeline))
    {
        auto new_pipeline = compiler.recreate_compute_pipeline(pipeline);
        if (new_pipeline.is_ok())
        {
            pipeline = new_pipeline.value();
            return true;
        }
        else
        {
            std::cout << new_pipeline.message() << std::endl;
        }
        return false;
    }
}

// end helper utils

struct DrawRasterPush
{
    glm::vec3 chunk_pos;
};
struct ChunkgenComputePush
{
    glm::vec3 chunk_pos;
    daxa::BufferId buffer_id;
};
struct MeshgenComputePush
{
    glm::vec3 chunk_pos;
    daxa::BufferId buffer_id;
};

struct App : AppWindow<App>
{
    daxa::Context daxa_ctx = daxa::create_context({.enable_validation = true});
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
        .debug_name = "Playground Swapchain",
    });
    daxa::ImageId depth_image = create_depth_image();
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

    // clang-format off
    daxa::RasterPipeline draw_raster_pipeline = pipeline_compiler.create_raster_pipeline({
        .vertex_shader_info = {.source = daxa::ShaderFile{"draw_vert.hlsl"}},
        .fragment_shader_info = {.source = daxa::ShaderFile{"draw_frag.hlsl"}},
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
    // clang-format on

    daxa::TaskList task_list = record_task_list();

    daxa::BufferId voxel_world_buffer = device.create_buffer(daxa::BufferInfo{
        .size = sizeof(u32) * CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * CHUNK_COUNT_X * CHUNK_COUNT_Y * CHUNK_COUNT_Z,
        .debug_name = "Voxel World buffer",
    });
    daxa::TaskBufferId task_voxel_world_buffer;

    App() : AppWindow<App>("Samples: Gpu Driven")
    {
    }

    ~App()
    {
        device.destroy_buffer(voxel_world_buffer);
        device.destroy_image(depth_image);
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
    }

    auto record_task_list() -> daxa::TaskList
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
        task_render_image = new_task_list.create_task_image({
            .fetch_callback = [this]()
            { return render_image; },
            .debug_name = "TaskList Task Render Image",
        });

        new_task_list.add_clear_image({
            .clear_value = {std::array<f32, 4>{1, 0, 1, 1}},
            .dst_image = task_render_image,
            .dst_slice = {},
            .debug_name = "TaskList Clear Render Image Task",
        });
        new_task_list.add_task({
            .resources = {
                .images = {
                    {task_render_image, daxa::TaskImageAccess::FRAGMENT_SHADER_WRITE_ONLY},
                },
            },
            .task = [this](daxa::TaskInterface interf)
            {
                auto cmd_list = interf.get_command_list();
                cmd_list.begin_renderpass({
                    .color_attachments = {{.image_view = render_image.default_view()}},
                    .render_area = {.x = 0, .y = 0, .width = size_x, .height = size_y},
                });
                cmd_list.set_pipeline(raster_pipeline);
                cmd_list.draw({.vertex_count = 3});
                cmd_list.end_renderpass();
            },
            .debug_name = "TaskList Draw to Render Image Task",
        });

        new_task_list.add_copy_image_to_image({
            .src_image = task_render_image,
            .dst_image = task_swapchain_image,
            .extent = {size_x, size_y, 1},
        });
        new_task_list.compile();

        return new_task_list;
    }

    void on_resize(u32 sx, u32 sy)
    {
        size_x = sx;
        size_y = sy;
        minimized = (sx == 0 || sy == 0);

        if (!minimized)
        {
            task_list = record_task_list();
            device.destroy_image(depth_image);
            depth_image = create_depth_image();
            swapchain.resize(size_x, size_y);
            on_update();
        }
    }

    auto create_depth_image() -> daxa::ImageId
    {
        return device.create_image({
            .format = daxa::Format::D32_SFLOAT,
            .aspect = daxa::ImageAspectFlagBits::DEPTH,
            .size = {size_x, size_y, 1},
            .usage = daxa::ImageUsageFlagBits::DEPTH_STENCIL_ATTACHMENT,
        });
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
