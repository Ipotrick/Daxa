#include <daxa/daxa.hpp>
#include <daxa/utils/pipeline_manager.hpp>
#include <daxa/utils/task_graph.hpp>

#include <iostream>
#include <GLFW/glfw3.h>
#if defined(_WIN32)
#define GLFW_EXPOSE_NATIVE_WIN32
#elif defined(__linux__)
#define GLFW_EXPOSE_NATIVE_X11
#endif
#include <GLFW/glfw3native.h>

auto get_native_window_info(GLFWwindow * glfw_window_ptr) -> daxa::NativeWindowInfo
{
#if defined(_WIN32)
    return daxa::NativeWindowInfoWin32{ glfwGetWin32Window(glfw_window_ptr) };
#elif defined(__linux__)
    switch (glfwGetPlatform())
    {
    case GLFW_PLATFORM_WAYLAND:
        return daxa::NativeWindowInfoWayland {
            .display = glfwGetWaylandDisplay(),
            .surface = glfwGetWaylandWindow(glfw_window_ptr),
            .width   = size_x,
            .height  = size_y,
        };
    default:
        return daxa::NativeWindowInfoXlib {
            .window = reinterpret_cast<void*>(glfwGetX11Window(glfw_window_ptr))
        };
    }
#endif
}

struct WindowInfo
{
    daxa::u32 width{}, height{};
    bool swapchain_out_of_date = false;
};

#include "shared.inl"

daxa::BufferId indirect_dispatch_buffer = {};

