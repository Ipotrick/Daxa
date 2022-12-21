#include "common.hpp"
#include "mipmapping.hpp"

namespace tests
{
    void simplest()
    {
        AppContext const app = {};
        auto task_list = daxa::TaskList({
            .device = app.device,
            .debug_name = APPNAME_PREFIX("task_list (simplest)"),
        });
    }

    void execution()
    {
        AppContext const app = {};
        auto task_list = daxa::TaskList({
            .device = app.device,
            .debug_name = APPNAME_PREFIX("task_list (execution)"),
        });

        // This is pointless, but done to show how the task list executes
        task_list.add_task({
            .task = [&](daxa::TaskRuntime const &)
            {
                std::cout << "Hello, ";
            },
            .debug_name = APPNAME_PREFIX("task 1 (execution)"),
        });
        task_list.add_task({
            .task = [&](daxa::TaskRuntime const &)
            {
                std::cout << "World!" << std::endl;
            },
            .debug_name = APPNAME_PREFIX("task 2 (execution)"),
        });

        task_list.complete();

        task_list.execute();
    }

    void write_read_image()
    {
        // TEST:
        //    1) CREATE image
        //    2) WRITE image
        //    3) READ image
        AppContext app = {};
        auto image = app.device.create_image({
            .size = {1, 1, 1},
            .array_layer_count = 2,
            .usage = daxa::ImageUsageFlagBits::SHADER_READ_WRITE,
            .debug_name = APPNAME_PREFIX("image (create image)"),
        });
        auto task_list = daxa::TaskList({.device = app.device,
                                         .debug_name = APPNAME_PREFIX("task_list (create-write-read image)")});
        // CREATE IMAGE
        auto task_image = task_list.create_task_image({
            .initial_access = {daxa::PipelineStageFlagBits::ALL_GRAPHICS, daxa::AccessTypeFlagBits::WRITE},
            .initial_layout = daxa::ImageLayout::GENERAL,
            .debug_name = "task list image",
        });
        task_list.add_runtime_image(task_image, image);
        // WRITE IMAGE 1
        task_list.add_task({
            .used_buffers = {},
            .used_images = {
                {task_image, daxa::TaskImageAccess::SHADER_WRITE_ONLY, daxa::ImageMipArraySlice{}},
            },
            .task = [](daxa::TaskRuntime const &) {},
            .debug_name = APPNAME_PREFIX("write image 1"),
        });
        // READ_IMAGE 1
        task_list.add_task({
            .used_buffers = {},
            .used_images = {
                {task_image, daxa::TaskImageAccess::SHADER_READ_ONLY, daxa::ImageMipArraySlice{}},
            },
            .task = [](daxa::TaskRuntime const &) {},
            .debug_name = APPNAME_PREFIX("read image 1"),
        });
        // READ_IMAGE 2
        task_list.add_task({
            .used_buffers = {},
            .used_images = {
                {task_image, daxa::TaskImageAccess::SHADER_READ_ONLY, daxa::ImageMipArraySlice{}},
            },
            .task = [](daxa::TaskRuntime const &) {},
            .debug_name = APPNAME_PREFIX("read image 2"),
        });
        // WRITE_IMAGE 2
        task_list.add_task({
            .used_buffers = {},
            .used_images = {
                {
                    task_image,
                    daxa::TaskImageAccess::SHADER_READ_WRITE,
                    daxa::ImageMipArraySlice{
                        .image_aspect = daxa::ImageAspectFlagBits::COLOR,
                        .level_count = 1,
                        .layer_count = 2,
                    },
                },
            },
            .task = [](daxa::TaskRuntime const &) {},
            .debug_name = APPNAME_PREFIX("write image 2"),
        });
        task_list.complete();
        task_list.debug_print();
        // task_list.execute();
        app.device.destroy_image(image);
    }

