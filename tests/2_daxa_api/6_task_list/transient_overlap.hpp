#pragma once

#include "common.hpp"

namespace tests
{
    void transient_permutation_overlap()
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
            daxa::TaskImage task_swapchain_image{{
                .swapchain_image = true,
                .name = "swapchain",
            }};

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
        
            bool upload_settings = {};
            CondSettings settings = {};
            daxa::BufferId settings_buffer = device.create_buffer({
                .size = sizeof(CondSettings),
                .allocate_info = daxa::AutoAllocInfo{daxa::MemoryFlagBits::DEDICATED_MEMORY},
                .name = "settings",
            });
            daxa::TaskBuffer task_settings_buffer = daxa::TaskBuffer{{
                .initial_buffers = { .buffers = { &settings_buffer, 1 } },
                .name = "settings",
            }};
        
            daxa::BufferId vertex_buffer = device.create_buffer({.size = sizeof(DrawVertex) * 6, .name = "vertex buffer"});
            daxa::TaskBuffer task_vertex_buffer = daxa::TaskBuffer{{
                .initial_buffers = {.buffers = {&vertex_buffer,1}},
                .name = "vertex buffer",
            }};
        
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

                new_task_list.use_persistent_buffer(task_vertex_buffer);
                new_task_list.use_persistent_buffer(task_settings_buffer);
                new_task_list.use_persistent_image(task_swapchain_image);
        
                new_task_list.conditional({
                    .condition_index = CONDITION_FIRST_FRAME,
                    .when_true = [&]()
                    {
                        new_task_list.add_task({
                            .used_buffers = {
                                daxa::TaskBufferUse{ task_vertex_buffer, daxa::TaskBufferAccess::TRANSFER_WRITE }
                            },
                            .task = [this](daxa::TaskRuntimeInterface const & tri)
                            {
                                auto cmd = tri.get_command_list();
                                daxa::TransferMemoryPool::Allocation alloc = tri.get_allocator().allocate(sizeof(DrawVertex) * 6).value();
                                std::array<DrawVertex, 6> vertices = {
                                    
                                };
                                std::memcpy(alloc.host_address, vertices.data(), sizeof(DrawVertex) * 6);
                                cmd.copy_buffer_to_buffer({
                                    .src_buffer = tri.get_allocator().get_buffer(),
                                    .src_offset = alloc.buffer_offset,
                                    .dst_buffer = tri.get_buffers(task_vertex_buffer)[0],
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
                                daxa::TaskBufferUse{task_settings_buffer, daxa::TaskBufferAccess::TRANSFER_WRITE},
                            },
                            .task = [=, this](daxa::TaskRuntimeInterface const & tri)
                            {
                                auto cmd = tri.get_command_list();
                                daxa::TransferMemoryPool::Allocation alloc = tri.get_allocator().allocate(sizeof(CondSettings)).value();
                                *reinterpret_cast<CondSettings*>(alloc.host_address) = this->settings;
                                cmd.copy_buffer_to_buffer({
                                    .src_buffer = tri.get_allocator().get_buffer(),
                                    .src_offset = alloc.buffer_offset,
                                    .dst_buffer = tri.get_buffers(task_settings_buffer)[0],
                                    .size = sizeof(CondSettings),
                                });
                            },
                        });
                    },
                });
        
                new_task_list.add_task({
                    .used_buffers = {
                        {task_vertex_buffer, daxa::TaskBufferAccess::VERTEX_SHADER_READ_ONLY},
                        {task_settings_buffer, daxa::TaskBufferAccess::FRAGMENT_SHADER_READ_ONLY},
                    },
                    .used_images = {
                        {task_swapchain_image, daxa::TaskImageAccess::FRAGMENT_SHADER_WRITE_ONLY},
                    },
                    .task = [=, this](daxa::TaskRuntimeInterface tri)
                    {
                        auto cmd_list = tri.get_command_list();
                        auto const local_swapchain_image = tri.get_images(task_swapchain_image)[0];
                        cmd_list.begin_renderpass({
                            .color_attachments = {{.image_view = local_swapchain_image.default_view()}},
                            .render_area = {.x = 0, .y = 0, .width = size_x, .height = size_y},
                        });
                        cmd_list.set_pipeline(*raster_pipeline);
                        cmd_list.push_constant(DrawPush{
                            .vertex_buffer = this->device.get_device_address(tri.get_buffers(task_vertex_buffer)[0]),
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
                device.destroy_buffer(settings_buffer);
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
        
                auto swapchain_image = swapchain.acquire_next_image();
                task_swapchain_image.set_images({.images = {&swapchain_image, 1}});
        
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
                    settings.value = 1;
                    upload_settings = true;
                }
                else if (key == GLFW_KEY_B && action == GLFW_RELEASE)
                {
                    settings.value = 0;
                    upload_settings = true;
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
            app.upload_settings = false;
            if (app.update())
            {
                break;
            }
        }
    }
}

