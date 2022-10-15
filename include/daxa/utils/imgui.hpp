#pragma once

#if !DAXA_BUILT_WITH_UTILS
#error "[package management error] You must build Daxa with the UTILS option enabled"
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
        PipelineCompiler pipeline_compiler;
        Format format;
    };

    struct ImGuiRenderer : ManagedPtr
    {
        ImGuiRenderer() = default;

        ImGuiRenderer(ImGuiRendererInfo const & info);
        ~ImGuiRenderer();

        void record_commands(ImDrawData * draw_data, CommandList & cmd_list, ImageId target_image, u32 size_x, u32 size_y);
        void record_task(ImDrawData * draw_data, TaskList & task_list, TaskImageId task_swapchain_image, u32 size_x, u32 size_y);
    };
} // namespace daxa
