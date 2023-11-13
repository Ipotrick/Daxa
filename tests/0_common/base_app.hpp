#pragma once

#include <0_common/window.hpp>

#include <thread>
using namespace std::chrono_literals;
#include <iostream>
#include <cmath>

#include <daxa/utils/pipeline_manager.hpp>

#include <daxa/utils/imgui.hpp>
#include <imgui_impl_glfw.h>

using Clock = std::chrono::high_resolution_clock;

#if !defined(APPNAME)
#define APPNAME "Daxa App"
#endif
#define APPNAME_PREFIX(x) ("[" APPNAME "] " x)

#if !defined(DAXA_SHADERLANG)
#define DAXA_SHADERLANG DAXA_SHADERLANG_GLSL
#endif


template <typename T>
struct BaseApp : AppWindow<T>
{
    daxa::Instance daxa_ctx = daxa::create_instance({});
    daxa::Device device = daxa_ctx.create_device({
        .selector = [](daxa::DeviceProperties const & device_props) -> i32
        {
            i32 score = 0;
            switch (device_props.device_type)
            {
            case daxa::DeviceType::DISCRETE_GPU: score += 10000; break;
            case daxa::DeviceType::VIRTUAL_GPU: score += 1000; break;
            case daxa::DeviceType::INTEGRATED_GPU: score += 100; break;
            default: break;
            }
            return score;
        },
        .name = "device",
    });

    daxa::Swapchain swapchain = device.create_swapchain({
        .native_window = AppWindow<T>::get_native_handle(),
        .native_window_platform = AppWindow<T>::get_native_platform(),
        .present_mode = daxa::PresentMode::IMMEDIATE,
        .image_usage = daxa::ImageUsageFlagBits::TRANSFER_DST,
        .name = "swapchain",
    });

    daxa::PipelineManager pipeline_manager = daxa::PipelineManager({
        .device = device,
        .shader_compile_options = {
            .root_paths = {
                DAXA_SHADER_INCLUDE_DIR,
                DAXA_SAMPLE_PATH "/shaders",
                "tests/0_common/shaders",
            },
#if DAXA_SHADERLANG == DAXA_SHADERLANG_GLSL
            .language = daxa::ShaderLanguage::GLSL,
#elif DAXA_SHADERLANG == DAXA_SHADERLANG_HLSL
            .language = daxa::ShaderLanguage::HLSL,
#endif
            .enable_debug_info = true,
        },
        .name = "pipeline_manager",
    });

    daxa::ImGuiRenderer imgui_renderer = create_imgui_renderer();
    auto create_imgui_renderer() -> daxa::ImGuiRenderer
    {
        ImGui::CreateContext();
        ImGui_ImplGlfw_InitForVulkan(AppWindow<T>::glfw_window_ptr, true);
        return daxa::ImGuiRenderer({
            .device = device,
            .format = swapchain.get_format(),
        });
    }

    Clock::time_point start = Clock::now(), prev_time = start;
    f32 time = 0.0f, delta_time = 1.0f;

    daxa::TaskImage task_swapchain_image{{.swapchain_image = true, .name = "swapchain_image"}};
    std::vector<daxa::GenericTaskResourceUse> imgui_task_uses{};

    BaseApp() : AppWindow<T>(APPNAME)
    {
    }

    ~BaseApp()
    {
        ImGui_ImplGlfw_Shutdown();
    }

    void base_on_update()
    {
        auto now = Clock::now();
        time = std::chrono::duration<f32>(now - start).count();
        delta_time = std::chrono::duration<f32>(now - prev_time).count();
        prev_time = now;
        reinterpret_cast<T *>(this)->on_update();
    }

    auto update() -> bool
    {
        glfwPollEvents();
        if (glfwWindowShouldClose(AppWindow<T>::glfw_window_ptr))
        {
            return true;
        }

        if (!AppWindow<T>::minimized)
        {
            base_on_update();
        }
        else
        {
            std::this_thread::sleep_for(1ms);
        }

        return false;
    }

    auto record_loop_task_graph() -> daxa::TaskGraph
    {
        using namespace daxa::task_resource_uses;
        daxa::TaskGraph new_task_graph = daxa::TaskGraph({
            .device = device,
            .swapchain = swapchain,
            .use_split_barriers = false,
            .name = "main_task_graph",
        });
        new_task_graph.use_persistent_image(task_swapchain_image);

        using namespace daxa::task_resource_uses;
        imgui_task_uses.push_back(ImageColorAttachment<>{task_swapchain_image});
        reinterpret_cast<T *>(this)->record_tasks(new_task_graph);

        new_task_graph.add_task({
            .uses = imgui_task_uses,
            .task = [this](daxa::TaskInterface ti)
            {
                auto& recorder = ti.get_recorder();
                imgui_renderer.record_commands(ImGui::GetDrawData(), recorder, ti.uses[task_swapchain_image].image(), AppWindow<T>::size_x, AppWindow<T>::size_y);
            },
            .name = "ImGui Task",
        });

        new_task_graph.submit({});
        new_task_graph.present({});
        new_task_graph.complete({});

        // new_task_graph.output_graphviz();

        return new_task_graph;
    }
};
