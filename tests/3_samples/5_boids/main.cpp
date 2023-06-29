#include <0_common/window.hpp>
#include <thread>
#include <iostream>
#include <cmath>

#include <daxa/utils/pipeline_manager.hpp>
#include <daxa/utils/task_list.hpp>

#define APPNAME "Daxa Sample: Boids"
#define APPNAME_PREFIX(x) ("[" APPNAME "] " x)

using namespace daxa::types;
using Clock = std::chrono::high_resolution_clock;

#include "shared.inl"

namespace daxa
{
    using namespace task_resource_uses;
}

struct App : AppWindow<App>
{
    daxa::Context daxa_ctx = daxa::create_context({
        .enable_validation = false,
    });
    daxa::Device device = daxa_ctx.create_device({
        .name = APPNAME_PREFIX("device"),
    });

    daxa::Swapchain swapchain = device.create_swapchain({
        .native_window = get_native_handle(),
        .native_window_platform = get_native_platform(),
        .present_mode = daxa::PresentMode::FIFO,
        .image_usage = daxa::ImageUsageFlagBits::TRANSFER_DST,
        .name = APPNAME_PREFIX("swapchain"),
    });

    daxa::PipelineManager pipeline_manager = daxa::PipelineManager({
        .device = device,
        .shader_compile_options = {
            .root_paths = {
                DAXA_SHADER_INCLUDE_DIR,
                "tests/3_samples/5_boids/shaders",
                "tests/3_samples/5_boids",
            },
            .language = daxa::ShaderLanguage::GLSL,
        },
        .name = APPNAME_PREFIX("pipeline_manager"),
    });
    // clang-format off
    std::shared_ptr<daxa::RasterPipeline> draw_pipeline = pipeline_manager.add_raster_pipeline({
        .vertex_shader_info = daxa::ShaderCompileInfo{.source = daxa::ShaderFile{"vert.glsl"}},
        .fragment_shader_info = daxa::ShaderCompileInfo{.source = daxa::ShaderFile{"frag.glsl"}},
        .color_attachments = {{.format = swapchain.get_format()}},
        .raster = {},
        .push_constant_size = sizeof(DrawPushConstant),
        .name = APPNAME_PREFIX("draw_pipeline"),
    }).value();
    std::shared_ptr<daxa::ComputePipeline> update_boids_pipeline = pipeline_manager.add_compute_pipeline({
        .shader_info = {.source = daxa::ShaderFile{"update_boids.glsl"}},
        .push_constant_size = sizeof(UpdateBoidsPushConstant),
        .name = APPNAME_PREFIX("draw_pipeline"),
    }).value();
    // clang-format on

    u32 current_buffer_i = 1;

    f32 aspect = static_cast<f32>(size_x) / static_cast<f32>(size_y);

    using Clock = std::chrono::high_resolution_clock;
    Clock::time_point start = Clock::now();
    Clock::time_point prev_time = start;

    daxa::BufferId boid_buffer = device.create_buffer({
        .size = sizeof(Boids),
        .name = APPNAME_PREFIX("boids buffer a"),
    });

    daxa::BufferId old_boid_buffer = device.create_buffer({
        .size = sizeof(Boids),
        .name = APPNAME_PREFIX("boids buffer b"),
    });

    daxa::TaskImage task_swapchain_image{{.swapchain_image = true, .name = "swapchain image"}};
    daxa::TaskBuffer task_boids_current{{.initial_buffers = {.buffers = {&boid_buffer, 1}}, .name = "task_boids_current"}};
    daxa::TaskBuffer task_boids_old{{.initial_buffers = {.buffers = {&old_boid_buffer, 1}}, .name = "task_boids_old"}};

    daxa::CommandSubmitInfo submit_info;

    daxa::TaskList task_list = record_tasks();

    App() : AppWindow<App>(APPNAME)
    {
        auto cmd_list = device.create_command_list({.name = APPNAME_PREFIX("boid buffer init commands")});

        auto upload_buffer_id = device.create_buffer({
            .size = sizeof(Boids),
            .allocate_info = daxa::MemoryFlagBits::HOST_ACCESS_SEQUENTIAL_WRITE,
            .name = APPNAME_PREFIX("voids buffer init staging buffer"),
        });
        cmd_list.destroy_buffer_deferred(upload_buffer_id);

        auto * ptr = device.get_host_address_as<Boids>(upload_buffer_id);

        for (auto & boid : ptr->boids)
        {
            boid.position.x = static_cast<f32>(rand() % ((FIELD_SIZE)*100)) / 100.0f;
            boid.position.y = static_cast<f32>(rand() % ((FIELD_SIZE)*100)) / 100.0f;
            f32 const angle = static_cast<f32>(rand() % 3600) * 0.1f;
            boid.speed.x = std::cos(angle);
            boid.speed.y = std::sin(angle);
        }

        cmd_list.copy_buffer_to_buffer({
            .src_buffer = upload_buffer_id,
            .dst_buffer = boid_buffer,
            .size = sizeof(Boids),
        });

        cmd_list.copy_buffer_to_buffer({
            .src_buffer = upload_buffer_id,
            .dst_buffer = old_boid_buffer,
            .size = sizeof(Boids),
        });

        cmd_list.pipeline_barrier({
            .src_access = daxa::AccessConsts::TRANSFER_WRITE,
            .dst_access = daxa::AccessConsts::COMPUTE_SHADER_READ_WRITE | daxa::AccessConsts::VERTEX_SHADER_READ,
        });
        cmd_list.complete();
        device.submit_commands({
            .command_lists = {cmd_list},
        });
    }

