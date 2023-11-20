#pragma once

#if !DAXA_BUILT_WITH_UTILS_IMGUI
#error "[package management error] You must build Daxa with the DAXA_ENABLE_UTILS_IMGUI CMake option enabled, or request the utils-imgui feature in vcpkg"
#endif

#include <daxa/core.hpp>
#include <daxa/device.hpp>

#if DAXA_BUILT_WITH_UTILS_TASK_GRAPH
#include <daxa/utils/task_graph.hpp>
#endif

#include <imgui.h>

namespace daxa
{
    struct ImGuiImageContext
    {
        ImageViewId image_view_id;
        SamplerId sampler_id;
    };

    struct ImGuiRendererInfo
    {
        Device device;
        Format format;
        ImGuiContext * context = {};
        // NOTE: This is for backwards compatibility. Though,
        // I'm not sure the ImGui renderer util should set the
        // ImGui style. Something to bikeshed.
        bool use_custom_config = true;
    };

    struct ImplImGuiRenderer;
    struct DAXA_EXPORT_CXX ImGuiRenderer : ManagedPtr<ImGuiRenderer, ImplImGuiRenderer *>
    {
        ImGuiRenderer() = default;

        ImGuiRenderer(ImGuiRendererInfo const & info);

        auto create_texture_id(ImGuiImageContext const & context) -> ImTextureID;

        void record_commands(ImDrawData * draw_data, CommandRecorder & recorder, ImageId target_image, u32 size_x, u32 size_y);
#if DAXA_BUILT_WITH_UTILS_TASK_GRAPH
        void record_task(ImDrawData * draw_data, TaskGraph & task_graph, TaskImageView task_swapchain_image, u32 size_x, u32 size_y);
#endif
      protected:
        template <typename T, typename H_T>
        friend struct ManagedPtr;
        static auto inc_refcnt(ImplHandle const * object) -> u64;
        static auto dec_refcnt(ImplHandle const * object) -> u64;
    };
} // namespace daxa
