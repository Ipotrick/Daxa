#include <0_common/window.hpp>
#include <iostream>
#include <thread>

#include <daxa/utils/task_list.hpp>

#define APPNAME "Daxa API Sample: TaskList"
#define APPNAME_PREFIX(x) ("[" APPNAME "] " x)

#include "shaders/shared.inl"

#include <daxa/utils/imgui.hpp>
#include <0_common/imgui/imgui_impl_glfw.h>

struct AppContext
{
    daxa::Context daxa_ctx = daxa::create_context({
        .enable_validation = true,
    });
    daxa::Device device = daxa_ctx.create_device({
        .debug_name = APPNAME_PREFIX("device"),
    });
};

namespace tests
{
    using namespace daxa::types;

    void simplest()
    {
        AppContext app = {};
        auto task_list = daxa::TaskList({
            .device = app.device,
            .debug_name = APPNAME_PREFIX("task_list (simplest)"),
        });
    }

    void execution()
    {
        AppContext app = {};
        auto task_list = daxa::TaskList({
            .device = app.device,
            .debug_name = APPNAME_PREFIX("task_list (execution)"),
        });

        // This is pointless, but done to show how the task list executes
        task_list.add_task({
            .task = [&](daxa::TaskInterface &)
            {
                std::cout << "Hello, ";
            },
            .debug_name = APPNAME_PREFIX("task 1 (execution)"),
        });
        task_list.add_task({
            .task = [&](daxa::TaskInterface &)
            {
                std::cout << "World!" << std::endl;
            },
            .debug_name = APPNAME_PREFIX("task 2 (execution)"),
        });

        task_list.compile();

        task_list.execute();
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

        auto task_image = task_list.create_task_image({
            .fetch_callback = [=]()
            { return image; },
            .debug_name = APPNAME_PREFIX("task_image (image_upload)"),
        });

        auto upload_buffer = task_list.create_task_buffer({
            .fetch_callback = [=]()
            { return buffer; },
            .debug_name = APPNAME_PREFIX("upload_buffer (image_upload)"),
        });

        task_list.add_task({
            .used_buffers = {{upload_buffer, daxa::TaskBufferAccess::TRANSFER_READ}},
            .used_images = {{task_image, daxa::TaskImageAccess::TRANSFER_WRITE}},
            .task = [](daxa::TaskInterface &)
            {
                // TODO: Implement this task!
            },
            .debug_name = APPNAME_PREFIX("upload task (image_upload)"),
        });

        task_list.compile();

        task_list.execute();

        app.device.destroy_buffer(buffer);
        app.device.destroy_image(image);
    }

    void output_graph()
    {
        AppContext app = {};
        auto task_list = daxa::TaskList({
            .device = app.device,
            .debug_name = APPNAME_PREFIX("task_list (output_graph)"),
        });

        auto task_buffer1 = task_list.create_task_buffer({
            .last_access = {daxa::PipelineStageFlagBits::HOST, daxa::AccessTypeFlagBits::WRITE},
            .debug_name = APPNAME_PREFIX("task_buffer1 (output_graph)"),
        });
        auto task_buffer2 = task_list.create_task_buffer({
            .last_access = {daxa::PipelineStageFlagBits::HOST, daxa::AccessTypeFlagBits::WRITE},
            .debug_name = APPNAME_PREFIX("task_buffer2 (output_graph)"),
        });
        auto task_buffer3 = task_list.create_task_buffer({
            .last_access = {daxa::PipelineStageFlagBits::HOST, daxa::AccessTypeFlagBits::WRITE},
            .debug_name = APPNAME_PREFIX("task_buffer3 (output_graph)"),
        });

        task_list.add_task({
            .used_buffers = {
                {task_buffer1, daxa::TaskBufferAccess::SHADER_WRITE_ONLY},
                {task_buffer2, daxa::TaskBufferAccess::SHADER_READ_ONLY},
            },
            .task = [](daxa::TaskInterface &) {},
            .debug_name = APPNAME_PREFIX("task 1 (output_graph)"),
        });

        task_list.add_task({
            .used_buffers = {
                {task_buffer2, daxa::TaskBufferAccess::SHADER_WRITE_ONLY},
            },
            .task = [](daxa::TaskInterface &) {},
            .debug_name = APPNAME_PREFIX("task 2 (output_graph)"),
        });

        task_list.add_task({
            .used_buffers = {
                {task_buffer2, daxa::TaskBufferAccess::SHADER_WRITE_ONLY},
                {task_buffer3, daxa::TaskBufferAccess::SHADER_WRITE_ONLY},
            },
            .task = [](daxa::TaskInterface &) {},
            .debug_name = APPNAME_PREFIX("task 3 (output_graph)"),
        });

        task_list.add_task({
            .used_buffers = {
                {task_buffer3, daxa::TaskBufferAccess::SHADER_READ_ONLY},
            },
            .task = [](daxa::TaskInterface &) {},
            .debug_name = APPNAME_PREFIX("task 4 (output_graph)"),
        });

        task_list.compile();
        task_list.output_graphviz();
    }