auto main() -> int
{
    auto window_info = WindowInfo{.width = 800, .height = 600};
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    auto * glfw_window_ptr = glfwCreateWindow(
        static_cast<daxa::i32>(window_info.width),
        static_cast<daxa::i32>(window_info.height),
        "Daxa mesh shader sample", nullptr, nullptr);
    glfwSetWindowUserPointer(glfw_window_ptr, &window_info);
    glfwSetWindowSizeCallback(
        glfw_window_ptr,
        [](GLFWwindow * glfw_window, int width, int height)
        {
            auto & window_info_ref = *reinterpret_cast<WindowInfo *>(glfwGetWindowUserPointer(glfw_window));
            window_info_ref.swapchain_out_of_date = true;
            window_info_ref.width = static_cast<daxa::u32>(width);
            window_info_ref.height = static_cast<daxa::u32>(height);
        });
    auto native_window_info = get_native_window_info(glfw_window_ptr);

    daxa::Instance instance = daxa::create_instance({});

    daxa::Device device = instance.create_device_2(instance.choose_device(daxa::ImplicitFeatureFlagBits::MESH_SHADER, {}));

    indirect_dispatch_buffer = device.create_buffer({
        .size = sizeof(daxa_u32vec3),
        .memory_flags = daxa::MemoryFlagBits::HOST_ACCESS_SEQUENTIAL_WRITE,
    });
    *device.buffer_host_address_as<daxa_u32vec3>(indirect_dispatch_buffer).value() = {
        1,
        1,
        1,
    };

    daxa::Swapchain swapchain = device.create_swapchain({
        .native_window_info = native_window_info,
        .present_mode = daxa::PresentMode::FIFO,
        .surface_format = device.choose_swapchain_surface_format({
            .native_window_info = native_window_info,
            .preferred_formats = std::array{daxa::SurfaceFormat{ .format = daxa::Format::B8G8R8A8_UNORM }},
        }),
        .image_usage = daxa::ImageUsageFlagBits::TRANSFER_DST,
        .name = "my swapchain",
    });

    daxa::ImageId render_target = device.create_image({
        .format = daxa::Format::B8G8R8A8_UNORM,
        .size = {window_info.width, window_info.height, 1},
        .sample_count = 4,
        .usage = daxa::ImageUsageFlagBits::COLOR_ATTACHMENT | daxa::ImageUsageFlagBits::TRANSFER_SRC,
        .name = "render target",
    });
    daxa::ExternalTaskImage trender_image = daxa::ExternalTaskImage{{
        .image = render_target,
        .name = "render target",
    }};

    auto pipeline_manager = daxa::PipelineManager({
        .device = device,
        .root_paths = {
            DAXA_SHADER_INCLUDE_DIR,
            "./tests/2_daxa_api/11_mesh_shader",
        },
        .default_language = daxa::ShaderLanguage::SLANG,
        .default_enable_debug_info = true,
        .name = "my pipeline manager",
    });
    // Then just adding it to the pipeline manager
    std::shared_ptr<daxa::RasterPipeline> pipeline;
    {
        auto result = pipeline_manager.add_raster_pipeline2({
            .mesh_shader_info = daxa::ShaderCompileInfo2{
                .source = daxa::ShaderFile{"draw.slang"},
                .entry_point = "entry_mesh",
            },
            .fragment_shader_info = daxa::ShaderCompileInfo2{
                .source = daxa::ShaderFile{"draw.slang"},
                .entry_point = "entry_fragment",
            },
            .task_shader_info = daxa::ShaderCompileInfo2{
                .source = daxa::ShaderFile{"draw.slang"},
                .entry_point = "entry_task",
            },
            .color_attachments = {{.format = swapchain.get_format()}},
            .raster = {.static_state_sample_count = daxa::None},
            .name = "my pipeline",
        });
        if (result.is_err())
        {
            std::cerr << result.message() << std::endl;
            return -1;
        }
        pipeline = result.value();
    }

    auto task_swapchain_image = daxa::ExternalTaskImage{{.is_swapchain_image = true, .name = "swapchain image"}};

    auto loop_task_graph = daxa::TaskGraph({
        .device = device,
        .swapchain = swapchain,
        .name = "loop",
    });

    auto swapchain_tview = loop_task_graph.register_image(task_swapchain_image);
    loop_task_graph.register_image(trender_image);

    // And a task to draw to the screen
    loop_task_graph.add_task(daxa::HeadTask<DrawTri::Info>()
        .head_views({
            .resolve_target = swapchain_tview,
            .render_target = trender_image.view(),
        })
        .executes([=, pipeline_ptr = &pipeline](daxa::TaskInterface ti){
            using namespace DrawTri;
            daxa::ImageInfo color_img_info = ti.info(AT.render_target).value();
            auto const size_x = color_img_info.size.x;
            auto const size_y = color_img_info.size.y;
            auto render_recorder = std::move(ti.recorder).begin_renderpass({
                .color_attachments = std::array{
                    daxa::RenderAttachmentInfo{
                        .image_view = ti.get(AT.render_target).view_ids[0],
                        .load_op = daxa::AttachmentLoadOp::CLEAR,
                        .clear_value = std::array<daxa::f32, 4>{0.1f, 0.0f, 0.5f, 1.0f},
                        .resolve = daxa::AttachmentResolveInfo{
                            .image = ti.get(AT.resolve_target).view_ids[0],
                        },
                    },
                },
                .render_area = {.x = 0, .y = 0, .width = size_x, .height = size_y},
            });
            render_recorder.set_rasterization_samples(daxa::RasterizationSamples::E4);
            render_recorder.set_pipeline(**pipeline_ptr);

            render_recorder.draw_mesh_tasks_indirect({
                .indirect_buffer = indirect_dispatch_buffer, 
                .draw_count = 1u,
                .stride = 12u,
            });
            ti.recorder = std::move(render_recorder).end_renderpass();
        }));

    loop_task_graph.submit({});
    loop_task_graph.present({});
    loop_task_graph.complete({});

    while (true)
    {
        glfwPollEvents();
        if (glfwWindowShouldClose(glfw_window_ptr) != 0)
        {
            break;
        }

        if (window_info.swapchain_out_of_date)
        {
            swapchain.resize();
            window_info.swapchain_out_of_date = false;
        }

        // acquire the next image as usual,
        auto swapchain_image = swapchain.acquire_next_image();
        if (swapchain_image.is_empty())
        {
            continue;
        }
        task_swapchain_image.set_image(swapchain_image);
        loop_task_graph.execute({});
        device.collect_garbage();
    }

    device.wait_idle();
    device.destroy_image(render_target);
    device.collect_garbage();
}