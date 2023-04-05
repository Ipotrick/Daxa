#include "common.hpp"
#include "mipmapping.hpp"

#include "shaders/shader_integration.inl"

namespace tests
{
    void simplest()
    {
        AppContext const app = {};
        auto task_list = daxa::TaskList({
            .device = app.device,
            .name = APPNAME_PREFIX("task_list (simplest)"),
        });
    }

    void execution()
    {
        AppContext const app = {};
        auto task_list = daxa::TaskList({
            .device = app.device,
            .name = APPNAME_PREFIX("task_list (execution)"),
        });

        // This is pointless, but done to show how the task list executes
        task_list.add_task({
            .task = [&](daxa::TaskRuntimeInterface const &)
            {
                std::cout << "Hello, ";
            },
            .name = APPNAME_PREFIX("task 1 (execution)"),
        });
        task_list.add_task({
            .task = [&](daxa::TaskRuntimeInterface const &)
            {
                std::cout << "World!" << std::endl;
            },
            .name = APPNAME_PREFIX("task 2 (execution)"),
        });

        task_list.complete({});

        task_list.execute({});
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
            .name = APPNAME_PREFIX("tested image"),
        });
        auto task_list = daxa::TaskList({
            .device = app.device,
            .name = APPNAME_PREFIX("create-write-read image"),
        });
        // CREATE IMAGE
        auto task_image = task_list.create_task_image({.name = "task list tested image"});
        task_list.add_runtime_image(task_image, image);
        // WRITE IMAGE 1
        task_list.add_task({
            .used_buffers = {},
            .used_images =
                {
                    {task_image,
                     daxa::TaskImageAccess::COMPUTE_SHADER_WRITE_ONLY,
                     daxa::ImageMipArraySlice{}},
                },
            .task = [](daxa::TaskRuntimeInterface const &) {},
            .name = APPNAME_PREFIX("write image 1"),
        });
        // READ_IMAGE 1
        task_list.add_task({
            .used_buffers = {},
            .used_images =
                {
                    {task_image,
                     daxa::TaskImageAccess::COMPUTE_SHADER_READ_ONLY,
                     daxa::ImageMipArraySlice{}},
                },
            .task = [](daxa::TaskRuntimeInterface const &) {},
            .name = APPNAME_PREFIX("read image 1"),
        });

        task_list.complete({});
        // task_list.execute();
        app.device.destroy_image(image);
    }

    void write_read_image_layer()
    {
        // TEST:
        //    1) CREATE image
        //    2) WRITE into array layer 1 of the image
        //    3) READ from array layer 2 of the image
        AppContext app = {};
        auto image = app.device.create_image({
            .size = {1, 1, 1},
            .array_layer_count = 2,
            .usage = daxa::ImageUsageFlagBits::SHADER_READ_WRITE,
            .name = APPNAME_PREFIX("tested image"),
        });
        auto task_list = daxa::TaskList({
            .device = app.device,
            .name = APPNAME_PREFIX("create-write-read array layer"),
        });
        // CREATE IMAGE
        auto task_image = task_list.create_task_image({
            .name = "task list tested image",
        });
        task_list.add_task({
            .used_buffers = {},
            .used_images =
                {
                    {task_image,
                     daxa::TaskImageAccess::COMPUTE_SHADER_WRITE_ONLY,
                     daxa::ImageMipArraySlice{.base_array_layer = 0, .layer_count = 1}}},
            .task = [](daxa::TaskRuntimeInterface const &) {},
            .name = APPNAME_PREFIX("write image array layer 1"),
        });
        // READ_IMAGE 1
        task_list.add_task({
            .used_buffers = {},
            .used_images =
                {
                    {task_image,
                     daxa::TaskImageAccess::COMPUTE_SHADER_READ_ONLY,
                     daxa::ImageMipArraySlice{.base_array_layer = 1, .layer_count = 1}}},
            .task = [](daxa::TaskRuntimeInterface const &) {},
            .name = APPNAME_PREFIX("read image array layer 1"),
        });
        task_list.complete({});
        // task_list.execute();
        app.device.destroy_image(image);
    }

    void create_transfer_read_buffer()
    {
        // TEST:
        //    1) CREATE buffer
        //    2) TRANSFER into the image
        //    3) READ from the image
        AppContext app = {};
        auto buffer = app.device.create_buffer({.size = sizeof(u32),
                                                .name = "tested buffer"});
        auto task_list = daxa::TaskList({
            .device = app.device,
            .name = APPNAME_PREFIX("create-transfer-read buffer"),
        });
        // CREATE IMAGE
        auto task_buffer = task_list.create_task_buffer({
            .pre_task_list_slice_states = daxa::AccessConsts::NONE,
            .name = "task list tested buffer",
        });
        task_list.add_task({
            .used_buffers =
                {
                    {task_buffer,
                     daxa::TaskBufferAccess::HOST_TRANSFER_WRITE},
                },
            .used_images = {},
            .task = [](daxa::TaskRuntimeInterface const &) {},
            .name = APPNAME_PREFIX("host transfer buffer"),
        });
        // READ_IMAGE 1
        task_list.add_task({
            .used_buffers =
                {
                    {
                        task_buffer,
                        daxa::TaskBufferAccess::COMPUTE_SHADER_READ_ONLY,
                    },
                },
            .used_images = {},
            .task = [](daxa::TaskRuntimeInterface const &) {},
            .name = APPNAME_PREFIX("read image array layer 1"),
        });
        task_list.complete({});
        // task_list.execute();
        app.device.destroy_buffer(buffer);
    }

    void initial_layout_access()
    {
        // TEST:
        //    1) CREATE image - set the task image initial access to write from compute shader
        //    2) READ from a the subimage
        //    3) WRITE into the subimage
        AppContext app = {};
        auto image = app.device.create_image({
            .size = {1, 1, 1},
            .array_layer_count = 2,
            .usage = daxa::ImageUsageFlagBits::SHADER_READ_WRITE,
            .name = APPNAME_PREFIX("tested image"),
        });
        auto task_list = daxa::TaskList({
            .device = app.device,
            .name = APPNAME_PREFIX("initial layout image"),
        });
        // CREATE IMAGE
        std::array init_access = {
            daxa::ImageSliceState{
                .latest_access = daxa::AccessConsts::COMPUTE_SHADER_WRITE,
                .latest_layout = daxa::ImageLayout::GENERAL}};
        auto task_image = task_list.create_task_image(
            {
                .pre_task_list_slice_states = {init_access.begin(), init_access.end()},
                .name = "task list tested image",
            });
        task_list.add_runtime_image(task_image, image);

        task_list.add_task({
            .used_buffers = {},
            .used_images =
                {
                    {task_image,
                     daxa::TaskImageAccess::COMPUTE_SHADER_READ_ONLY,
                     daxa::ImageMipArraySlice{.base_array_layer = 1, .layer_count = 1}},
                },
            .task = [](daxa::TaskRuntimeInterface const &) {},
            .name = APPNAME_PREFIX("read array layer 2"),
        });

        task_list.add_task({
            .used_buffers = {},
            .used_images =
                {
                    {task_image,
                     daxa::TaskImageAccess::COMPUTE_SHADER_WRITE_ONLY,
                     daxa::ImageMipArraySlice{.base_array_layer = 0, .layer_count = 1}},
                },
            .task = [](daxa::TaskRuntimeInterface const &) {},
            .name = APPNAME_PREFIX("write array layer 1"),
        });

        task_list.complete({});
        // task_list.execute();
        app.device.destroy_image(image);
    }

    void tracked_slice_barrier_collapsing()
    {
        // TEST:
        //    1) CREATE image - set the task image initial access to write from compute
        //                      shader for one subresouce and read for the other
        //    2) WRITE into the subsubimage
        //    3) READ from a the subsubimage
        //    4) WRITE into the entire image
        //    5) READ the entire image
        //    Expected: There should only be a single barrier between tests 4 and 5.
        AppContext app = {};
        auto image = app.device.create_image({
            .size = {1, 1, 1},
            .array_layer_count = 4,
            .usage = daxa::ImageUsageFlagBits::SHADER_READ_WRITE,
            .name = APPNAME_PREFIX("tested image"),
        });
        auto task_list = daxa::TaskList({
            .device = app.device,
            .record_debug_information = true,
            .name = APPNAME_PREFIX("tracked slice barrier collapsing"),
        });
        // CREATE IMAGE
        std::array init_access = {
            daxa::ImageSliceState{
                .latest_access = daxa::AccessConsts::COMPUTE_SHADER_WRITE,
                .latest_layout = daxa::ImageLayout::GENERAL,
                .slice = {.base_array_layer = 0, .layer_count = 2},
            },
            daxa::ImageSliceState{
                .latest_access = daxa::AccessConsts::COMPUTE_SHADER_READ,
                .latest_layout = daxa::ImageLayout::READ_ONLY_OPTIMAL,
                .slice = {.base_array_layer = 2, .layer_count = 2},
            }};
        auto task_image = task_list.create_task_image({
            .pre_task_list_slice_states = {init_access.begin(), init_access.end()},
            .name = "task list tested image",
        });
        task_list.add_runtime_image(task_image, image);

        auto cmd = app.device.create_command_list({.name = "initialization commands"});
        // cmd.pipeline_barrier_image_transition({
        //     .waiting_pipeline_access = daxa::AccessConsts::READ_WRITE,
        //     .image_id = image,
        //     .after_layout = daxa::ImageLayout::GENERAL,
        //     .image_slice = {.base_array_layer = 0, .layer_count = 2},
        // });
        // cmd.pipeline_barrier_image_transition({
        //     .waiting_pipeline_access = daxa::AccessConsts::READ_WRITE,
        //     .image_id = image,
        //     .after_layout = daxa::ImageLayout::READ_ONLY_OPTIMAL,
        //     .image_slice = {.base_array_layer = 2, .layer_count = 2},
        // });
        cmd.complete();
        app.device.submit_commands({.command_lists = {std::move(cmd)}});

        task_list.add_task({
            .used_buffers = {},
            .used_images =
                {
                    {task_image,
                     daxa::TaskImageAccess::COMPUTE_SHADER_READ_ONLY,
                     daxa::ImageMipArraySlice{.base_array_layer = 1, .layer_count = 1}},
                },
            .task = [](daxa::TaskRuntimeInterface const &) {},
            .name = APPNAME_PREFIX("read image layer 2"),
        });
        task_list.add_task({
            .used_buffers = {},
            .used_images =
                {
                    {task_image,
                     daxa::TaskImageAccess::COMPUTE_SHADER_WRITE_ONLY,
                     daxa::ImageMipArraySlice{.base_array_layer = 3, .layer_count = 1}},
                },
            .task = [](daxa::TaskRuntimeInterface const &) {},
            .name = APPNAME_PREFIX("write image layer 4"),
        });
        task_list.add_task({
            .used_buffers = {},
            .used_images =
                {
                    {task_image,
                     daxa::TaskImageAccess::COMPUTE_SHADER_WRITE_ONLY,
                     daxa::ImageMipArraySlice{.base_array_layer = 0, .layer_count = 4}},
                },
            .task = [](daxa::TaskRuntimeInterface const &) {},
            .name = APPNAME_PREFIX("write image layer 1 - 4"),
        });
        task_list.add_task({
            .used_buffers = {},
            .used_images =
                {
                    {task_image,
                     daxa::TaskImageAccess::COMPUTE_SHADER_READ_ONLY,
                     daxa::ImageMipArraySlice{.base_array_layer = 0, .layer_count = 4}},
                },
            .task = [](daxa::TaskRuntimeInterface const &) {},
            .name = APPNAME_PREFIX("read image layer 1 - 4"),
        });

        task_list.complete({});
        task_list.execute({});
        std::cout << task_list.get_debug_string() << std::endl;
        app.device.wait_idle();
        app.device.destroy_image(image);
        app.device.collect_garbage();
    }

    void shader_integration_inl_use()
    {
        // TEST:
        //  1) Create resources
        //  2) Use Compute dispatch to write to image
        //  4) readback and validate
        AppContext app = {};
        auto image = app.device.create_image({
            .size = {16, 16, 1},
            .array_layer_count = 1,
            .usage = daxa::ImageUsageFlagBits::SHADER_READ_WRITE | daxa::ImageUsageFlagBits::TRANSFER_SRC,
            .name = "underlying image",
        });
        auto buffer = app.device.create_buffer({
            .memory_flags = daxa::MemoryFlagBits::HOST_ACCESS_SEQUENTIAL_WRITE,
            .size = 16,
            .name = "underlying buffer",
        });
        *app.device.get_host_address_as<float>(buffer) = 0.75f;

        daxa::PipelineManager pipeline_manager = daxa::PipelineManager({
            .device = app.device,
            .shader_compile_options = {
                .root_paths = {
                    DAXA_SHADER_INCLUDE_DIR,
                    "tests/2_daxa_api/6_task_list/shaders",
                },
            },
            .name = "pipeline manager",
        });

        auto compile_result = pipeline_manager.add_compute_pipeline({
            .shader_info = {
                .source = daxa::ShaderFile{"shader_integration.glsl"},
                .compile_options{
                    .enable_debug_info = true,
                },
            },
            .name = "compute_pipeline",
        });
        auto compute_pipeline = compile_result.value();

        auto task_list = daxa::TaskList({
            .device = app.device,
            .record_debug_information = true,
            .name = "shader integration test - task list",
        });

        auto task_image = task_list.create_task_image({
            // In this test, this image name will be "aliased", so the name must not be the same.
            .name = "image",
        });
        task_list.add_runtime_image(task_image, image);

        auto task_buffer = task_list.create_task_buffer({
            .name = "settings", // This name MUST be identical to the name used in the shader.
        });
        task_list.add_runtime_buffer(task_buffer, buffer);

        task_list.add_task({
            // daxa::ShaderIntegrationTaskListUses is a startup time generated constant global.
            // This global is set from auto generated code in the .inl file.
            .shader_uses = daxa::ShaderIntegrationTaskListUses,
            .shader_uses_image_aliases = {
                daxa::TaskImageAliasInfo{.alias = "shader_integration_image", .aliased_image = task_image },
            },
            .task = [&](daxa::TaskRuntimeInterface const & tri)
            {
                auto cmd = tri.get_command_list();
                cmd.set_pipeline(*compute_pipeline);
                cmd.dispatch(1, 1, 1);
            },
            .name = "write image in compute",
        });
        task_list.submit({});

        task_list.complete({});
        task_list.execute({});
        std::cout << task_list.get_debug_string() << std::endl;
        app.device.wait_idle();
        app.device.destroy_image(image);
        app.device.destroy_buffer(buffer);
        app.device.collect_garbage();
    }

    void output_graph()
    {
        AppContext const app = {};
        auto task_list = daxa::TaskList({
            .device = app.device,
            .name = APPNAME_PREFIX("task_list (output_graph)"),
        });

        auto task_buffer1 = task_list.create_task_buffer({
            .pre_task_list_slice_states = {daxa::PipelineStageFlagBits::HOST, daxa::AccessTypeFlagBits::WRITE},
            .name = APPNAME_PREFIX("task_buffer1"),
        });
        auto task_buffer2 = task_list.create_task_buffer({
            .pre_task_list_slice_states = {daxa::PipelineStageFlagBits::HOST, daxa::AccessTypeFlagBits::WRITE},
            .name = APPNAME_PREFIX("task_buffer2"),
        });
        auto task_buffer3 = task_list.create_task_buffer({
            .pre_task_list_slice_states = {daxa::PipelineStageFlagBits::HOST, daxa::AccessTypeFlagBits::WRITE},
            .name = APPNAME_PREFIX("task_buffer3"),
        });

        std::array init_access = {
            daxa::ImageSliceState{
                .latest_access = daxa::Access{daxa::PipelineStageFlagBits::HOST, daxa::AccessTypeFlagBits::WRITE},
                .latest_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL}};
        auto task_image1 = task_list.create_task_image({
            .pre_task_list_slice_states = {init_access.begin(), init_access.end()},
            .name = APPNAME_PREFIX("task_image1"),
        });
        auto task_image2 = task_list.create_task_image({
            .pre_task_list_slice_states = {init_access.begin(), init_access.end()},
            .name = APPNAME_PREFIX("task_image2"),
        });
        auto task_image3 = task_list.create_task_image({
            .pre_task_list_slice_states = {init_access.begin(), init_access.end()},
            .name = APPNAME_PREFIX("task_image3"),
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
            .task = [](daxa::TaskRuntimeInterface const &) {},
            .name = APPNAME_PREFIX("task 1 (output_graph)"),
        });

        task_list.add_task({
            .used_buffers = {
                {task_buffer2, daxa::TaskBufferAccess::SHADER_WRITE_ONLY},
            },
            .used_images = {
                {task_image2, daxa::TaskImageAccess::SHADER_WRITE_ONLY, daxa::ImageMipArraySlice{}},
            },
            .task = [](daxa::TaskRuntimeInterface const &) {},
            .name = APPNAME_PREFIX("task 2 (output_graph)"),
        });
        task_list.add_task({
            .used_buffers = {
                {task_buffer3, daxa::TaskBufferAccess::SHADER_WRITE_ONLY},
            },
            .used_images = {
                {task_image3, daxa::TaskImageAccess::SHADER_WRITE_ONLY, daxa::ImageMipArraySlice{}},
            },
            .task = [](daxa::TaskRuntimeInterface const &) {},
            .name = APPNAME_PREFIX("task 3 (output_graph)"),
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
            .task = [](daxa::TaskRuntimeInterface const &) {},
            .name = APPNAME_PREFIX("task 4 (output_graph)"),
        });

        task_list.add_task({
            .used_buffers = {
                {task_buffer3, daxa::TaskBufferAccess::SHADER_READ_ONLY},
            },
            .used_images = {
                {task_image3, daxa::TaskImageAccess::SHADER_READ_ONLY, daxa::ImageMipArraySlice{}},
            },
            .task = [](daxa::TaskRuntimeInterface const &) {},
            .name = APPNAME_PREFIX("task 5 (output_graph)"),
        });

        task_list.complete({});
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
                .name = APPNAME_PREFIX("device (drawing)"),
            });

            daxa::Swapchain swapchain = device.create_swapchain({
                .native_window = get_native_handle(),
                .native_window_platform = get_native_platform(),
                .image_usage = daxa::ImageUsageFlagBits::TRANSFER_DST,
                .name = APPNAME_PREFIX("swapchain (drawing)"),
            });

            daxa::PipelineManager pipeline_manager = daxa::PipelineManager({
                .device = device,
                .shader_compile_options = {
                    .root_paths = {
                        DAXA_SHADER_INCLUDE_DIR,
                        "tests/2_daxa_api/6_task_list/shaders",
                    },
                },
                .name = APPNAME_PREFIX("pipeline_manager (drawing)"),
            });
            // clang-format off
            std::shared_ptr<daxa::RasterPipeline> raster_pipeline = pipeline_manager.add_raster_pipeline({
                .vertex_shader_info = daxa::ShaderCompileInfo{.source = daxa::ShaderFile{"vert.hlsl"}},
                .fragment_shader_info = daxa::ShaderCompileInfo{.source = daxa::ShaderFile{"frag.hlsl"}},
                .color_attachments = {{.format = swapchain.get_format()}},
                .raster = {},
                .name = APPNAME_PREFIX("raster_pipeline (drawing)"),
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
                    .name = APPNAME_PREFIX("task_list (drawing)"),
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
                //     .name = APPNAME_PREFIX("Clear render_image Task (drawing)"),
                // });
                new_task_list.add_task({
                    .used_images = {
                        {task_render_image, daxa::TaskImageAccess::FRAGMENT_SHADER_WRITE_ONLY, daxa::ImageMipArraySlice{}},
                    },
                    .task = [this](daxa::TaskRuntimeInterface runtime)
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
                    .name = APPNAME_PREFIX("Draw to render_image Task (drawing)"),
                });

                // new_task_list.add_copy_image_to_image({
                //     .src_image = task_render_image,
                //     .dst_image = task_swapchain_image,
                //     .extent = {size_x, size_y, 1},
                // });
                new_task_list.complete({});

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
                task_list.execute({});
                auto command_lists = task_list.get_command_lists();
                auto cmd_list = device.create_command_list({});
                // auto last_si_use = task_list.last_uses(task_swapchain_image).back();
                // cmd_list.pipeline_barrier_image_transition({
                //     .awaited_pipeline_access = last_si_use.access,
                //     .before_layout = last_si_use.layout,
                //     .after_layout = daxa::ImageLayout::PRESENT_SRC,
                //     .image_id = swapchain_image,
                // });
                // cmd_list.complete();
                // command_lists.push_back(cmd_list);
                // device.submit_commands({
                //     .command_lists = command_lists,
                //     .wait_binary_semaphores = {swapchain.get_acquire_semaphore()},
                //     .signal_binary_semaphores = {swapchain.get_present_semaphore()},
                //     .signal_timeline_semaphores = {
                //         {swapchain.get_gpu_timeline_semaphore(), swapchain.get_cpu_timeline_value()}},
                // });
                // device.present_frame({
                //     .wait_binary_semaphores = {swapchain.get_present_semaphore()},
                //     .swapchain = swapchain,
                // });
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
                    .name = APPNAME_PREFIX("render_image (drawing)"),
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

    void conditionals()
    {
        struct App : AppWindow<App>
        {
            enum Condition
            {
                CONDITION_FIRST_FRAME = 0,
                CONDITION_UPLOAD_SETTINGS = 1,
            };

            daxa::Context daxa_ctx = daxa::create_context({});

            daxa::Device device = daxa_ctx.create_device({
                .name = APPNAME_PREFIX("conditionals"),
            });

            daxa::Swapchain swapchain = device.create_swapchain({
                .native_window = get_native_handle(),
                .native_window_platform = get_native_platform(),
                .image_usage = daxa::ImageUsageFlagBits::TRANSFER_DST,
                .name = APPNAME_PREFIX("conditionals"),
            });

            daxa::PipelineManager pipeline_manager = daxa::PipelineManager({
                .device = device,
                .shader_compile_options = {
                    .root_paths = {
                        DAXA_SHADER_INCLUDE_DIR,
                        "tests/2_daxa_api/6_task_list/shaders",
                    },
                },
                .name = APPNAME_PREFIX("conditionals"),
            });
            // clang-format off
            std::shared_ptr<daxa::RasterPipeline> raster_pipeline = pipeline_manager.add_raster_pipeline({
                .vertex_shader_info = daxa::ShaderCompileInfo{.source = daxa::ShaderFile{"draw.glsl"}},
                .fragment_shader_info = daxa::ShaderCompileInfo{.source = daxa::ShaderFile{"draw.glsl"}},
                .color_attachments = {{
                    .format = swapchain.get_format(),
                }},
                .raster = {},
                .push_constant_size = sizeof(DrawPush),
                .name = APPNAME_PREFIX("raster_pipeline"),
            }).value();

            bool upload_data = {};
            ConditionalUploadStruct data = {};

            daxa::BufferId buffer = {};
            daxa::TaskBufferId t_buffer = {};

            daxa::BufferId vertex_buffer = {};
            daxa::TaskBufferId t_vertex_buffer = {};

            daxa::ImageId swapchain_image = {};
            daxa::TaskImageId t_swapchain_image = {};

            daxa::TaskList task_list = record_task_list();

            App()
                : AppWindow<App>("Daxa API: Swapchain (conditionals)")
            {
            }

            auto record_task_list() -> daxa::TaskList
            {
                daxa::TaskList new_task_list = daxa::TaskList({
                    .device = device,
                    .swapchain = swapchain,
                    .permutation_condition_count = 2,
                    .name = APPNAME_PREFIX("conditionals"),
                });

                t_swapchain_image = new_task_list.create_task_image({.swapchain_image = true, .name = "swapchain image"});

                t_buffer = new_task_list.create_task_buffer({.name = "conditional upload buffer"});
                buffer = device.create_buffer({.size = sizeof(ConditionalUploadStruct), .name = "t conditional upload data"});
                new_task_list.add_runtime_buffer(t_buffer, buffer);

                t_vertex_buffer = new_task_list.create_task_buffer({.name = "vertex buffer"});
                vertex_buffer = device.create_buffer({.size = sizeof(DrawVertex) * 6, .name = "t vertex buffer"});
                new_task_list.add_runtime_buffer(t_vertex_buffer, vertex_buffer);

                new_task_list.conditional({
                    .condition_index = CONDITION_FIRST_FRAME,
                    .when_true = [&]()
                    {
                        new_task_list.add_task({
                            .used_buffers = {
                                daxa::TaskBufferUse{ t_vertex_buffer, daxa::TaskBufferAccess::TRANSFER_WRITE }
                            },
                            .task = [this](daxa::TaskRuntimeInterface const & runtime)
                            {
                                auto cmd = runtime.get_command_list();
                                auto staging_buffer = device.create_buffer({
                                    .memory_flags = daxa::MemoryFlagBits::HOST_ACCESS_SEQUENTIAL_WRITE,
                                    .size = sizeof(DrawVertex) * 6,
                                    .name = "staging buffer",
                                });
                                std::array<DrawVertex, 6> vertices = {

                                };
                                std::memcpy(device.get_host_address(staging_buffer), vertices.data(), sizeof(DrawVertex) * 6);
                                cmd.destroy_buffer_deferred(staging_buffer);
                                cmd.copy_buffer_to_buffer({
                                    .src_buffer = staging_buffer,
                                    .dst_buffer = runtime.get_buffers(t_vertex_buffer)[0],
                                    .size = sizeof(DrawVertex) * 6,
                                });
                            }
                        });
                    },
                });

                new_task_list.conditional({
                    .condition_index = CONDITION_UPLOAD_SETTINGS,
                    .when_true = [&]()
                    {
                        new_task_list.add_task({
                            .used_buffers = {
                                daxa::TaskBufferUse{t_buffer, daxa::TaskBufferAccess::TRANSFER_WRITE},
                            },
                            .task = [=](daxa::TaskRuntimeInterface const & runtime)
                            {
                                auto cmd = runtime.get_command_list();
                                auto staging_buffer = device.create_buffer({
                                    .memory_flags = daxa::MemoryFlagBits::HOST_ACCESS_SEQUENTIAL_WRITE,
                                    .size = sizeof(ConditionalUploadStruct),
                                    .name = "staging buffer",
                                });
                                *device.get_host_address_as<ConditionalUploadStruct>(staging_buffer) = data;
                                cmd.destroy_buffer_deferred(staging_buffer);
                                cmd.copy_buffer_to_buffer({
                                    .src_buffer = staging_buffer,
                                    .dst_buffer = runtime.get_buffers(t_buffer)[0],
                                    .size = sizeof(ConditionalUploadStruct),
                                });
                            },
                        });
                    },
                });

                new_task_list.add_task({
                    .used_buffers = {
                        {t_vertex_buffer, daxa::TaskBufferAccess::FRAGMENT_SHADER_READ_ONLY},
                        {t_buffer, daxa::TaskBufferAccess::FRAGMENT_SHADER_READ_ONLY},
                    },
                    .used_images = {
                        {t_swapchain_image, daxa::TaskImageAccess::FRAGMENT_SHADER_WRITE_ONLY, daxa::ImageMipArraySlice{}},
                    },
                    .task = [this](daxa::TaskRuntimeInterface runtime)
                    {
                        auto cmd_list = runtime.get_command_list();
                        auto const local_swapchain_image = runtime.get_images(t_swapchain_image)[0];
                        cmd_list.begin_renderpass({
                            .color_attachments = {{.image_view = local_swapchain_image.default_view()}},
                            .render_area = {.x = 0, .y = 0, .width = size_x, .height = size_y},
                        });
                        cmd_list.set_pipeline(*raster_pipeline);
                        cmd_list.push_constant(DrawPush{
                            .vertex_buffer = this->device.get_device_address(runtime.get_buffers(t_vertex_buffer)[0]),
                        });
                        cmd_list.draw({.vertex_count = 3});
                        cmd_list.end_renderpass();
                    },
                    .name = APPNAME_PREFIX("draw task"),
                });

                new_task_list.submit({});
                new_task_list.present({});
                new_task_list.complete({});

                return new_task_list;
            }

            ~App()
            {
                device.wait_idle();
                device.destroy_buffer(buffer);
                device.destroy_buffer(vertex_buffer);
                device.collect_garbage();
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
                pipeline_manager.reload_all();

                if (!swapchain_image.is_empty())
                {
                    task_list.remove_runtime_image(t_swapchain_image, swapchain_image);
                }
                swapchain_image = swapchain.acquire_next_image();
                task_list.add_runtime_image(t_swapchain_image, swapchain_image);

                if (swapchain_image.is_empty())
                {
                    return;
                }
                std::array permutation_conditions = {true, false};
                task_list.execute({.permutation_condition_values = permutation_conditions});
            }

            void on_mouse_move(f32 /*unused*/, f32 /*unused*/) {}

            void on_mouse_button(i32 /*unused*/, i32 /*unused*/) {}

            void on_key(i32 key, i32 action)
            {
                if (key == GLFW_KEY_A && action == GLFW_RELEASE)
                {
                    data.value = 1;
                    upload_data = true;
                }
                else if (key == GLFW_KEY_B && action == GLFW_RELEASE)
                {
                    data.value = 0;
                    upload_data = true;
                }
            }

            auto create_render_image(u32 sx, u32 sy) -> daxa::ImageId
            {
                return device.create_image(daxa::ImageInfo{
                    .format = swapchain.get_format(),
                    .size = {sx, sy, 1},
                    .usage = daxa::ImageUsageFlagBits::COLOR_ATTACHMENT | daxa::ImageUsageFlagBits::TRANSFER_SRC | daxa::ImageUsageFlagBits::TRANSFER_DST,
                    .name = APPNAME_PREFIX("render_image (drawing)"),
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
                    draw();
                }
            }
        };

        App app = {};
        while (true)
        {
            app.upload_data = false;
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
    // tests::write_read_image_layer();
    // tests::create_transfer_read_buffer();
    // tests::initial_layout_access();
    // tests::tracked_slice_barrier_collapsing();
    tests::shader_integration_inl_use();
    // tests::mipmapping();
}
