#include <0_common/window.hpp>
#include <0_common/player.hpp>
#include <thread>
#include <iostream>

#include "shaders/shared.inl"

#include <daxa/utils/imgui.hpp>
#include <imgui_impl_glfw.h>

#include <cgltf.h>

#define APPNAME "Daxa Sample: Sponza"
#define APPNAME_PREFIX(x) ("[" APPNAME "] " x)

using namespace daxa::types;
using Clock = std::chrono::high_resolution_clock;

struct App : AppWindow<App>
{
    daxa::Context daxa_ctx = daxa::create_context({
        .enable_validation = true,
    });
    daxa::Device device = daxa_ctx.create_device({
        .debug_name = APPNAME_PREFIX("device"),
    });

    daxa::Swapchain swapchain = device.create_swapchain({
        .native_window = get_native_handle(),
        .native_window_platform = get_native_platform(),
        .present_mode = daxa::PresentMode::DO_NOT_WAIT_FOR_VBLANK,
        .image_usage = daxa::ImageUsageFlagBits::TRANSFER_DST,
        .debug_name = APPNAME_PREFIX("swapchain"),
    });

    daxa::PipelineCompiler pipeline_compiler = device.create_pipeline_compiler({
        .shader_compile_options = {
            .root_paths = {
                "tests/3_samples/6_sponza/shaders",
                "include",
            },
            .language = daxa::ShaderLanguage::GLSL,
        },
        .debug_name = APPNAME_PREFIX("pipeline_compiler"),
    });

    // daxa::ImGuiRenderer imgui_renderer = create_imgui_renderer();
    // auto create_imgui_renderer() -> daxa::ImGuiRenderer
    // {
    //     ImGui::CreateContext();
    //     ImGui_ImplGlfw_InitForVulkan(glfw_window_ptr, true);
    //     return daxa::ImGuiRenderer({
    //         .device = device,
    //         .pipeline_compiler = pipeline_compiler,
    //         .format = swapchain.get_format(),
    //     });
    // }

    // clang-format off
    daxa::RasterPipeline raster_pipeline = pipeline_compiler.create_raster_pipeline({
        .vertex_shader_info = {.source = daxa::ShaderFile{"draw.glsl"}, .compile_options = {.defines = {daxa::ShaderDefine{"DRAW_VERT"}}}},
        .fragment_shader_info = {.source = daxa::ShaderFile{"draw.glsl"}, .compile_options = {.defines = {daxa::ShaderDefine{"DRAW_FRAG"}}}},
        .color_attachments = {{.format = swapchain.get_format()}},
        .raster = {},
        .push_constant_size = sizeof(DrawPush),
        .debug_name = APPNAME_PREFIX("raster_pipeline"),
    }).value();
    // clang-format on

    struct Model
    {
        daxa::BufferId vbuffer;
        u32 vcount;
        daxa::BufferId ibuffer;
        u32 icount;
    };

    daxa::BufferId gpu_input_buffer = device.create_buffer(daxa::BufferInfo{
        .size = sizeof(GpuInput),
        .debug_name = APPNAME_PREFIX("gpu_input_buffer"),
    });
    GpuInput gpu_input = {};

    std::vector<std::optional<daxa::BufferId>> model_buffers;
    std::vector<Model> models;

    Clock::time_point start = Clock::now(), prev_time = start;
    Player3D player = {
        .rot = {0.0f, 0.0f, 0.0f},
    };
    bool paused = true;

