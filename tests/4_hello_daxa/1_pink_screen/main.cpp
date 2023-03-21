#include <daxa/daxa.hpp>

#include <GLFW/glfw3.h>
#if defined(_WIN32)
#define GLFW_EXPOSE_NATIVE_WIN32
#elif defined(__linux__)
#define GLFW_EXPOSE_NATIVE_X11
#endif
#include <GLFW/glfw3native.h>

daxa::NativeWindowHandle get_native_handle(GLFWwindow * glfw_window_ptr)
{
#if defined(_WIN32)
    return glfwGetWin32Window(glfw_window_ptr);
#elif defined(__linux__)
    return reinterpret_cast<daxa::NativeWindowHandle>(glfwGetX11Window(glfw_window_ptr));
#endif
}

daxa::NativeWindowPlatform get_native_platform(GLFWwindow *)
{
#if defined(_WIN32)
    return daxa::NativeWindowPlatform::WIN32_API;
#elif defined(__linux__)
    return daxa::NativeWindowPlatform::XLIB_API;
#endif
}

struct WindowInfo
{
    daxa::u32 width, height;
    bool swapchain_out_of_date = false;
};

int main()
{
    auto window_info = WindowInfo{.width = 800, .height = 600};
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    auto glfw_window_ptr = glfwCreateWindow(
        static_cast<daxa::i32>(window_info.width),
        static_cast<daxa::i32>(window_info.height),
        "Daxa sample window name", nullptr, nullptr);
    glfwSetWindowUserPointer(glfw_window_ptr, &window_info);
    glfwSetWindowSizeCallback(
        glfw_window_ptr,
        [](GLFWwindow * glfw_window_ptr, int width, int height)
        {
            auto & window_info = *reinterpret_cast<WindowInfo *>(glfwGetWindowUserPointer(glfw_window_ptr));
            window_info.swapchain_out_of_date = true;
            window_info.width = static_cast<daxa::u32>(width);
            window_info.height = static_cast<daxa::u32>(height);
        });
    auto native_window_handle = get_native_handle(glfw_window_ptr);

    daxa::Context context = daxa::create_context({.enable_validation = false});

    daxa::Device device = context.create_device({
        .selector = [](daxa::DeviceProperties const & device_props) -> daxa::i32
        {
            daxa::i32 score = 0;
            switch (device_props.device_type)
            {
            case daxa::DeviceType::DISCRETE_GPU: score += 10000; break;
            case daxa::DeviceType::VIRTUAL_GPU: score += 1000; break;
            case daxa::DeviceType::INTEGRATED_GPU: score += 100; break;
            default: break;
            }
            score += static_cast<daxa::i32>(device_props.limits.max_memory_allocation_count / 100000);
            return score;
        },
        .debug_name = "my device",
    });

    daxa::Swapchain swapchain = device.create_swapchain({
        .native_window = native_window_handle,
        .native_window_platform = get_native_platform(glfw_window_ptr),
        .surface_format_selector = [](daxa::Format format)
        {
            switch (format)
            {
            case daxa::Format::R8G8B8A8_UINT: return 100;
            default: return daxa::default_format_score(format);
            }
        },
        .present_mode = daxa::PresentMode::FIFO,
        .image_usage = daxa::ImageUsageFlagBits::TRANSFER_DST,
        .debug_name = "my swapchain",
    });

    while (true)
    {
        glfwPollEvents();
        if (glfwWindowShouldClose(glfw_window_ptr))
        {
            break;
        }

        if (window_info.swapchain_out_of_date)
        {
            swapchain.resize();
            window_info.swapchain_out_of_date = false;
        }

        daxa::ImageId swapchain_image = swapchain.acquire_next_image();
        if (swapchain_image.is_empty())
        {
            continue;
        }

        daxa::ImageMipArraySlice swapchain_image_full_slice = device.info_image_view(swapchain_image.default_view()).slice;

        daxa::CommandList command_list = device.create_command_list({.debug_name = "my command list"});

        command_list.pipeline_barrier_image_transition({
            .waiting_pipeline_access = daxa::AccessConsts::TRANSFER_WRITE,
            .before_layout = daxa::ImageLayout::UNDEFINED,
            .after_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
            .image_slice = swapchain_image_full_slice,
            .image_id = swapchain_image,
        });

        command_list.clear_image({
            .dst_image_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
            .clear_value = std::array<daxa::f32, 4>{1.0f, 0.0f, 1.0f, 1.0f},
            .dst_image = swapchain_image,
            .dst_slice = swapchain_image_full_slice,
        });

        command_list.pipeline_barrier_image_transition({
            .awaited_pipeline_access = daxa::AccessConsts::TRANSFER_WRITE,
            .before_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
            .after_layout = daxa::ImageLayout::PRESENT_SRC,
            .image_slice = swapchain_image_full_slice,
            .image_id = swapchain_image,
        });

        command_list.complete();

        auto const & acquire_semaphore = swapchain.get_acquire_semaphore();
        auto const & present_semaphore = swapchain.get_present_semaphore();
        auto const & gpu_timeline = swapchain.get_gpu_timeline_semaphore();
        auto const cpu_timeline = swapchain.get_cpu_timeline_value();

        device.submit_commands({
            .command_lists = {command_list},
            .wait_binary_semaphores = {acquire_semaphore},
            .signal_binary_semaphores = {present_semaphore},
            .signal_timeline_semaphores = {{gpu_timeline, cpu_timeline}},
        });
        device.present_frame({
            .wait_binary_semaphores = {present_semaphore},
            .swapchain = swapchain,
        });
    }

    device.wait_idle();
    device.collect_garbage();
}