    void mipmapping()
    {
        struct App : AppWindow<App>
        {
            daxa::Context daxa_ctx = daxa::create_context({
                .enable_validation = true,
            });
            daxa::Device device = daxa_ctx.create_device({
                .debug_name = APPNAME_PREFIX("device (mipmapping)"),
            });

            daxa::Swapchain swapchain = device.create_swapchain({
                .native_window = get_native_handle(),
                .present_mode = daxa::PresentMode::DO_NOT_WAIT_FOR_VBLANK,
                .image_usage = daxa::ImageUsageFlagBits::TRANSFER_DST,
                .debug_name = APPNAME_PREFIX("swapchain"),
            });

            daxa::PipelineCompiler pipeline_compiler = device.create_pipeline_compiler({
                .shader_compile_options = {
                    .root_paths = {
                        "tests/2_daxa_api/6_task_list/shaders",
                        "include",
                    },
                    .opt_level = 2,
                    .language = daxa::ShaderLanguage::GLSL,
                },
                .debug_name = APPNAME_PREFIX("pipeline_compiler"),
            });
            daxa::ImGuiRenderer imgui_renderer = create_imgui_renderer();
            auto create_imgui_renderer() -> daxa::ImGuiRenderer
            {
                ImGui::CreateContext();
                ImGui_ImplGlfw_InitForVulkan(glfw_window_ptr, true);
                return daxa::ImGuiRenderer({
                    .device = device,
                    .pipeline_compiler = pipeline_compiler,
                    .format = swapchain.get_format(),
                });
            }

            // clang-format off
            daxa::ComputePipeline compute_pipeline = pipeline_compiler.create_compute_pipeline({
                .shader_info = {.source = daxa::ShaderFile{"mipmapping.glsl"}},
                .push_constant_size = sizeof(MipmappingComputePushConstant),
                .debug_name = APPNAME_PREFIX("compute_pipeline"),
            }).value();
            // clang-format on

            daxa::ImageId render_image = device.create_image(daxa::ImageInfo{
                .format = daxa::Format::R8G8B8A8_UNORM,
                .size = {512, 512, 1},
                .mip_level_count = 5,
                .usage = daxa::ImageUsageFlagBits::SHADER_READ_WRITE | daxa::ImageUsageFlagBits::TRANSFER_SRC,
                .debug_name = APPNAME_PREFIX("render_image"),
            });

            daxa::BinarySemaphore acquire_semaphore = device.create_binary_semaphore({.debug_name = APPNAME_PREFIX("acquire_semaphore")});
            daxa::BinarySemaphore present_semaphore = device.create_binary_semaphore({.debug_name = APPNAME_PREFIX("present_semaphore")});

            daxa::CommandSubmitInfo submit_info = {};
            daxa::TaskImageId task_swapchain_image = {};
            daxa::TaskImageId task_render_image = {};

            MipmappingComputeInput compute_input = {
                .paint_col = {1.0f, 0.0f, 0.0f},
            };
            daxa::BufferId mipmapping_compute_input_buffer = device.create_buffer({
                .size = sizeof(MipmappingComputeInput),
                .debug_name = APPNAME_PREFIX("mipmapping_compute_input_buffer"),
            });
            daxa::BufferId staging_mipmapping_compute_input_buffer = device.create_buffer({
                .memory_flags = daxa::MemoryFlagBits::HOST_ACCESS_RANDOM,
                .size = sizeof(MipmappingComputeInput),
                .debug_name = APPNAME_PREFIX("staging_mipmapping_compute_input_buffer"),
            });
            daxa::TaskBufferId task_mipmapping_compute_input_buffer;
            daxa::TaskBufferId task_staging_mipmapping_compute_input_buffer;

            bool mouse_drawing = false, gen_mipmaps = false;

            daxa::TaskList task_list = record_tasks();

            App() : AppWindow<App>(APPNAME, 768, 512) {}

            ~App()
            {
                device.wait_idle();
                device.collect_garbage();
                device.destroy_buffer(mipmapping_compute_input_buffer);
                device.destroy_buffer(staging_mipmapping_compute_input_buffer);
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

            void ui_update()
            {
                ImGui_ImplGlfw_NewFrame();
                ImGui::NewFrame();
                ImGui::Begin("Settings");
                ImGui::ColorEdit3("Brush Color", reinterpret_cast<f32 *>(&compute_input.paint_col));
                if (ImGui::Button("Gen Mip-maps"))
                    gen_mipmaps = true;
                ImGui::End();
                ImGui::Render();
            }

            void draw()
            {
                ui_update();

                if (pipeline_compiler.check_if_sources_changed(compute_pipeline))
                {
                    auto new_pipeline = pipeline_compiler.recreate_compute_pipeline(compute_pipeline);
                    std::cout << new_pipeline.to_string() << std::endl;
                    if (new_pipeline.is_ok())
                    {
                        compute_pipeline = new_pipeline.value();
                    }
                }
                task_list.execute();
            }

            void on_mouse_move(f32 x, f32 y)
            {
                compute_input.mouse_x = x / static_cast<f32>(size_x * (2.0f / 3.0f)) * 512.0f;
                compute_input.mouse_y = y / static_cast<f32>(size_y) * 512.0f;
            }

            void on_mouse_button(i32 button, i32 action)
            {
                if (button == GLFW_MOUSE_BUTTON_1)
                    mouse_drawing = action != GLFW_RELEASE;
            }

            void on_key(i32 key, i32 action) {}

            void on_resize(u32 sx, u32 sy)
            {
                minimized = (sx == 0 || sy == 0);
                if (!minimized)
                {
                    swapchain.resize();
                    size_x = swapchain.info().width;
                    size_y = swapchain.info().height;
                    draw();
                }
            }

            auto record_tasks() -> daxa::TaskList
            {
                daxa::TaskList new_task_list = daxa::TaskList({.device = device, .debug_name = APPNAME_PREFIX("main task list")});

                task_swapchain_image = new_task_list.create_task_image({
                    .fetch_callback = [=, this]()
                    { return swapchain.acquire_next_image(acquire_semaphore); },
                    .swapchain_parent = std::pair{swapchain, acquire_semaphore},
                    .debug_name = "task swapchain image",
                });
                task_render_image = new_task_list.create_task_image({
                    .fetch_callback = [=, this]()
                    { return render_image; },
                    .debug_name = "task render image",
                });

                task_mipmapping_compute_input_buffer = new_task_list.create_task_buffer({
                    .fetch_callback = [this]()
                    { return mipmapping_compute_input_buffer; },
                    .debug_name = APPNAME_PREFIX("task_mipmapping_compute_input_buffer"),
                });
                task_staging_mipmapping_compute_input_buffer = new_task_list.create_task_buffer({
                    .fetch_callback = [this]()
                    { return staging_mipmapping_compute_input_buffer; },
                    .debug_name = APPNAME_PREFIX("task_staging_mipmapping_compute_input_buffer"),
                });

                new_task_list.add_task({
                    .used_buffers = {
                        {task_staging_mipmapping_compute_input_buffer, daxa::TaskBufferAccess::HOST_TRANSFER_WRITE},
                    },
                    .task = [this](daxa::TaskInterface interf)
                    {
                        auto staging_buffer = interf.get_buffer(task_staging_mipmapping_compute_input_buffer);
                        MipmappingComputeInput * buffer_ptr = device.map_memory_as<MipmappingComputeInput>(staging_buffer);
                        *buffer_ptr = this->compute_input;
                        this->compute_input.p_mouse_x = this->compute_input.mouse_x;
                        this->compute_input.p_mouse_y = this->compute_input.mouse_y;
                        device.unmap_memory(staging_buffer);
                    },
                    .debug_name = APPNAME_PREFIX("Input MemMap"),
                });

                new_task_list.add_task({
                    .used_buffers = {
                        {task_staging_mipmapping_compute_input_buffer, daxa::TaskBufferAccess::HOST_TRANSFER_WRITE},
                    },
                    .task = [this](daxa::TaskInterface interf)
                    {
                        auto cmd_list = interf.get_command_list();
                        cmd_list.copy_buffer_to_buffer({
                            .src_buffer = interf.get_buffer(task_staging_mipmapping_compute_input_buffer),
                            .dst_buffer = interf.get_buffer(task_mipmapping_compute_input_buffer),
                            .size = sizeof(MipmappingComputeInput),
                        });
                    },
                    .debug_name = APPNAME_PREFIX("Input Transfer"),
                });

                new_task_list.add_clear_image({
                    .clear_value = {std::array<f32, 4>{0.2f, 0.2f, 0.2f, 1.0f}},
                    .dst_image = task_swapchain_image,
                    .dst_slice = {},
                    .debug_name = APPNAME_PREFIX("Clear swapchain"),
                });

                new_task_list.add_task(daxa::TaskInfo{
                    .used_buffers = {
                        {task_mipmapping_compute_input_buffer, daxa::TaskBufferAccess::COMPUTE_SHADER_READ_ONLY},
                    },
                    .used_images = {
                        {task_render_image, daxa::TaskImageAccess::COMPUTE_SHADER_READ_WRITE},
                    },
                    .task = [=, this](daxa::TaskInterface & interf)
                    {
                        if (mouse_drawing)
                        {
                            auto cmd_list = interf.get_command_list();
                            auto render_target_id = interf.get_image(task_render_image);
                            auto input_buffer = interf.get_buffer(task_mipmapping_compute_input_buffer);
                            auto render_target_size = device.info_image(render_target_id).size;
                            cmd_list.set_pipeline(compute_pipeline);
                            cmd_list.push_constant(MipmappingComputePushConstant{
                                .image_id = render_target_id.default_view(),
                                .compute_input = this->device.buffer_reference(input_buffer),
                                .frame_dim = {render_target_size[0], render_target_size[1]},
                            });
                            cmd_list.dispatch((render_target_size[0] + 7) / 8, (render_target_size[1] + 7) / 8);
                        }
                    },
                    .debug_name = "mouse paint",
                });

                // TODO: HOW TO GENERATE MIPS?
                // new_task_list.add_task({
                //     .used_images = {
                //         {task_render_image, daxa::TaskImageAccess::COMPUTE_SHADER_READ_WRITE, mip0},
                //         {task_render_image, daxa::TaskImageAccess::COMPUTE_SHADER_READ_WRITE, mip1},
                //     },
                // });

                new_task_list.add_task({
                    .used_images = {
                        {task_render_image, daxa::TaskImageAccess::TRANSFER_READ},
                        {task_swapchain_image, daxa::TaskImageAccess::TRANSFER_WRITE},
                    },
                    .task = [=, this](daxa::TaskInterface & interf)
                    {
                        daxa::CommandList cmd_list = interf.get_command_list();
                        ImageId src_image_id = interf.get_image(task_render_image);
                        ImageId dst_image_id = interf.get_image(task_swapchain_image);
                        auto const & src_info = device.info_image(src_image_id);
                        auto src_size = src_info.size;
                        auto dst_size = device.info_image(dst_image_id).size;

                        cmd_list.blit_image_to_image({
                            .src_image = src_image_id,
                            .src_image_layout = daxa::ImageLayout::TRANSFER_SRC_OPTIMAL,
                            .dst_image = dst_image_id,
                            .dst_image_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
                            .src_slice = {.image_aspect = daxa::ImageAspectFlagBits::COLOR},
                            .src_offsets = {{{0, 0, 0}, {static_cast<i32>(src_size[0]), static_cast<i32>(src_size[1]), 1}}},
                            .dst_slice = {.image_aspect = daxa::ImageAspectFlagBits::COLOR},
                            .dst_offsets = {
                                {
                                    {0, 0, 0},
                                    {static_cast<i32>(dst_size[0] * (2.0f / 3.0f)), static_cast<i32>(dst_size[1]), 1},
                                },
                            },
                        });

                        for (i32 i = 0; i < src_info.mip_level_count - 1; ++i)
                        {
                            i32 scl_1 = 1 << (i + 0);
                            i32 scl_2 = 1 << (i + 1);
                            f32 s0 = static_cast<f32>(scl_1) * 0.5f;
                            f32 s1 = (static_cast<f32>(scl_1) - 1.0f) / static_cast<f32>(scl_1);
                            f32 s2 = (static_cast<f32>(scl_2) - 1.0f) / static_cast<f32>(scl_2);
                            cmd_list.blit_image_to_image({
                                .src_image = src_image_id,
                                .src_image_layout = daxa::ImageLayout::TRANSFER_SRC_OPTIMAL,
                                .dst_image = dst_image_id,
                                .dst_image_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
                                .src_slice = {.image_aspect = daxa::ImageAspectFlagBits::COLOR, .mip_level = static_cast<u32>(i + 1)},
                                .src_offsets = {{{0, 0, 0}, {static_cast<i32>(static_cast<f32>(src_size[0]) / scl_2), static_cast<i32>(static_cast<f32>(src_size[1]) / scl_2), 1}}},
                                .dst_slice = {.image_aspect = daxa::ImageAspectFlagBits::COLOR},
                                .dst_offsets = {
                                    {
                                        {static_cast<i32>(dst_size[0] * (2.0f / 3.0f)), static_cast<i32>(dst_size[1] * s1), 0},
                                        {static_cast<i32>(dst_size[0] * (2.0f / 3.0f + 1.0f / (s0 * 6.0f))), static_cast<i32>(static_cast<f32>(dst_size[1]) * s2), 1},
                                    },
                                },
                            });
                        }
                    },
                    .debug_name = "blit to swapchain",
                });

                new_task_list.add_task({
                    .used_images = {
                        {task_swapchain_image, daxa::TaskImageAccess::COLOR_ATTACHMENT},
                    },
                    .task = [=, this](daxa::TaskInterface & interf)
                    {
                        daxa::CommandList cmd_list = interf.get_command_list();
                        ImageId render_target_id = interf.get_image(task_swapchain_image);
                        auto render_size = device.info_image(render_target_id).size;
                        imgui_renderer.record_commands(ImGui::GetDrawData(), cmd_list, render_target_id, render_size[0], render_size[1]);
                    },
                    .debug_name = "Imgui",
                });

                new_task_list.submit(&submit_info);
                new_task_list.present({});
                new_task_list.compile();

                return new_task_list;
            }
        };

        App app = {};
        while (true)
        {
            if (app.update())
                break;
        }
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
                .image_usage = daxa::ImageUsageFlagBits::TRANSFER_DST,
                .debug_name = APPNAME_PREFIX("swapchain (drawing)"),
            });