    auto load_buffer(daxa::CommandList & cmd_list, cgltf_accessor & accessor, bool is_vbuffer) -> daxa::BufferId
    {
        auto stride = is_vbuffer ? accessor.stride : sizeof(u32);
        auto size = static_cast<u32>(stride * accessor.count);
        // auto size = static_cast<u32>(is_vbuffer ? (3 * sizeof(DrawVertex)) : (3 * sizeof(u32)));

        auto result = device.create_buffer(daxa::BufferInfo{
            .size = size,
            .debug_name = APPNAME_PREFIX("model buffer"),
        });

        auto staging_buffer = device.create_buffer({
            .memory_flags = daxa::MemoryFlagBits::HOST_ACCESS_RANDOM,
            .size = size,
            .debug_name = APPNAME_PREFIX("model staging_buffer"),
        });
        cmd_list.destroy_buffer_deferred(staging_buffer);

        void * cpu_buffer_ptr = reinterpret_cast<void *>(reinterpret_cast<u8 *>(accessor.buffer_view->buffer->data) + accessor.buffer_view->offset + accessor.offset);

        // if (is_vbuffer)
        // {
        //     auto buffer_ptr = device.map_memory_as<DrawVertex>(staging_buffer);
        //     *buffer_ptr = DrawVertex{-0.5f, +0.5f, 0.0f};
        //     ++buffer_ptr;
        //     *buffer_ptr = DrawVertex{+0.5f, +0.5f, 0.0f};
        //     ++buffer_ptr;
        //     *buffer_ptr = DrawVertex{+0.0f, -0.5f, 0.0f};
        //     ++buffer_ptr;
        // }
        // else
        // {
        //     auto buffer_ptr = device.map_memory_as<u32>(staging_buffer);
        //     *buffer_ptr = 0;
        //     ++buffer_ptr;
        //     *buffer_ptr = 1;
        //     ++buffer_ptr;
        //     *buffer_ptr = 2;
        //     ++buffer_ptr;
        // }

        auto buffer_ptr = device.map_memory_as<u32>(staging_buffer);
        switch (stride)
        {
        case 1:
            for (size_t i = 0; i < accessor.count; i++)
                buffer_ptr[i] = (reinterpret_cast<u8 *>(cpu_buffer_ptr))[i];
            break;
        case 2:
            for (size_t i = 0; i < accessor.count; i++)
                buffer_ptr[i] = (reinterpret_cast<u16 *>(cpu_buffer_ptr))[i];
            break;
        case 4:
            for (size_t i = 0; i < accessor.count; i++)
                buffer_ptr[i] = (reinterpret_cast<u32 *>(cpu_buffer_ptr))[i];
            break;
        case 8:
            for (size_t i = 0; i < accessor.count; i++)
                buffer_ptr[i] = static_cast<u32>(reinterpret_cast<u64 *>(cpu_buffer_ptr)[i]);
            break;
        }

        device.unmap_memory(staging_buffer);

        cmd_list.pipeline_barrier({
            .awaited_pipeline_access = daxa::AccessConsts::HOST_WRITE,
            .waiting_pipeline_access = daxa::AccessConsts::TRANSFER_READ,
        });
        cmd_list.copy_buffer_to_buffer({
            .src_buffer = staging_buffer,
            .dst_buffer = result,
            .size = size,
        });
        cmd_list.pipeline_barrier({
            .awaited_pipeline_access = daxa::AccessConsts::TRANSFER_WRITE,
            .waiting_pipeline_access = daxa::AccessConsts::VERTEX_SHADER_READ,
        });

        return result;
    }

    void import_node(daxa::CommandList & cmd_list, cgltf_node * node, cgltf_data * data)
    {
        if (node->mesh)
        {
            auto & root_mesh = *node->mesh;
            for (usize prim_i = 0; prim_i < 1; /*root_mesh.primitives_count;*/ ++prim_i)
            {
                Model result;

                auto & prim = root_mesh.primitives[prim_i];
                result.icount = static_cast<u32>(prim.indices->count);
                usize buffer_i = static_cast<usize>(std::distance(data->accessors, prim.indices));

                if (!model_buffers[buffer_i].has_value())
                {
                    model_buffers[buffer_i] = load_buffer(cmd_list, *prim.indices, false);
                }
                result.ibuffer = model_buffers[buffer_i].value();
                bool has_vbuffer = false;

                for (usize attr_i = 0; attr_i < prim.attributes_count; attr_i++)
                {
                    auto & attribute = prim.attributes[attr_i];
                    usize index_of_attrib = static_cast<usize>(std::distance(data->accessors, attribute.data));

                    switch (attribute.type)
                    {
                    case cgltf_attribute_type_color:
                        // printf("models dont support vertex colors\n");
                        break;
                    case cgltf_attribute_type_position:
                        if (!model_buffers[index_of_attrib].has_value())
                        {
                            model_buffers[index_of_attrib] = load_buffer(cmd_list, *attribute.data, true);
                        }
                        result.vbuffer = model_buffers[index_of_attrib].value();
                        has_vbuffer = true;
                        break;
                    case cgltf_attribute_type_texcoord:
                        // result.vertexUVs = model_buffers[index_of_attrib];
                        break;
                    case cgltf_attribute_type_normal:
                        // result.vertexNormals = model_buffers[index_of_attrib];
                        break;
                    case cgltf_attribute_type_tangent:
                        // result.vertexTangents = model_buffers[index_of_attrib];
                        break;
                    default: break;
                    }
                }

                if (has_vbuffer)
                    models.push_back(result);
            }
        }

        for (usize child_i = 0; child_i < node->children_count; ++child_i)
        {
            import_node(cmd_list, node->children[child_i], data);
        }
    }