    void image_upload()
    {
        AppContext app = {};
        auto task_list = daxa::TaskList({
            .device = app.device,
            .debug_name = APPNAME_PREFIX("task_list (image_upload)"),
        });

        auto buffer = app.device.create_buffer({
            .size = 4,
            .debug_name = APPNAME_PREFIX("buffer (image_upload)"),
        });
        auto image = app.device.create_image({
            .size = {1, 1, 1},
            .usage = daxa::ImageUsageFlagBits::TRANSFER_DST | daxa::ImageUsageFlagBits::SHADER_READ_WRITE,
            .debug_name = APPNAME_PREFIX("image (image_upload)"),
        });

        auto task_image = task_list.create_task_image({});
        task_list.add_runtime_image(task_image, image);
        auto upload_buffer = task_list.create_task_buffer({});
        task_list.add_runtime_buffer(upload_buffer, buffer);

        task_list.add_task({
            .used_buffers = {{upload_buffer, daxa::TaskBufferAccess::TRANSFER_READ}},
            .used_images = {{task_image, daxa::TaskImageAccess::TRANSFER_WRITE, daxa::ImageMipArraySlice{}}},
            .task = [](daxa::TaskRuntime const &)
            {
                // TODO: Implement this task!
            },
            .debug_name = APPNAME_PREFIX("upload task (image_upload)"),
        });

        task_list.complete();

        task_list.execute();

        app.device.destroy_buffer(buffer);
        app.device.destroy_image(image);
    }

    void output_graph()
    {
        AppContext const app = {};
        auto task_list = daxa::TaskList({
            .device = app.device,
            .debug_name = APPNAME_PREFIX("task_list (output_graph)"),
        });

        auto task_buffer1 = task_list.create_task_buffer({
            .initial_access = {daxa::PipelineStageFlagBits::HOST, daxa::AccessTypeFlagBits::WRITE},
            .debug_name = APPNAME_PREFIX("task_buffer1"),
        });
        auto task_buffer2 = task_list.create_task_buffer({
            .initial_access = {daxa::PipelineStageFlagBits::HOST, daxa::AccessTypeFlagBits::WRITE},
            .debug_name = APPNAME_PREFIX("task_buffer2"),
        });
        auto task_buffer3 = task_list.create_task_buffer({
            .initial_access = {daxa::PipelineStageFlagBits::HOST, daxa::AccessTypeFlagBits::WRITE},
            .debug_name = APPNAME_PREFIX("task_buffer3"),
        });

        auto task_image1 = task_list.create_task_image({
            .initial_access = {daxa::PipelineStageFlagBits::HOST, daxa::AccessTypeFlagBits::WRITE},
            .debug_name = APPNAME_PREFIX("task_image1"),
        });
        auto task_image2 = task_list.create_task_image({
            .initial_access = {daxa::PipelineStageFlagBits::HOST, daxa::AccessTypeFlagBits::WRITE},
            .debug_name = APPNAME_PREFIX("task_image2"),
        });
        auto task_image3 = task_list.create_task_image({
            .initial_access = {daxa::PipelineStageFlagBits::HOST, daxa::AccessTypeFlagBits::WRITE},
            .debug_name = APPNAME_PREFIX("task_image3"),
        });

        task_list.add_task({
            .used_buffers = {
                {task_buffer1, daxa::TaskBufferAccess::SHADER_WRITE_ONLY},
                {task_buffer2, daxa::TaskBufferAccess::SHADER_READ_ONLY},
            },
            .used_images = {
                {task_image1, daxa::TaskImageAccess::SHADER_WRITE_ONLY, daxa::ImageMipArraySlice{}},
                {task_image2, daxa::TaskImageAccess::SHADER_READ_ONLY, daxa::ImageMipArraySlice{}},
            },
            .task = [](daxa::TaskRuntime const &) {},
            .debug_name = APPNAME_PREFIX("task 1 (output_graph)"),
        });

        task_list.add_task({
            .used_buffers = {
                {task_buffer2, daxa::TaskBufferAccess::SHADER_WRITE_ONLY},
            },
            .used_images = {
                {task_image2, daxa::TaskImageAccess::SHADER_WRITE_ONLY, daxa::ImageMipArraySlice{}},
            },
            .task = [](daxa::TaskRuntime const &) {},
            .debug_name = APPNAME_PREFIX("task 2 (output_graph)"),
        });
        task_list.add_task({
            .used_buffers = {
                {task_buffer3, daxa::TaskBufferAccess::SHADER_WRITE_ONLY},
            },
            .used_images = {
                {task_image3, daxa::TaskImageAccess::SHADER_WRITE_ONLY, daxa::ImageMipArraySlice{}},
            },
            .task = [](daxa::TaskRuntime const &) {},
            .debug_name = APPNAME_PREFIX("task 3 (output_graph)"),
        });

        task_list.add_task({
            .used_buffers = {
                {task_buffer2, daxa::TaskBufferAccess::SHADER_WRITE_ONLY},
                {task_buffer3, daxa::TaskBufferAccess::SHADER_WRITE_ONLY},
            },
            .used_images = {
                {
                    task_image2,
                    daxa::TaskImageAccess::SHADER_WRITE_ONLY,
                    daxa::ImageMipArraySlice{
                        .base_mip_level = 0,
                        .level_count = 3,
                        .base_array_layer = 2,
                        .layer_count = 2,
                    },
                },
                {
                    task_image3,
                    daxa::TaskImageAccess::SHADER_WRITE_ONLY,
                    daxa::ImageMipArraySlice{
                        .base_mip_level = 0,
                        .level_count = 3,
                        .base_array_layer = 2,
                        .layer_count = 2,
                    },
                },
            },
            .task = [](daxa::TaskRuntime const &) {},
            .debug_name = APPNAME_PREFIX("task 4 (output_graph)"),
        });

        task_list.add_task({
            .used_buffers = {
                {task_buffer3, daxa::TaskBufferAccess::SHADER_READ_ONLY},
            },
            .used_images = {
                {task_image3, daxa::TaskImageAccess::SHADER_READ_ONLY, daxa::ImageMipArraySlice{}},
            },
            .task = [](daxa::TaskRuntime const &) {},
            .debug_name = APPNAME_PREFIX("task 5 (output_graph)"),
        });

        task_list.complete();
        task_list.debug_print();
        task_list.output_graphviz();
    }