    ~App()
    {
        device.wait_idle();
        device.collect_garbage();
        device.destroy_buffer(boid_buffer);
        device.destroy_buffer(old_boid_buffer);
    }

    auto update() -> bool
    {
        glfwPollEvents();
        if (glfwWindowShouldClose(glfw_window_ptr) != 0)
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

    // Update task:

    struct UpdateBoidsTask
    {
        struct Uses
        {
            daxa::BufferComputeShaderReadWrite current{};
            daxa::BufferComputeShaderRead previous{};
        } uses = {};
        std::string_view name = "update boids";

        std::shared_ptr<daxa::ComputePipeline> update_boids_pipeline = {};
        void callback(daxa::TaskInterface ti)
        {
            auto cmd_list = ti.get_command_list();
            cmd_list.set_pipeline(*update_boids_pipeline);

            cmd_list.push_constant(UpdateBoidsPushConstant{
                .boids_buffer = ti.get_device().get_device_address(uses.current.buffer()),
                .old_boids_buffer = ti.get_device().get_device_address(uses.previous.buffer()),
            });

            cmd_list.dispatch((MAX_BOIDS + 63) / 64, 1, 1);
        }
    };

    // Draw task:

    struct DrawBoidsTask
    {
        struct Uses
        {
            daxa::BufferVertexShaderRead boids{};
            daxa::ImageColorAttachment<> render_image{};
        } uses = {};
        std::string_view name = "draw boids";

        std::shared_ptr<daxa::RasterPipeline> draw_pipeline = {};
        u32 * size_x = {};
        u32 * size_y = {};
        void callback(daxa::TaskInterface ti)
        {
            auto cmd_list = ti.get_command_list();
            cmd_list.set_pipeline(*draw_pipeline);
            cmd_list.begin_renderpass({
                .color_attachments = {
                    {
                        .image_view = uses.render_image.view(),
                        .layout = daxa::ImageLayout::ATTACHMENT_OPTIMAL,
                        .load_op = daxa::AttachmentLoadOp::CLEAR,
                        .store_op = daxa::AttachmentStoreOp::STORE,
                        .clear_value = std::array<f32, 4>{1.0f, 1.0f, 1.0f, 1.0f},
                    },
                },
                .render_area = {
                    .width = *size_x,
                    .height = *size_y,
                },
            });

            cmd_list.push_constant(DrawPushConstant{
                .boids_buffer = ti.get_device().get_device_address(uses.boids.buffer()),
                .axis_scaling = {
                    std::min(1.0f, static_cast<f32>(*this->size_y) / static_cast<f32>(*this->size_x)),
                    std::min(1.0f, static_cast<f32>(*this->size_x) / static_cast<f32>(*this->size_y)),
                },
            });

            cmd_list.draw({.vertex_count = 3 * MAX_BOIDS});

            cmd_list.end_renderpass();
        }
    };

    auto record_tasks() -> daxa::TaskList
    {
        daxa::TaskList new_task_list = daxa::TaskList({.device = device, .swapchain = swapchain, .name = APPNAME_PREFIX("main task list")});
        new_task_list.use_persistent_image(task_swapchain_image);
        new_task_list.use_persistent_buffer(task_boids_current);
        new_task_list.use_persistent_buffer(task_boids_old);

        new_task_list.add_task(UpdateBoidsTask{
            .uses = {
                .current = {task_boids_current},
                .previous = {task_boids_old},
            },
            .update_boids_pipeline = update_boids_pipeline,
        });

        new_task_list.add_task(DrawBoidsTask{
            .uses = {
                .boids = {task_boids_current},
                .render_image = {task_swapchain_image},
            },
            .draw_pipeline = draw_pipeline,
            .size_x = &size_x,
            .size_y = &size_y,
        });

        new_task_list.submit({});
        new_task_list.present({});
        new_task_list.complete({});

        return new_task_list;
    }

    void draw()
    {
        auto now = Clock::now();
        prev_time = now;

        auto reloaded_result = pipeline_manager.reload_all();
        if (reloaded_result.has_value())
        {
            std::cout << reloaded_result.value().to_string() << std::endl;
        }

        auto swapchain_image = swapchain.acquire_next_image();
        task_swapchain_image.set_images({.images = {&swapchain_image, 1}});
        if (swapchain_image.is_empty())
        {
            return;
        }
        task_list.execute({});
        // Switch boids front and back buffers.
        task_boids_current.swap_buffers(task_boids_old);
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
            aspect = static_cast<f32>(size_x) / static_cast<f32>(size_y);
            draw();
        }
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
