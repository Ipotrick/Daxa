#include <0_common/window.hpp>
#include <thread>
#include <iostream>

#define DAXA_GLSL 1
#define DAXA_HLSL 0

#include "shaders/shared.inl"

#include <daxa/utils/imgui.hpp>
#include <0_common/imgui/imgui_impl_glfw.h>

#define APPNAME "Daxa Sample: HelloTriangle"
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
        .present_mode = daxa::PresentMode::DOUBLE_BUFFER_WAIT_FOR_VBLANK,
        .image_usage = daxa::ImageUsageFlagBits::TRANSFER_DST,
        .debug_name = APPNAME_PREFIX("swapchain"),
    });

    daxa::PipelineCompiler pipeline_compiler = device.create_pipeline_compiler({
        .shader_compile_options = {
            .root_paths = {
                "tests/3_samples/2_hello_triangle/shaders",
                "include",
            },
#if DAXA_GLSL
            .language = daxa::ShaderLanguage::GLSL,
#elif DAXA_HLSL
            .language = daxa::ShaderLanguage::HLSL,
#endif
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
    daxa::RasterPipeline raster_pipeline = pipeline_compiler.create_raster_pipeline({
#if DAXA_GLSL
        .vertex_shader_info = {.source = daxa::ShaderFile{"draw.glsl"}, .compile_options = {.defines = {daxa::ShaderDefine{"DRAW_VERT"}}}},
        .fragment_shader_info = {.source = daxa::ShaderFile{"draw.glsl"}, .compile_options = {.defines = {daxa::ShaderDefine{"DRAW_FRAG"}}}},
#elif DAXA_HLSL
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

    static inline constexpr u64 FRAMES_IN_FLIGHT = 1;
    daxa::TimelineSemaphore gpu_framecount_timeline_sema = device.create_timeline_semaphore(daxa::TimelineSemaphoreInfo{
        .initial_value = 0,
        .debug_name = APPNAME_PREFIX("gpu_framecount_timeline_sema"),
    });
    u64 cpu_framecount = FRAMES_IN_FLIGHT - 1;

    daxa::BinarySemaphore acquire_semaphore = device.create_binary_semaphore({.debug_name = APPNAME_PREFIX("acquire_semaphore")});
    daxa::BinarySemaphore present_semaphore = device.create_binary_semaphore({.debug_name = APPNAME_PREFIX("present_semaphore")});

    App() : AppWindow<App>(APPNAME) {}

    ~App()
    {
        device.wait_idle();
        device.collect_garbage();
        ImGui_ImplGlfw_Shutdown();
        device.destroy_buffer(vertex_buffer);
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
        ImGui::Begin("Debug");
        ImGui::End();
        ImGui::ShowDemoWindow();
        ImGui::Render();
    }

    void draw()
    {
        ui_update();

        if (pipeline_compiler.check_if_sources_changed(raster_pipeline))
        {
            auto new_pipeline = pipeline_compiler.recreate_raster_pipeline(raster_pipeline);
            std::cout << new_pipeline.to_string() << std::endl;
            if (new_pipeline.is_ok())
            {
                raster_pipeline = new_pipeline.value();
            }
        }

        auto swapchain_image = swapchain.acquire_next_image(acquire_semaphore);

        auto cmd_list = device.create_command_list({
            .debug_name = APPNAME_PREFIX("cmd_list"),
        });

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

        cmd_list.pipeline_barrier({
            .awaited_pipeline_access = daxa::AccessConsts::HOST_WRITE,
            .waiting_pipeline_access = daxa::AccessConsts::TRANSFER_READ,
        });

        cmd_list.copy_buffer_to_buffer({
            .src_buffer = vertex_staging_buffer,
            .dst_buffer = vertex_buffer,
            .size = sizeof(DrawVertex) * 3,
        });

        cmd_list.pipeline_barrier({
            .awaited_pipeline_access = daxa::AccessConsts::TRANSFER_WRITE,
            .waiting_pipeline_access = daxa::AccessConsts::VERTEX_SHADER_READ,
        });

        cmd_list.pipeline_barrier_image_transition({
            .waiting_pipeline_access = daxa::AccessConsts::COLOR_ATTACHMENT_OUTPUT_WRITE,
            .before_layout = daxa::ImageLayout::UNDEFINED,
            .after_layout = daxa::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,
            .image_id = swapchain_image,
        });

        cmd_list.begin_renderpass({
            .color_attachments = {{.image_view = swapchain_image.default_view(), .load_op = daxa::AttachmentLoadOp::CLEAR, .clear_value = std::array<f32, 4>{0.1f, 0.0f, 0.5f, 1.0f}}},
            .render_area = {.x = 0, .y = 0, .width = size_x, .height = size_y},
        });
        cmd_list.set_pipeline(raster_pipeline);
        cmd_list.push_constant(DrawPush {
#if DAXA_GLSL
            .face_buffer = this->device.buffer_reference(vertex_buffer),
#elif DAXA_HLSL
                .vertex_buffer_id = vertex_buffer,
#endif
        });
        cmd_list.draw({.vertex_count = 3});
        cmd_list.end_renderpass();

        imgui_renderer.record_commands(ImGui::GetDrawData(), cmd_list, swapchain_image, size_x, size_y);

        cmd_list.pipeline_barrier_image_transition({
            .awaited_pipeline_access = daxa::AccessConsts::ALL_GRAPHICS_READ_WRITE,
            .before_layout = daxa::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,
            .after_layout = daxa::ImageLayout::PRESENT_SRC,
            .image_id = swapchain_image,
        });

        cmd_list.complete();

        ++cpu_framecount;
        device.submit_commands({
            .command_lists = {std::move(cmd_list)},
            .wait_binary_semaphores = {acquire_semaphore},
            .signal_binary_semaphores = {present_semaphore},
            .signal_timeline_semaphores = {{gpu_framecount_timeline_sema, cpu_framecount}},
        });

        device.present_frame({
            .wait_binary_semaphores = {present_semaphore},
            .swapchain = swapchain,
        });

        gpu_framecount_timeline_sema.wait_for_value(cpu_framecount - 1);
    }

    void on_mouse_move(f32, f32)
    {
    }

    void on_key(int, int)
    {
    }

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
