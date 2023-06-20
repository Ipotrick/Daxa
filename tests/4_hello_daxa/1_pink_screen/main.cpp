#include <daxa/daxa.hpp>

#include <GLFW/glfw3.h>
#if defined(_WIN32)
#define GLFW_EXPOSE_NATIVE_WIN32
#elif defined(__linux__)
#define GLFW_EXPOSE_NATIVE_X11
#elif defined(__APPLE__)
#define GLFW_EXPOSE_NATIVE_COCOA
#endif
#include <GLFW/glfw3native.h>

auto get_native_handle(GLFWwindow * glfw_window_ptr) -> daxa::NativeWindowHandle
{
#if defined(_WIN32)
    return glfwGetWin32Window(glfw_window_ptr);
#elif defined(__linux__)
    return reinterpret_cast<daxa::NativeWindowHandle>(glfwGetX11Window(glfw_window_ptr));
#elif defined(__APPLE__)
    return glfwGetCocoaWindow(glfw_window_ptr);
#else
    return {};
#endif
}

auto get_native_platform(GLFWwindow * /*unused*/) -> daxa::NativeWindowPlatform
{
#if defined(_WIN32)
    return daxa::NativeWindowPlatform::WIN32_API;
#elif defined(__linux__)
    return daxa::NativeWindowPlatform::XLIB_API;
#elif defined(__APPLE__)
    return daxa::NativeWindowPlatform::COCOA_API;
#else
    return daxa::NativeWindowPlatform::UNKNOWN;
#endif
}

struct WindowInfo
{
    daxa::u32 width{}, height{};
    bool swapchain_out_of_date = false;
};

auto main() -> int
{
    auto window_info = WindowInfo{.width = 800, .height = 600};
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    auto *glfw_window_ptr = glfwCreateWindow(
        static_cast<daxa::i32>(window_info.width),
        static_cast<daxa::i32>(window_info.height),
        "Daxa sample window name", nullptr, nullptr);
    glfwSetWindowUserPointer(glfw_window_ptr, &window_info);
    glfwSetWindowSizeCallback(
        glfw_window_ptr,
        [](GLFWwindow * glfw_window_ptr, int width, int height)
        {
            auto & window_info = *reinterpret_cast<WindowInfo *>(glfwGetWindowUserPointer(glfw_window_ptr));
            // We set this bool because it lets us know we need to resize the swapchain later!
            window_info.swapchain_out_of_date = true;
            window_info.width = static_cast<daxa::u32>(width);
            window_info.height = static_cast<daxa::u32>(height);
        });
    auto *native_window_handle = get_native_handle(glfw_window_ptr);
    auto native_window_platform = get_native_platform(glfw_window_ptr);

    // First thing we do is create a Daxa context. This essentially exists
    // to initialize the Vulkan context, and allows for the creation of multiple
    // devices.
    daxa::Context context = daxa::create_context({});

    // We then just create a Daxa device by using the context, and providing
    // the necessary parameters. This is where you'd select different features
    // and extensions that aren't necessarily present on all GPU's drivers.
    // A user's computer may have multiple devices, so you can provide a selector
    // function which just returns to daxa the rating level of each device listed,
    // ultimately returning a handle to the device which was scored highest!
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
        .name = "my device",
    });

    // To be able to render to a window, Daxa requires the user to create a swapchain
    // for a system window. To create a Swapchain, Daxa needs a native window handle
    // and native platform. Optionally, the user can provide a debug name, surface
    // format type selector, the additional image uses (image uses will be explained later),
    // and present mode (this controls sync)
    daxa::Swapchain swapchain = device.create_swapchain({
        // this handle is given by the windowing API
        .native_window = native_window_handle,
        // The platform would also be retrieved from the windowing API,
        // or by hard-coding it depending on the OS.
        .native_window_platform = native_window_platform,
        .surface_format_selector = [](daxa::Format format)
        {
            switch (format)
            {
            case daxa::Format::R8G8B8A8_UINT: return 100;
            default: return daxa::default_format_score(format);
            }
        },
        .present_mode = daxa::PresentMode::MAILBOX,
        .image_usage = daxa::ImageUsageFlagBits::TRANSFER_DST,
        .name = "my swapchain",
    });

    while (true)
    {
        glfwPollEvents();
        if (glfwWindowShouldClose(glfw_window_ptr) != 0)
        {
            break;
        }

        // Here's where we handle that variable we set in the resize callback of GLFW,
        // in order to properly handle resizing of a window!
        if (window_info.swapchain_out_of_date)
        {
            swapchain.resize();
            window_info.swapchain_out_of_date = false;
        }

        // The first thing that is done is retrieving the current swapchain image.
        // This is the image we are going to clear. If swapchain image acquisition
        // failed, then the result of this acquire function will be an "empty image",
        // and thus we want to skip this frame. We do this by saying "continue".
        daxa::ImageId const swapchain_image = swapchain.acquire_next_image();
        if (swapchain_image.is_empty())
        {
            continue;
        }

        // Directly after, we define an image slice. As images can be made up of multiple
        // layers, memory planes, and mip levels, Daxa takes slices in many calls to
        // specify a slice of an image that should be operated upon.
        daxa::ImageMipArraySlice const swapchain_image_full_slice = device.info_image_view(swapchain_image.default_view()).slice;

        daxa::CommandList command_list = device.create_command_list({.name = "my command list"});

        command_list.pipeline_barrier_image_transition({
            .dst_access = daxa::AccessConsts::TRANSFER_WRITE,
            .src_layout = daxa::ImageLayout::UNDEFINED,
            .dst_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
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
            .src_access = daxa::AccessConsts::TRANSFER_WRITE,
            .src_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
            .dst_layout = daxa::ImageLayout::PRESENT_SRC,
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