            daxa::PipelineCompiler pipeline_compiler = device.create_pipeline_compiler({
                .shader_compile_options = {
                    .root_paths = {
                        "tests/2_daxa_api/6_task_list/shaders",
                        "include",
                    },
                },
                .debug_name = APPNAME_PREFIX("pipeline_compiler (drawing)"),
            });
            // clang-format off
            daxa::RasterPipeline raster_pipeline = pipeline_compiler.create_raster_pipeline({
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

            daxa::ImageId swapchain_image;
            daxa::TaskImageId task_swapchain_image;

            daxa::BinarySemaphore acquire_semaphore = device.create_binary_semaphore({.debug_name = APPNAME_PREFIX("acquire_semaphore")});

            daxa::BinarySemaphore present_semaphore = device.create_binary_semaphore({.debug_name = APPNAME_PREFIX("present_semaphore")});

            App() : AppWindow<App>("Daxa API: Swapchain (clearcolor)")
            {
                record_task_list();
            }

            auto record_task_list() -> daxa::TaskList
            {
                daxa::TaskList new_task_list = daxa::TaskList({
                    .device = device,
                    .debug_name = APPNAME_PREFIX("task_list (drawing)"),
                });
                task_swapchain_image = new_task_list.create_task_image({
                    .fetch_callback = [this]()
                    { return swapchain_image; },
                    .debug_name = APPNAME_PREFIX("task_swapchain_image (drawing)"),
                });
                task_render_image = new_task_list.create_task_image({
                    .fetch_callback = [this]()
                    { return render_image; },
                    .debug_name = APPNAME_PREFIX("task_render_image (drawing)"),
                });

                new_task_list.add_clear_image({
                    .clear_value = {std::array<f32, 4>{1, 0, 1, 1}},
                    .dst_image = task_render_image,
                    .dst_slice = {},
                    .debug_name = APPNAME_PREFIX("Clear render_image Task (drawing)"),
                });
                new_task_list.add_task({
                    .used_images = {
                        {task_render_image, daxa::TaskImageAccess::FRAGMENT_SHADER_WRITE_ONLY},
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
                    .debug_name = APPNAME_PREFIX("Draw to render_image Task (drawing)"),
                });

                new_task_list.add_copy_image_to_image({
                    .src_image = task_render_image,
                    .dst_image = task_swapchain_image,
                    .extent = {size_x, size_y, 1},
                });
                new_task_list.compile();

                return new_task_list;
            }

            ~App()
            {
                device.wait_idle();
                device.collect_garbage();
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
                if (pipeline_compiler.check_if_sources_changed(raster_pipeline))
                {
                    auto new_pipeline = pipeline_compiler.recreate_raster_pipeline(raster_pipeline);
                    std::cout << new_pipeline.to_string() << std::endl;
                    if (new_pipeline.is_ok())
                    {
                        raster_pipeline = new_pipeline.value();
                    }
                }
                swapchain_image = swapchain.acquire_next_image(acquire_semaphore);
                task_list.execute();
                auto command_lists = task_list.command_lists();
                auto cmd_list = device.create_command_list({});
                cmd_list.pipeline_barrier_image_transition({
                    .awaited_pipeline_access = task_list.last_access(task_swapchain_image),
                    .before_layout = task_list.last_layout(task_swapchain_image),
                    .after_layout = daxa::ImageLayout::PRESENT_SRC,
                    .image_id = swapchain_image,
                });
                cmd_list.complete();
                command_lists.push_back(cmd_list);
                device.submit_commands({
                    .command_lists = command_lists,
                    .wait_binary_semaphores = {acquire_semaphore},
                    .signal_binary_semaphores = {present_semaphore},
                });
                device.present_frame({
                    .wait_binary_semaphores = {present_semaphore},
                    .swapchain = swapchain,
                });
            }

            void on_mouse_move(f32, f32) {}
            void on_mouse_button(i32, i32) {}
            void on_key(i32, i32) {}

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
                    size_x = swapchain.info().width;
                    size_y = swapchain.info().height;
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
                break;
        }
    }
} // namespace tests

int main()
{
    // tests::simplest();
    // tests::image_upload();
    // tests::execution();
    // tests::output_graph();
    tests::mipmapping();
    // tests::drawing();
}
