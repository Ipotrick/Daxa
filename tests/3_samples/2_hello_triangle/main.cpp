#define DAXA_SHADERLANG DAXA_SHADERLANG_HLSL
#define APPNAME "Daxa Sample: HelloTriangle"
#include <0_common/base_app.hpp>

using namespace daxa::types;
#include "shaders/shared.inl"

struct App : BaseApp<App>
{
    // clang-format off
    daxa::RasterPipeline raster_pipeline = pipeline_compiler.create_raster_pipeline({
#if DAXA_SHADERLANG == DAXA_SHADERLANG_GLSL
        .vertex_shader_info = {.source = daxa::ShaderFile{"draw.glsl"}, .compile_options = {.defines = {daxa::ShaderDefine{"DRAW_VERT"}}}},
        .fragment_shader_info = {.source = daxa::ShaderFile{"draw.glsl"}, .compile_options = {.defines = {daxa::ShaderDefine{"DRAW_FRAG"}}}},
#elif DAXA_SHADERLANG == DAXA_SHADERLANG_HLSL
        .vertex_shader_info = {.source = daxa::ShaderFile{"draw.hlsl"}, .compile_options = {.entry_point = "vs_main"}},
        .fragment_shader_info = {.source = daxa::ShaderFile{"draw.hlsl"}, .compile_options = {.entry_point = "fs_main"}},
#endif
        .color_attachments = {{.format = swapchain.get_format()}},
        .raster = {},
        .push_constant_size = sizeof(DrawPush),
        .debug_name = APPNAME_PREFIX("raster_pipeline"),
    }).value();
    // clang-format on

    daxa::BufferId vertex_buffer = device.create_buffer(daxa::BufferInfo{
        .size = sizeof(DrawVertex) * 3,
        .debug_name = APPNAME_PREFIX("vertex_buffer"),
    });
    daxa::TaskBufferId task_vertex_buffer;

    daxa::TaskList loop_task_list = record_loop_task_list();

    ~App()
    {
        device.wait_idle();
        device.collect_garbage();
        device.destroy_buffer(vertex_buffer);
    }

    void ui_update()
    {
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::Begin("Debug");
        ImGui::End();
        ImGui::ShowDemoWindow();
        ImGui::Render();
    }
    void on_update()
    {
        reload_pipeline(raster_pipeline);
        ui_update();

        loop_task_list.remove_runtime_image(task_swapchain_image, swapchain_image);
        swapchain_image = swapchain.acquire_next_image();
        loop_task_list.add_runtime_image(task_swapchain_image, swapchain_image);
        if (swapchain_image.is_empty())
            return;
        loop_task_list.execute();
    }

    void on_mouse_move(f32, f32) {}
    void on_mouse_button(i32, i32) {}
    void on_key(i32, i32) {}
    void on_resize(u32 sx, u32 sy)
    {
        minimized = (sx == 0 || sy == 0);
        if (!minimized)
        {
            swapchain.resize();
            size_x = swapchain.get_surface_extent().x;
            size_y = swapchain.get_surface_extent().y;
            base_on_update();
        }
    }

    void record_tasks(daxa::TaskList & new_task_list)
    {
        task_vertex_buffer = new_task_list.create_task_buffer({.debug_name = APPNAME_PREFIX("task_vertex_buffer")});
        new_task_list.add_runtime_buffer(task_vertex_buffer, vertex_buffer);
        new_task_list.add_task({
            .used_buffers = {
                {task_vertex_buffer, daxa::TaskBufferAccess::VERTEX_SHADER_READ_ONLY},
            },
            .task = [this](daxa::TaskRuntime runtime)
            {
                auto cmd_list = runtime.get_command_list();
                auto vertex_staging_buffer = device.create_buffer({
                    .memory_flags = daxa::MemoryFlagBits::HOST_ACCESS_RANDOM,
                    .size = sizeof(DrawVertex) * 3,
                    .debug_name = APPNAME_PREFIX("vertex_staging_buffer"),
                });
                cmd_list.destroy_buffer_deferred(vertex_staging_buffer);
                auto buffer_ptr = device.map_memory_as<DrawVertex>(vertex_staging_buffer);
                *buffer_ptr = DrawVertex{{-0.5f, +0.5f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}};
                ++buffer_ptr;
                *buffer_ptr = DrawVertex{{+0.5f, +0.5f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}};
                ++buffer_ptr;
                *buffer_ptr = DrawVertex{{+0.0f, -0.5f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}};
                ++buffer_ptr;
                device.unmap_memory(vertex_staging_buffer);
                cmd_list.copy_buffer_to_buffer({
                    .src_buffer = vertex_staging_buffer,
                    .dst_buffer = vertex_buffer,
                    .size = sizeof(DrawVertex) * 3,
                });
            },
            .debug_name = APPNAME_PREFIX("Upload vertices"),
        });

        new_task_list.add_task({
            .used_images = {
                {task_swapchain_image, daxa::TaskImageAccess::COLOR_ATTACHMENT, daxa::ImageMipArraySlice{}},
            },
            .task = [this](daxa::TaskRuntime runtime)
            {
                auto cmd_list = runtime.get_command_list();
                cmd_list.begin_renderpass({
                    .color_attachments = {{.image_view = swapchain_image.default_view(), .load_op = daxa::AttachmentLoadOp::CLEAR, .clear_value = std::array<f32, 4>{0.1f, 0.0f, 0.5f, 1.0f}}},
                    .render_area = {.x = 0, .y = 0, .width = size_x, .height = size_y},
                });
                cmd_list.set_pipeline(raster_pipeline);
                cmd_list.push_constant(DrawPush {
#if DAXA_SHADERLANG == DAXA_SHADERLANG_GLSL
                    .face_buffer = this->device.get_device_address(vertex_buffer),
#elif DAXA_SHADERLANG == DAXA_SHADERLANG_HLSL
                    .vertex_buffer_id = vertex_buffer,
#endif
                });
                cmd_list.draw({.vertex_count = 3});
                cmd_list.end_renderpass();
            },
            .debug_name = APPNAME_PREFIX("Draw to swapchain"),
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
