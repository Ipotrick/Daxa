#include <0_common/window.hpp>
#include <thread>

#define APPNAME "Daxa API Sample: Swapchain"
#define APPNAME_PREFIX(x) ("[" APPNAME "] " x)

namespace tests
{
    void simple_creation()
    {
        struct App : AppWindow<App>
        {
            daxa::Context daxa_ctx = daxa::create_context({
                .enable_validation = true,
            });
            daxa::Device device = daxa_ctx.create_device({
                .name = APPNAME_PREFIX("device (simple_creation)"),
            });

            daxa::Swapchain swapchain = device.create_swapchain({
                .native_window = get_native_handle(),
                .native_window_platform = get_native_platform(),
                .present_mode = daxa::PresentMode::FIFO,
                .image_usage = daxa::ImageUsageFlagBits::TRANSFER_DST,
                .name = APPNAME_PREFIX("swapchain (simple_creation)"),
            });

            App() : AppWindow<App>(APPNAME " (simple_creation)") {}

            void on_mouse_move(f32 /*unused*/, f32 /*unused*/) {}
            void on_mouse_button(i32 /*unused*/, i32 /*unused*/) {}
            void on_key(i32 /*unused*/, i32 /*unused*/) {}
            void on_resize(u32 /*unused*/, u32 /*unused*/) {}
        };
        App const app;
    }

    void clearcolor()
    {
        struct App : AppWindow<App>
        {
            daxa::Context daxa_ctx = daxa::create_context({
                .enable_validation = true,
            });
            daxa::Device device = daxa_ctx.create_device({
                .name = APPNAME_PREFIX("device (clearcolor)"),
            });

            daxa::Swapchain swapchain = device.create_swapchain({
                .native_window = get_native_handle(),
                .native_window_platform = get_native_platform(),
                .image_usage = daxa::ImageUsageFlagBits::TRANSFER_DST,
                .name = APPNAME_PREFIX("swapchain (clearcolor)"),
            });

            App() : AppWindow<App>(APPNAME " (clearcolor)") {}

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
                auto swapchain_image = swapchain.acquire_next_image();
                if (swapchain_image.is_empty())
                {
                    return;
                }
                auto cmd_list = device.create_command_list({
                    .name = APPNAME_PREFIX("cmd_list (clearcolor)"),
                });

                cmd_list.pipeline_barrier_image_transition({
                    .dst_access = daxa::AccessConsts::TRANSFER_WRITE,
                    .src_layout = daxa::ImageLayout::UNDEFINED,
                    .dst_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
                    .image_id = swapchain_image,
                });

                cmd_list.clear_image({
                    .dst_image_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
                    .clear_value = {std::array<f32, 4>{1, 0, 1, 1}},
                    .dst_image = swapchain_image,
                });

                cmd_list.pipeline_barrier_image_transition({
                    .src_access = daxa::AccessConsts::TRANSFER_WRITE,
                    .src_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
                    .dst_layout = daxa::ImageLayout::PRESENT_SRC,
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

            void on_mouse_move(f32 /*unused*/, f32 /*unused*/) {}
            void on_mouse_button(i32 /*unused*/, i32 /*unused*/) {}
            void on_key(i32 /*unused*/, i32 /*unused*/) {}

            void on_resize(u32 sx, u32 sy)
            {
                minimized = sx == 0 || sy == 0;
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
            if (app.update())
            {
                break;
            }
        }
    }
} // namespace tests

auto main() -> int
{
    tests::simple_creation();
    tests::clearcolor();
}