    void drawing()
    {
        struct App : AppWindow<App>
        {
            daxa::Context daxa_ctx = daxa::create_context({
                .enable_validation = true,
            });
            daxa::Device device = daxa_ctx.create_device({
                .debug_name = APPNAME_PREFIX("device (drawing)"),
            });

            daxa::Swapchain swapchain = device.create_swapchain({
                .native_window = get_native_handle(),
                .native_window_platform = get_native_platform(),
                .image_usage = daxa::ImageUsageFlagBits::TRANSFER_DST,
                .debug_name = APPNAME_PREFIX("swapchain (drawing)"),
            });

            daxa::PipelineManager pipeline_manager = daxa::PipelineManager({
                .device = device,
                .shader_compile_options = {
                    .root_paths = {
                        "tests/2_daxa_api/6_task_list/shaders",
                        "include",
                    },
                },
                .debug_name = APPNAME_PREFIX("pipeline_manager (drawing)"),
            });
            // clang-format off
            std::shared_ptr<daxa::RasterPipeline> raster_pipeline = pipeline_manager.add_raster_pipeline({
                .vertex_shader_info = {.source = daxa::ShaderFile{"vert.hlsl"}},
                .fragment_shader_info = {.source = daxa::ShaderFile{"frag.hlsl"}},
                .color_attachments = {{.format = swapchain.get_format()}},
                .raster = {},
                .debug_name = APPNAME_PREFIX("raster_pipeline (drawing)"),
            }).value();
            // clang-format on

            daxa::TaskList task_list = record_task_list();

            daxa::ImageId render_image = create_render_image(size_x, size_y);
            daxa::TaskImageId task_render_image;

            daxa::ImageId swapchain_image{};
            daxa::TaskImageId task_swapchain_image;

            App() : AppWindow<App>("Daxa API: Swapchain (clearcolor)")
            {
                record_task_list();
            }

            auto record_task_list() -> daxa::TaskList
            {
                daxa::TaskList new_task_list = daxa::TaskList({
                    .device = device,
                    .use_split_barriers = false,
                    .debug_name = APPNAME_PREFIX("task_list (drawing)"),
                });
                // task_swapchain_image = new_task_list.create_task_image({
                //     .image = &swapchain_image,
                //     .swapchain_parent = {swapchain, present_semaphore},
                // });
                task_render_image = new_task_list.create_task_image({});
                new_task_list.add_runtime_image(task_render_image, render_image);

                // new_task_list.add_clear_image({
                //     .clear_value = {std::array<f32, 4>{1, 0, 1, 1}},
                //     .dst_image = task_render_image,
                //     .dst_slice = {},
                //     .debug_name = APPNAME_PREFIX("Clear render_image Task (drawing)"),
                // });
                new_task_list.add_task({
                    .used_images = {
                        {task_render_image, daxa::TaskImageAccess::FRAGMENT_SHADER_WRITE_ONLY, daxa::ImageMipArraySlice{}},
                    },
                    .task = [this](daxa::TaskRuntime runtime)
                    {
                        auto cmd_list = runtime.get_command_list();
                        cmd_list.begin_renderpass({
                            .color_attachments = {{.image_view = render_image.default_view()}},
                            .render_area = {.x = 0, .y = 0, .width = size_x, .height = size_y},
                        });
                        cmd_list.set_pipeline(*raster_pipeline);
                        cmd_list.draw({.vertex_count = 3});
                        cmd_list.end_renderpass();
                    },
                    .debug_name = APPNAME_PREFIX("Draw to render_image Task (drawing)"),
                });

                // new_task_list.add_copy_image_to_image({
                //     .src_image = task_render_image,
                //     .dst_image = task_swapchain_image,
                //     .extent = {size_x, size_y, 1},
                // });
                new_task_list.complete();

                return new_task_list;
            }

            ~App()
            {
                device.wait_idle();
                device.collect_garbage();
                device.destroy_image(render_image);
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

            void draw()
            {
                // if (pipeline_manager.check_if_sources_changed(raster_pipeline))
                // {
                //     auto new_pipeline = pipeline_manager.recreate_raster_pipeline(raster_pipeline);
                //     std::cout << new_pipeline.to_string() << std::endl;
                //     if (new_pipeline.is_ok())
                //     {
                //         raster_pipeline = new_pipeline.value();
                //     }
                // }
                pipeline_manager.reload_all();

                task_list.remove_runtime_image(task_swapchain_image, swapchain_image);
                swapchain_image = swapchain.acquire_next_image();
                task_list.add_runtime_image(task_swapchain_image, swapchain_image);

                if (swapchain_image.is_empty())
                {
                    return;
                }
                task_list.execute();
                auto command_lists = task_list.get_command_lists();
                auto cmd_list = device.create_command_list({});
                auto last_si_use = task_list.last_uses(task_swapchain_image).back();
                cmd_list.pipeline_barrier_image_transition({
                    .awaited_pipeline_access = last_si_use.access,
                    .before_layout = last_si_use.layout,
                    .after_layout = daxa::ImageLayout::PRESENT_SRC,
                    .image_id = swapchain_image,
                });
                cmd_list.complete();
                command_lists.push_back(cmd_list);
                device.submit_commands({
                    .command_lists = command_lists,
                    .wait_binary_semaphores = {swapchain.get_acquire_semaphore()},
                    .signal_binary_semaphores = {swapchain.get_present_semaphore()},
                    .signal_timeline_semaphores = {
                        {swapchain.get_gpu_timeline_semaphore(), swapchain.get_cpu_timeline_value()}},
                });
                device.present_frame({
                    .wait_binary_semaphores = {swapchain.get_present_semaphore()},
                    .swapchain = swapchain,
                });
            }

            void on_mouse_move(f32 /*unused*/, f32 /*unused*/) {}
            void on_mouse_button(i32 /*unused*/, i32 /*unused*/) {}
            void on_key(i32 /*unused*/, i32 /*unused*/) {}

            auto create_render_image(u32 sx, u32 sy) -> daxa::ImageId
            {
                return device.create_image(daxa::ImageInfo{
                    .format = swapchain.get_format(),
                    .size = {sx, sy, 1},
                    .usage = daxa::ImageUsageFlagBits::COLOR_ATTACHMENT | daxa::ImageUsageFlagBits::TRANSFER_SRC | daxa::ImageUsageFlagBits::TRANSFER_DST,
                    .debug_name = APPNAME_PREFIX("render_image (drawing)"),
                });
            }

            void on_resize(u32 sx, u32 sy)
            {
                minimized = (sx == 0 || sy == 0);
                if (!minimized)
                {
                    swapchain.resize();
                    size_x = swapchain.get_surface_extent().x;
                    size_y = swapchain.get_surface_extent().y;
                    task_list = record_task_list();
                    device.destroy_image(render_image);
                    render_image = create_render_image(sx, sy);
                    draw();
                }
            }
        };

        App app = {};
        while (true)
        {
            if (app.update())
            {
                break;
            }
        }
    }
} // namespace tests

auto main() -> int
{
    // tests::write_read_image();
    //  tests::simplest();
    //  tests::image_upload();
    //  tests::execution();
    // tests::output_graph();
    tests::mipmapping();
    // tests::drawing();
}
