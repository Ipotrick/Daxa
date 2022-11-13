#pragma once

#include "common.hpp"

namespace tests
{
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
                .native_window_platform = get_native_platform(),
                .present_mode = daxa::PresentMode::DO_NOT_WAIT_FOR_VBLANK,
                .image_usage = daxa::ImageUsageFlagBits::TRANSFER_DST,
                .debug_name = APPNAME_PREFIX("swapchain"),
            });
            daxa::ImageId swapchain_image;

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
                .size = {128, 128, 1},
                .mip_level_count = 5,
                .usage = daxa::ImageUsageFlagBits::SHADER_READ_WRITE | daxa::ImageUsageFlagBits::TRANSFER_SRC | daxa::ImageUsageFlagBits::TRANSFER_DST,
                .debug_name = APPNAME_PREFIX("render_image"),
            });

            daxa::CommandSubmitInfo submit_info = {};
            daxa::TaskImageId task_swapchain_image = {};
            daxa::TaskImageId task_render_image = {};

            MipmappingGpuInput gpu_input = {
                .mouse_x = {},
                .mouse_y = {},
                .p_mouse_x = {},
                .p_mouse_y = {},
                .paint_radius = 5.0f,
                .paint_col = {1.0f, 0.0f, 0.0f},
            };
            daxa::BufferId mipmapping_gpu_input_buffer = device.create_buffer({
                .size = sizeof(MipmappingGpuInput),
                .debug_name = APPNAME_PREFIX("mipmapping_gpu_input_buffer"),
            });
            daxa::TaskBufferId task_mipmapping_gpu_input_buffer;
            daxa::TaskBufferId task_staging_mipmapping_gpu_input_buffer;

            bool mouse_drawing = false;

            daxa::TaskList task_list = record_tasks();

            App() : AppWindow<App>(APPNAME, 768 * 2, 512 * 2) {}
            ~App()
            {
                device.wait_idle();
                device.collect_garbage();
                device.destroy_buffer(mipmapping_gpu_input_buffer);
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
                ImGui::SliderFloat("Brush Radius", &gpu_input.paint_radius, 1.0f, 15.0f);
                ImGui::ColorEdit3("Brush Color", reinterpret_cast<f32 *>(&gpu_input.paint_col));
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

                // non_task_list_execute();

                task_list.remove_runtime_image(task_swapchain_image, swapchain_image);
                swapchain_image = swapchain.acquire_next_image();
                task_list.add_runtime_image(task_swapchain_image, swapchain_image);
                if (swapchain_image.is_empty())
                {
                    return;
                }
                // task_list.debug_print();
                task_list.execute();
            }

            void on_mouse_move(f32 x, f32 y)
            {
                auto & io = ImGui::GetIO();
                if (io.WantCaptureMouse || io.WantCaptureKeyboard)
                    return;
                gpu_input.mouse_x = x / static_cast<f32>(size_x) * (1.5f) * 128.0f;
                gpu_input.mouse_y = y / static_cast<f32>(size_y) * 128.0f;
            }
            void on_mouse_button(i32 button, i32 action)
            {
                auto & io = ImGui::GetIO();
                if (io.WantCaptureMouse || io.WantCaptureKeyboard)
                    return;
                if (button == GLFW_MOUSE_BUTTON_1)
                    mouse_drawing = action != GLFW_RELEASE;
            }
            void on_key(i32, i32) {}
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

            void update_gpu_input(daxa::CommandList & cmd_list, daxa::BufferId input_buffer)
            {
                auto staging_buffer = device.create_buffer({
                    .memory_flags = daxa::MemoryFlagBits::HOST_ACCESS_RANDOM,
                    .size = sizeof(MipmappingGpuInput),
                    .debug_name = APPNAME_PREFIX("staging_mipmapping_gpu_input_buffer"),
                });
                MipmappingGpuInput * buffer_ptr = device.map_memory_as<MipmappingGpuInput>(staging_buffer);
                *buffer_ptr = this->gpu_input;
                this->gpu_input.p_mouse_x = this->gpu_input.mouse_x;
                this->gpu_input.p_mouse_y = this->gpu_input.mouse_y;
                device.unmap_memory(staging_buffer);
                cmd_list.destroy_buffer_deferred(staging_buffer);

                cmd_list.copy_buffer_to_buffer({
                    .src_buffer = staging_buffer,
                    .dst_buffer = input_buffer,
                    .size = sizeof(MipmappingGpuInput),
                });
            }
            void paint(daxa::CommandList & cmd_list, daxa::ImageId render_target_id, daxa::BufferId input_buffer)
            {
                if (mouse_drawing)
                {
                    auto render_target_size = device.info_image(render_target_id).size;
                    cmd_list.set_pipeline(compute_pipeline);
                    auto const push = MipmappingComputePushConstant{
                        .image_id = render_target_id.default_view(),
                        .gpu_input = input_buffer,
                        // .gpu_input = this->device.get_device_address(input_buffer),
                        .frame_dim = {render_target_size.x, render_target_size.y},
                    };
                    cmd_list.push_constant(push);
                    cmd_list.dispatch((render_target_size.x + 7) / 8, (render_target_size.y + 7) / 8);
                }
            }
            void draw_ui(daxa::CommandList & cmd_list, daxa::ImageId render_target_id)
            {
                auto render_size = device.info_image(render_target_id).size;
                imgui_renderer.record_commands(ImGui::GetDrawData(), cmd_list, render_target_id, render_size.x, render_size.y);
            }
            void blit_image_to_swapchain(daxa::CommandList & cmd_list, daxa::ImageId src_image_id, daxa::ImageId dst_image_id)
            {
                auto const & src_info = device.info_image(src_image_id);
                auto src_size = src_info.size;
                auto dst_size = device.info_image(dst_image_id).size;
                cmd_list.blit_image_to_image({
                    .src_image = src_image_id,
                    .src_image_layout = daxa::ImageLayout::TRANSFER_SRC_OPTIMAL,
                    .dst_image = dst_image_id,
                    .dst_image_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
                    .src_slice = {.image_aspect = daxa::ImageAspectFlagBits::COLOR},
                    .src_offsets = {{{0, 0, 0}, {static_cast<i32>(src_size.x), static_cast<i32>(src_size.y), 1}}},
                    .dst_slice = {.image_aspect = daxa::ImageAspectFlagBits::COLOR},
                    .dst_offsets = {
                        {
                            {0, 0, 0},
                            {static_cast<i32>(static_cast<f32>(dst_size.x) * (2.0f / 3.0f)), static_cast<i32>(dst_size.y), 1},
                        },
                    },
                });
                for (i32 i = 0; i < static_cast<i32>(src_info.mip_level_count - 1); ++i)
                {
                    f32 scl_1 = static_cast<f32>(1 << (i + 0));
                    f32 scl_2 = static_cast<f32>(1 << (i + 1));
                    f32 s0 = scl_1 * 0.5f;
                    f32 s1 = (scl_1 - 1.0f) / scl_1;
                    f32 s2 = (scl_2 - 1.0f) / scl_2;
                    cmd_list.blit_image_to_image({
                        .src_image = src_image_id,
                        .src_image_layout = daxa::ImageLayout::TRANSFER_SRC_OPTIMAL,
                        .dst_image = dst_image_id,
                        .dst_image_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
                        .src_slice = {.image_aspect = daxa::ImageAspectFlagBits::COLOR, .mip_level = static_cast<u32>(i + 1)},
                        .src_offsets = {{{0, 0, 0}, {static_cast<i32>(static_cast<f32>(src_size.x) / scl_2), static_cast<i32>(static_cast<f32>(src_size.y) / scl_2), 1}}},
                        .dst_slice = {.image_aspect = daxa::ImageAspectFlagBits::COLOR},
                        .dst_offsets = {
                            {
                                {
                                    static_cast<i32>(static_cast<f32>(dst_size.x) * (2.0f / 3.0f)),
                                    static_cast<i32>(static_cast<f32>(dst_size.y) * s1),
                                    0,
                                },
                                {
                                    static_cast<i32>(static_cast<f32>(dst_size.x) * (2.0f / 3.0f + 1.0f / (s0 * 6.0f))),
                                    static_cast<i32>(static_cast<f32>(dst_size.y) * s2),
                                    1,
                                },
                            },
                        },
                    });
                }
            }

            void non_task_list_execute()
            {
                swapchain_image = swapchain.acquire_next_image();
                if (swapchain_image.is_empty())
                {
                    return;
                }
                auto cmd_list = device.create_command_list({});

                cmd_list.pipeline_barrier_image_transition({
                    .waiting_pipeline_access = daxa::AccessConsts::TRANSFER_WRITE,
                    .before_layout = daxa::ImageLayout::UNDEFINED,
                    .after_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
                    .image_id = swapchain_image,
                });
                cmd_list.clear_image({
                    .dst_image_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
                    .clear_value = {std::array<f32, 4>{1, 0, 1, 1}},
                    .dst_image = swapchain_image,
                });
                cmd_list.pipeline_barrier({
                    .awaited_pipeline_access = daxa::AccessConsts::NONE,
                    .waiting_pipeline_access = daxa::AccessConsts::TRANSFER_WRITE,
                });
                // update_gpu_input(staging_mipmapping_gpu_input_buffer);
                cmd_list.pipeline_barrier({
                    .awaited_pipeline_access = daxa::AccessConsts::TRANSFER_WRITE,
                    .waiting_pipeline_access = daxa::AccessConsts::TRANSFER_READ,
                });
                update_gpu_input(cmd_list, mipmapping_gpu_input_buffer);
                cmd_list.pipeline_barrier_image_transition({
                    .awaited_pipeline_access = daxa::AccessConsts::NONE,
                    .waiting_pipeline_access = daxa::AccessConsts::COMPUTE_SHADER_WRITE,
                    .before_layout = daxa::ImageLayout::UNDEFINED,
                    .after_layout = daxa::ImageLayout::GENERAL,
                    .image_id = render_image,
                });
                cmd_list.pipeline_barrier({
                    .awaited_pipeline_access = daxa::AccessConsts::TRANSFER_WRITE,
                    .waiting_pipeline_access = daxa::AccessConsts::COMPUTE_SHADER_READ,
                });
                paint(cmd_list, render_image, mipmapping_gpu_input_buffer);
                {
                    auto image_info = device.info_image(render_image);
                    std::array<i32, 3> mip_size = {static_cast<i32>(image_info.size.x), static_cast<i32>(image_info.size.y), static_cast<i32>(image_info.size.z)};

                    cmd_list.pipeline_barrier_image_transition({
                        .awaited_pipeline_access = daxa::AccessConsts::NONE,
                        .waiting_pipeline_access = daxa::AccessConsts::TRANSFER_WRITE,
                        .before_layout = daxa::ImageLayout::UNDEFINED,
                        .after_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
                        .image_slice = {
                            .image_aspect = image_info.aspect,
                            .base_mip_level = 1,
                            .level_count = 4,
                            .base_array_layer = 0,
                            .layer_count = 1,
                        },
                        .image_id = render_image,
                    });

                    for (u32 i = 0; i < image_info.mip_level_count - 1; ++i)
                    {
                        cmd_list.pipeline_barrier_image_transition({
                            .awaited_pipeline_access = (i == 0 ? daxa::AccessConsts::COMPUTE_SHADER_WRITE : daxa::AccessConsts::TRANSFER_WRITE),
                            .waiting_pipeline_access = daxa::AccessConsts::BLIT_READ,
                            .before_layout = (i == 0 ? daxa::ImageLayout::GENERAL : daxa::ImageLayout::TRANSFER_DST_OPTIMAL),
                            .after_layout = daxa::ImageLayout::TRANSFER_SRC_OPTIMAL,
                            .image_slice = {
                                .image_aspect = image_info.aspect,
                                .base_mip_level = i,
                                .level_count = 1,
                                .base_array_layer = 0,
                                .layer_count = 1,
                            },
                            .image_id = render_image,
                        });
                        std::array<i32, 3> next_mip_size = {std::max<i32>(1, mip_size[0] / 2), std::max<i32>(1, mip_size[1] / 2), std::max<i32>(1, mip_size[2] / 2)};
                        cmd_list.blit_image_to_image({
                            .src_image = render_image,
                            .src_image_layout = daxa::ImageLayout::TRANSFER_SRC_OPTIMAL,
                            .dst_image = render_image,
                            .dst_image_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
                            .src_slice = {
                                .image_aspect = image_info.aspect,
                                .mip_level = i,
                                .base_array_layer = 0,
                                .layer_count = 1,
                            },
                            .src_offsets = {{{0, 0, 0}, {mip_size[0], mip_size[1], mip_size[2]}}},
                            .dst_slice = {
                                .image_aspect = image_info.aspect,
                                .mip_level = i + 1,
                                .base_array_layer = 0,
                                .layer_count = 1,
                            },
                            .dst_offsets = {{{0, 0, 0}, {next_mip_size[0], next_mip_size[1], next_mip_size[2]}}},
                            .filter = daxa::Filter::LINEAR,
                        });
                        mip_size = next_mip_size;
                    }
                    cmd_list.pipeline_barrier_image_transition({
                        .awaited_pipeline_access = daxa::AccessConsts::TRANSFER_WRITE,
                        .waiting_pipeline_access = daxa::AccessConsts::TRANSFER_READ,
                        .before_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
                        .after_layout = daxa::ImageLayout::TRANSFER_SRC_OPTIMAL,
                        .image_slice = {
                            .image_aspect = image_info.aspect,
                            .base_mip_level = 4,
                            .level_count = 1,
                            .base_array_layer = 0,
                            .layer_count = 1,
                        },
                        .image_id = render_image,
                    });
                }
                cmd_list.pipeline_barrier_image_transition({
                    .awaited_pipeline_access = daxa::AccessConsts::TRANSFER_WRITE,
                    .waiting_pipeline_access = daxa::AccessConsts::TRANSFER_WRITE,
                    .before_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
                    .after_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
                    .image_id = swapchain_image,
                });
                blit_image_to_swapchain(cmd_list, render_image, swapchain_image);
                cmd_list.pipeline_barrier_image_transition({
                    .awaited_pipeline_access = daxa::AccessConsts::TRANSFER_WRITE,
                    .waiting_pipeline_access = daxa::AccessConsts::COLOR_ATTACHMENT_OUTPUT_READ_WRITE,
                    .before_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
                    .after_layout = daxa::ImageLayout::ATTACHMENT_OPTIMAL,
                    .image_id = swapchain_image,
                });
                draw_ui(cmd_list, swapchain_image);
                cmd_list.pipeline_barrier_image_transition({
                    .awaited_pipeline_access = daxa::AccessConsts::COLOR_ATTACHMENT_OUTPUT_READ_WRITE,
                    .waiting_pipeline_access = {.stages = daxa::PipelineStageFlagBits::BOTTOM_OF_PIPE, .type = daxa::AccessTypeFlagBits::NONE},
                    .before_layout = daxa::ImageLayout::ATTACHMENT_OPTIMAL,
                    .after_layout = daxa::ImageLayout::PRESENT_SRC,
                    .image_id = swapchain_image,
                });
                cmd_list.complete();
                device.submit_commands({
                    .command_lists = {std::move(cmd_list)},
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

            auto record_tasks() -> daxa::TaskList
            {
                daxa::TaskList new_task_list = daxa::TaskList({
                    .device = device,
                    .swapchain = swapchain,
                    //.dont_use_split_barriers = true,
                    .debug_name = APPNAME_PREFIX("main task list"),
                });
                task_swapchain_image = new_task_list.create_task_image({
                    .swapchain_image = true,
                    .debug_name = APPNAME_PREFIX("Task Swapchain Image"),
                });
                new_task_list.add_runtime_image(task_swapchain_image, swapchain_image);
                task_render_image = new_task_list.create_task_image({
                    .debug_name = APPNAME_PREFIX("Task Render Image"),
                });
                new_task_list.add_runtime_image(task_render_image, render_image);
                task_mipmapping_gpu_input_buffer = new_task_list.create_task_buffer({});
                new_task_list.add_runtime_buffer(task_mipmapping_gpu_input_buffer, mipmapping_gpu_input_buffer);
                new_task_list.add_task({
                    .used_buffers = {
                        {task_mipmapping_gpu_input_buffer, daxa::TaskBufferAccess::TRANSFER_WRITE},
                    },
                    .task = [this](daxa::TaskRuntime runtime)
                    {
                        auto cmd_list = runtime.get_command_list();
                        auto input_buffer = runtime.get_buffers(task_mipmapping_gpu_input_buffer)[0];
                        cmd_list.pipeline_barrier({
                            .awaited_pipeline_access = daxa::AccessConsts::READ_WRITE,
                            .waiting_pipeline_access = daxa::AccessConsts::READ_WRITE,
                        });
                        update_gpu_input(cmd_list, input_buffer);
                        cmd_list.pipeline_barrier({
                            .awaited_pipeline_access = daxa::AccessConsts::READ_WRITE,
                            .waiting_pipeline_access = daxa::AccessConsts::READ_WRITE,
                        });
                    },
                    .debug_name = APPNAME_PREFIX("Input Transfer"),
                });
                new_task_list.add_task(daxa::TaskInfo{
                    .used_buffers = {
                        {task_mipmapping_gpu_input_buffer, daxa::TaskBufferAccess::COMPUTE_SHADER_READ_ONLY},
                    },
                    .used_images = {
                        {task_render_image, daxa::TaskImageAccess::COMPUTE_SHADER_READ_WRITE, daxa::ImageMipArraySlice{}},
                    },
                    .task = [=, this](daxa::TaskRuntime const & runtime)
                    {
                        if (mouse_drawing)
                        {
                            auto cmd_list = runtime.get_command_list();
                            auto render_target_id = runtime.get_images(task_render_image)[0];
                            auto input_buffer = runtime.get_buffers(task_mipmapping_gpu_input_buffer)[0];
                            paint(cmd_list, render_target_id, input_buffer);
                        }
                    },
                    .debug_name = "mouse paint",
                });
                {
                    auto image_info = device.info_image(render_image);
                    std::array<i32, 3> mip_size = {std::max<i32>(1, static_cast<i32>(image_info.size.x)), std::max<i32>(1, static_cast<i32>(image_info.size.y)), std::max<i32>(1, static_cast<i32>(image_info.size.z))};
                    for (u32 i = 0; i < image_info.mip_level_count - 1; ++i)
                    {
                        std::array<i32, 3> next_mip_size = {std::max<i32>(1, mip_size[0] / 2), std::max<i32>(1, mip_size[1] / 2), std::max<i32>(1, mip_size[2] / 2)};
                        new_task_list.add_task({
                            .used_images = {
                                {task_render_image, daxa::TaskImageAccess::TRANSFER_READ, daxa::ImageMipArraySlice{.base_mip_level = i}},
                                {task_render_image, daxa::TaskImageAccess::TRANSFER_WRITE, daxa::ImageMipArraySlice{.base_mip_level = i + 1}},
                            },
                            .task = [=, this](daxa::TaskRuntime const & runtime)
                            {
                                auto cmd_list = runtime.get_command_list();
                                auto image_id = runtime.get_images(task_render_image)[0];
                                cmd_list.blit_image_to_image({
                                    .src_image = image_id,
                                    .src_image_layout = daxa::ImageLayout::TRANSFER_SRC_OPTIMAL, // TODO: get from TaskRuntime
                                    .dst_image = image_id,
                                    .dst_image_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
                                    .src_slice = {
                                        .image_aspect = image_info.aspect,
                                        .mip_level = i,
                                        .base_array_layer = 0,
                                        .layer_count = 1,
                                    },
                                    .src_offsets = {{{0, 0, 0}, {mip_size[0], mip_size[1], mip_size[2]}}},
                                    .dst_slice = {
                                        .image_aspect = image_info.aspect,
                                        .mip_level = i + 1,
                                        .base_array_layer = 0,
                                        .layer_count = 1,
                                    },
                                    .dst_offsets = {{{0, 0, 0}, {next_mip_size[0], next_mip_size[1], next_mip_size[2]}}},
                                    .filter = daxa::Filter::LINEAR,
                                });
                            },
                            .debug_name = "mip_level_" + std::to_string(i),
                        });
                        mip_size = next_mip_size;
                    }
                }
                new_task_list.add_task({
                    .used_images = {
                        {task_swapchain_image, daxa::TaskImageAccess::TRANSFER_WRITE, daxa::ImageMipArraySlice{}},
                    },
                    .task = [=, this](daxa::TaskRuntime const & runtime)
                    {
                        auto cmd_list = runtime.get_command_list();
                        cmd_list.clear_image({
                            .dst_image_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
                            .clear_value = {std::array<f32, 4>{1, 0, 1, 1}},
                            .dst_image = swapchain_image,
                        });
                    },
                    .debug_name = "clear swapchain",
                });
                new_task_list.add_task({
                    .used_images = {
                        {task_render_image, daxa::TaskImageAccess::TRANSFER_READ, daxa::ImageMipArraySlice{.level_count = 5}},
                        {task_swapchain_image, daxa::TaskImageAccess::TRANSFER_WRITE, daxa::ImageMipArraySlice{}},
                    },
                    .task = [=, this](daxa::TaskRuntime const & runtime)
                    {
                        auto cmd_list = runtime.get_command_list();
                        auto src_image_id = runtime.get_images(task_render_image)[0];
                        auto dst_image_id = runtime.get_images(task_swapchain_image)[0];
                        blit_image_to_swapchain(cmd_list, src_image_id, dst_image_id);
                    },
                    .debug_name = "blit to swapchain",
                });
                new_task_list.add_task({
                    .used_images = {
                        {task_swapchain_image, daxa::TaskImageAccess::COLOR_ATTACHMENT, daxa::ImageMipArraySlice{}},
                    },
                    .task = [=, this](daxa::TaskRuntime const & runtime)
                    {
                        auto cmd_list = runtime.get_command_list();
                        auto render_target_id = runtime.get_images(task_swapchain_image)[0];
                        draw_ui(cmd_list, render_target_id);
                    },
                    .debug_name = "Imgui",
                });
                new_task_list.submit(&submit_info);
                new_task_list.present({});
                new_task_list.complete();
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
} // namespace tests