    App() : AppWindow<App>(APPNAME)
    {
        auto path = "tests/0_common/assets/models/suzanne.glb";
        cgltf_options options = {};
        cgltf_data * data = nullptr;
        if (cgltf_parse_file(&options, path, &data) != cgltf_result_success)
        {
            cgltf_free(data);
            return;
        }
        if (cgltf_load_buffers(&options, data, path) != cgltf_result_success)
        {
            cgltf_free(data);
            return;
        }

        model_buffers.resize(data->accessors_count);

        auto cmd_list = device.create_command_list({});
        for (usize scene_i = 0; scene_i < data->scenes_count; ++scene_i)
        {
            auto & scene = data->scenes[scene_i];
            for (usize root_node_i = 0; root_node_i < scene.nodes_count; ++root_node_i)
            {
                auto * root_node = scene.nodes[root_node_i];
                import_node(cmd_list, root_node, data);
            }
        }
        cmd_list.complete();
        device.submit_commands({
            .command_lists = {std::move(cmd_list)},
        });
        device.wait_idle();
    }

    ~App()
    {
        device.wait_idle();
        device.collect_garbage();
        // ImGui_ImplGlfw_Shutdown();
        device.destroy_buffer(gpu_input_buffer);

        for (auto & buffer : model_buffers)
        {
            if (buffer.has_value())
                device.destroy_buffer(buffer.value());
        }
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
        // ImGui_ImplGlfw_NewFrame();
        // ImGui::NewFrame();
        // if (paused)
        // {
        //     ImGui::Begin("Debug");
        //     ImGui::End();
        //     ImGui::ShowDemoWindow();
        // }
        // ImGui::Render();
    }

