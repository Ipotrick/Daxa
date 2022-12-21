#pragma once

#include <0_common/window.hpp>

#include <thread>
using namespace std::chrono_literals;
#include <iostream>
#include <cmath>

#include <daxa/utils/pipeline_manager.hpp>

#include <daxa/utils/imgui.hpp>
#include <imgui_impl_glfw.h>

#include <daxa/utils/math_operators.hpp>
using namespace daxa::math_operators;

using Clock = std::chrono::high_resolution_clock;

#if !defined(APPNAME)
#define APPNAME "Daxa App"
#endif
#define APPNAME_PREFIX(x) ("[" APPNAME "] " x)

#if !defined(DAXA_SHADER_INCLUDE_DIR)
#define DAXA_SHADER_INCLUDE_DIR "include"
#endif

#if !defined(DAXA_SHADERLANG)
#define DAXA_SHADERLANG DAXA_SHADERLANG_GLSL
#endif

template <typename T>
struct BaseApp : AppWindow<T>
{
    daxa::Context daxa_ctx = daxa::create_context({
        .enable_validation = false,
    });
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
        .debug_name = APPNAME_PREFIX("device"),
    });

    daxa::Swapchain swapchain = device.create_swapchain({
        .native_window = AppWindow<T>::get_native_handle(),
        .native_window_platform = AppWindow<T>::get_native_platform(),
        .present_mode = daxa::PresentMode::DO_NOT_WAIT_FOR_VBLANK,
        .image_usage = daxa::ImageUsageFlagBits::TRANSFER_DST,
        .debug_name = APPNAME_PREFIX("swapchain"),
    });

    daxa::PipelineManager pipeline_manager = daxa::PipelineManager({
        .device = device,
        .shader_compile_options = {
            .root_paths = {
                DAXA_SAMPLE_PATH "/shaders",
                "tests/0_common/shaders",
                DAXA_SHADER_INCLUDE_DIR,
            },
#if DAXA_SHADERLANG == DAXA_SHADERLANG_GLSL
            .language = daxa::ShaderLanguage::GLSL,
#elif DAXA_SHADERLANG == DAXA_SHADERLANG_HLSL
            .language = daxa::ShaderLanguage::HLSL,
#endif
            .enable_debug_info = true,
        },
        .debug_name = APPNAME_PREFIX("pipeline_manager"),
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

    daxa::CommandSubmitInfo submit_info;

    daxa::ImageId swapchain_image;
    daxa::TaskImageId task_swapchain_image;

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

    auto record_loop_task_list() -> daxa::TaskList
    {
        daxa::TaskList new_task_list = daxa::TaskList({
            .device = device,
            .use_split_barriers = false,
            .swapchain = swapchain,
            .debug_name = APPNAME_PREFIX("task_list"),
        });
        task_swapchain_image = new_task_list.create_task_image({
            .swapchain_image = true,
            .debug_name = APPNAME_PREFIX("task_swapchain_image"),
        });
        new_task_list.add_runtime_image(task_swapchain_image, swapchain_image);

        reinterpret_cast<T *>(this)->record_tasks(new_task_list);

        new_task_list.add_task({
            .used_images = {
                {task_swapchain_image, daxa::TaskImageAccess::COLOR_ATTACHMENT, daxa::ImageMipArraySlice{}},
            },
            .task = [this](daxa::TaskRuntime interf)
            {
                auto cmd_list = interf.get_command_list();
                imgui_renderer.record_commands(ImGui::GetDrawData(), cmd_list, swapchain_image, AppWindow<T>::size_x, AppWindow<T>::size_y);
            },
            .debug_name = APPNAME_PREFIX("ImGui Task"),
        });

        new_task_list.submit(&submit_info);
        new_task_list.present({});
        new_task_list.complete();

        // new_task_list.output_graphviz();

        return new_task_list;
    }
};
