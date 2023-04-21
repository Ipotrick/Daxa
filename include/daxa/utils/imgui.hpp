#pragma once

#if !DAXA_BUILT_WITH_UTILS_IMGUI
#error "[package management error] You must build Daxa with the DAXA_ENABLE_UTILS_IMGUI CMake option enabled, or request the utils-imgui feature in vcpkg"
#endif

#include <daxa/core.hpp>
#include <daxa/device.hpp>

#include <daxa/utils/task_list.hpp>

#include <imgui.h>

namespace daxa
{
    struct ImGuiRendererInfo
    {
        Device device;
        Format format;
        // NOTE: This is for backwards compatability. Though,
        // I'm not sure the ImGui renderer util should set the
        // ImGui style. Something to bikeshed.
        bool use_custom_config = true;
    };

    struct ImGuiRenderer : ManagedPtr
    {
        ImGuiRenderer() = default;

        ImGuiRenderer(ImGuiRendererInfo const & info);
        ~ImGuiRenderer();

        void record_commands(ImDrawData * draw_data, CommandList & cmd_list, ImageId target_image, u32 size_x, u32 size_y);
        void record_task(ImDrawData * draw_data, TaskList & task_list, TaskImageHandle task_swapchain_image, u32 size_x, u32 size_y);
    };
} // namespace daxa