    void draw()
    {
        ui_update();

        auto now = Clock::now();
        auto elapsed_s = std::chrono::duration<f32>(now - prev_time).count();
        prev_time = now;

        player.camera.resize(static_cast<i32>(size_x), static_cast<i32>(size_y));
        player.camera.set_pos(player.pos);
        player.camera.set_rot(player.rot.x, player.rot.y);
        player.update(elapsed_s);

        if (pipeline_compiler.check_if_sources_changed(raster_pipeline))
        {
            auto new_pipeline = pipeline_compiler.recreate_raster_pipeline(raster_pipeline);
            std::cout << new_pipeline.to_string() << std::endl;
            if (new_pipeline.is_ok())
            {
                raster_pipeline = new_pipeline.value();
            }
        }

        auto swapchain_image = swapchain.acquire_next_image();
        if (swapchain_image.is_empty())
        {
            return;
        }

        auto cmd_list = device.create_command_list({
            .debug_name = APPNAME_PREFIX("cmd_list"),
        });

        auto gpu_input_staging_buffer = device.create_buffer({
            .memory_flags = daxa::MemoryFlagBits::HOST_ACCESS_RANDOM,
            .size = sizeof(GpuInput),
            .debug_name = APPNAME_PREFIX("gpu_input_staging_buffer"),
        });
        cmd_list.destroy_buffer_deferred(gpu_input_staging_buffer);
        auto mat = player.camera.get_vp();
        gpu_input.view_mat = *reinterpret_cast<f32mat4x4 *>(&mat);
        auto buffer_ptr = device.map_memory_as<GpuInput>(gpu_input_staging_buffer);
        *buffer_ptr = gpu_input;
        device.unmap_memory(gpu_input_staging_buffer);

        cmd_list.pipeline_barrier({
            .awaited_pipeline_access = daxa::AccessConsts::HOST_WRITE,
            .waiting_pipeline_access = daxa::AccessConsts::TRANSFER_READ,
        });

        cmd_list.copy_buffer_to_buffer({
            .src_buffer = gpu_input_staging_buffer,
            .dst_buffer = gpu_input_buffer,
            .size = sizeof(GpuInput),
        });

        cmd_list.pipeline_barrier({
            .awaited_pipeline_access = daxa::AccessConsts::TRANSFER_WRITE,
            .waiting_pipeline_access = daxa::AccessConsts::VERTEX_SHADER_READ,
        });

        cmd_list.pipeline_barrier_image_transition({
            .waiting_pipeline_access = daxa::AccessConsts::COLOR_ATTACHMENT_OUTPUT_WRITE,
            .before_layout = daxa::ImageLayout::UNDEFINED,
            .after_layout = daxa::ImageLayout::ATTACHMENT_OPTIMAL,
            .image_id = swapchain_image,
        });

        cmd_list.begin_renderpass({
            .color_attachments = {{.image_view = swapchain_image.default_view(), .load_op = daxa::AttachmentLoadOp::CLEAR, .clear_value = std::array<f32, 4>{0.1f, 0.0f, 0.5f, 1.0f}}},
            .render_area = {.x = 0, .y = 0, .width = size_x, .height = size_y},
        });
        cmd_list.set_pipeline(raster_pipeline);

        for (auto const & model : models)
        {
            cmd_list.push_constant(DrawPush{
                .input_buffer = this->device.get_device_address(gpu_input_buffer),
                .vbuffer = this->device.get_device_address(model.vbuffer),
            });
            // cmd_list.draw({.vertex_count = 3});
            cmd_list.set_index_buffer(model.ibuffer, 0);
            cmd_list.draw_indexed({
                .index_count = model.icount,
            });
        }

        cmd_list.end_renderpass();

        // imgui_renderer.record_commands(ImGui::GetDrawData(), cmd_list, swapchain_image, size_x, size_y);

        cmd_list.pipeline_barrier_image_transition({
            .awaited_pipeline_access = daxa::AccessConsts::ALL_GRAPHICS_READ_WRITE,
            .before_layout = daxa::ImageLayout::ATTACHMENT_OPTIMAL,
            .after_layout = daxa::ImageLayout::PRESENT_SRC,
            .image_id = swapchain_image,
        });

        cmd_list.complete();

        device.submit_commands({
            .command_lists = {std::move(cmd_list)},
            .wait_binary_semaphores = {swapchain.get_acquire_semaphore()},
            .signal_binary_semaphores = {swapchain.get_present_semaphore()},
            .signal_timeline_semaphores = {{swapchain.get_gpu_timeline_semaphore(), swapchain.get_cpu_timeline_value()}},
        });

        device.present_frame({
            .wait_binary_semaphores = {swapchain.get_present_semaphore()},
            .swapchain = swapchain,
        });
    }

    void on_mouse_move(f32 x, f32 y)
    {
        if (!paused)
        {
            f32 center_x = static_cast<f32>(size_x / 2);
            f32 center_y = static_cast<f32>(size_y / 2);
            auto offset = f32vec2{x - center_x, center_y - y};
            player.on_mouse_move(offset.x, offset.y);
            set_mouse_pos(center_x, center_y);
        }
    }

    void on_mouse_button(i32, i32) {}

    void on_key(i32 key, i32 action)
    {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        {
            toggle_pause();
        }

        if (!paused)
        {
            player.on_key(key, action);
        }
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

    void toggle_pause()
    {
        set_mouse_capture(paused);
        paused = !paused;
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
