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
                .debug_name = APPNAME_PREFIX("device (simple_creation)"),
            });

            daxa::Swapchain swapchain = device.create_swapchain({
                .native_window = get_native_handle(),
                .width = size_x,
                .height = size_y,
                .present_mode = daxa::PresentMode::DOUBLE_BUFFER_WAIT_FOR_VBLANK,
                .image_usage = daxa::ImageUsageFlagBits::TRANSFER_DST,
                .debug_name = APPNAME_PREFIX("swapchain (simple_creation)"),
            });

            App() : AppWindow<App>(APPNAME " (simple_creation)") {}

            void on_mouse_move(f32, f32) {}
            void on_key(int, int) {}
            void on_resize(u32, u32) {}
        };
        App app;
    }

    void clearcolor()
    {
        struct App : AppWindow<App>
        {
            daxa::Context daxa_ctx = daxa::create_context({
                .enable_validation = true,
            });
            daxa::Device device = daxa_ctx.create_device({
                .debug_name = APPNAME_PREFIX("device (clearcolor)"),
            });

            daxa::Swapchain swapchain = device.create_swapchain({
                .native_window = get_native_handle(),
                .width = size_x,
                .height = size_y,
                .image_usage = daxa::ImageUsageFlagBits::TRANSFER_DST,
                .debug_name = APPNAME_PREFIX("swpachain (clearcolor)"),
            });

            App() : AppWindow<App>(APPNAME " (clearcolor)") {}

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
                auto swapchain_image = swapchain.acquire_next_image();
                auto binary_semaphore = device.create_binary_semaphore({
                    .debug_name = APPNAME_PREFIX("binary_semaphore (clearcolor)"),
                });
                auto cmd_list = device.create_command_list({
                    .debug_name = APPNAME_PREFIX("cmd_list (clearcolor)"),
                });

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

                cmd_list.pipeline_barrier_image_transition({
                    .awaited_pipeline_access = daxa::AccessConsts::TRANSFER_WRITE,
                    .before_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
                    .after_layout = daxa::ImageLayout::PRESENT_SRC,
                    .image_id = swapchain_image,
                });

                cmd_list.complete();

                device.submit_commands({
                    .command_lists = {std::move(cmd_list)},
                    .signal_binary_semaphores = {binary_semaphore},
                });

                device.present_frame({
                    .wait_binary_semaphores = {binary_semaphore},
                    .swapchain = swapchain,
                });
            }

            void on_mouse_move(f32, f32)
            {
            }

            void on_key(int, int)
            {
            }

            void on_resize(u32 sx, u32 sy)
            {
                if (sx != size_x || sy != size_y)
                {
                    size_x = sx;
                    size_y = sy;
                    minimized = size_x == 0 || size_y == 0;
                    if (!minimized)
                    {
                        swapchain.resize(size_x, size_y);
                        draw();
                    }
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
    tests::simple_creation();
    tests::clearcolor();
}
