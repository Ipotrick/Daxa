#define DAXA_SHADERLANG DAXA_SHADERLANG_GLSL
#define APPNAME "Daxa Sample: HelloTriangle Compute"
#include <0_common/base_app.hpp>

using namespace daxa::types;
#include "shaders/shared.inl"

struct App : BaseApp<App>
{
    // clang-format off
    daxa::ComputePipeline compute_pipeline = pipeline_compiler.create_compute_pipeline({
#if DAXA_SHADERLANG == DAXA_SHADERLANG_GLSL
        .shader_info = {.source = daxa::ShaderFile{"compute.glsl"}},
#elif DAXA_SHADERLANG == DAXA_SHADERLANG_HLSL
        .shader_info = {.source = daxa::ShaderFile{"compute.hlsl"}},
#endif
        .push_constant_size = sizeof(ComputePush),
        .debug_name = APPNAME_PREFIX("compute_pipeline"),
    }).value();
    // clang-format on

    daxa::ImageId render_image = device.create_image(daxa::ImageInfo{
        .format = daxa::Format::R8G8B8A8_UNORM,
        .size = {size_x, size_y, 1},
        .usage = daxa::ImageUsageFlagBits::SHADER_READ_WRITE | daxa::ImageUsageFlagBits::TRANSFER_SRC,
        .debug_name = APPNAME_PREFIX("render_image"),
    });
    daxa::TaskImageId task_render_image;

    daxa::TaskList loop_task_list = record_loop_task_list();

    ~App()
    {
        device.wait_idle();
        device.collect_garbage();
        device.destroy_image(render_image);
    }

    void ui_update()
    {
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::Render();
    }
    void on_update()
    {
        reload_pipeline(compute_pipeline);
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
            device.destroy_image(render_image);
            loop_task_list.remove_runtime_image(task_render_image, render_image);
            render_image = device.create_image({
                .format = daxa::Format::R8G8B8A8_UNORM,
                .size = {size_x, size_y, 1},
                .usage = daxa::ImageUsageFlagBits::SHADER_READ_WRITE | daxa::ImageUsageFlagBits::TRANSFER_SRC,
            });
            loop_task_list.add_runtime_image(task_render_image, render_image);
            base_on_update();
        }
    }

    void record_tasks(daxa::TaskList & new_task_list)
    {
        task_render_image = new_task_list.create_task_image({.debug_name = APPNAME_PREFIX("task_render_image")});
        new_task_list.add_runtime_image(task_render_image, render_image);

        new_task_list.add_task({
            .used_images = {
                {task_render_image, daxa::TaskImageAccess::COMPUTE_SHADER_WRITE_ONLY, daxa::ImageMipArraySlice{}},
            },
            .task = [this](daxa::TaskRuntime runtime)
            {
                auto cmd_list = runtime.get_command_list();
                cmd_list.set_pipeline(compute_pipeline);
                cmd_list.push_constant(ComputePush{
                    .image_id = render_image.default_view(),
                    .frame_dim = {size_x, size_y},
                });
                cmd_list.dispatch((size_x + 7) / 8, (size_y + 7) / 8);
            },
            .debug_name = APPNAME_PREFIX("Draw (Compute)"),
        });
        new_task_list.add_task({
            .used_images = {
                {task_render_image, daxa::TaskImageAccess::TRANSFER_READ, daxa::ImageMipArraySlice{}},
                {task_swapchain_image, daxa::TaskImageAccess::TRANSFER_WRITE, daxa::ImageMipArraySlice{}},
            },
            .task = [this](daxa::TaskRuntime runtime)
            {
                auto cmd_list = runtime.get_command_list();
                cmd_list.blit_image_to_image({
                    .src_image = render_image,
                    .src_image_layout = daxa::ImageLayout::TRANSFER_SRC_OPTIMAL,
                    .dst_image = swapchain_image,
                    .dst_image_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
                    .src_slice = {.image_aspect = daxa::ImageAspectFlagBits::COLOR},
                    .src_offsets = {{{0, 0, 0}, {static_cast<i32>(size_x), static_cast<i32>(size_y), 1}}},
                    .dst_slice = {.image_aspect = daxa::ImageAspectFlagBits::COLOR},
                    .dst_offsets = {{{0, 0, 0}, {static_cast<i32>(size_x), static_cast<i32>(size_y), 1}}},
                });
            },
            .debug_name = APPNAME_PREFIX("Blit (render to swapchain)"),
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
