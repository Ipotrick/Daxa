#pragma once

#if !DAXA_BUILT_WITH_UTILS_IMGUI
#error "[build error] You must build Daxa with the DAXA_ENABLE_UTILS_IMGUI CMake option enabled"
#endif

#include <daxa/core.hpp>
#include <daxa/device.hpp>

#include <imgui.h>

struct ImPlotContext;

namespace daxa
{
    struct ImGuiImageContext
    {
        ImageViewId image_view;
        SamplerId sampler;
    };

    struct ImGuiRendererInfo
    {
        Device device;
        Format format;
        ImGuiContext * imgui_context = {};
        ImPlotContext * implot_context = {};
        // NOTE: This is for backwards compatibility. Though,
        // I'm not sure the ImGui renderer util should set the
        // ImGui style. Something to bikeshed.
        bool use_custom_config = true;
    };

    struct ImGuiRecordCommandsInfo
    {
        ImDrawData* draw_data;
        CommandRecorder & recorder;
        ImageId target_image = {};
        u32 size_x = {};
        u32 size_y = {};
    };

    struct ImplImGuiRenderer;
    struct DAXA_EXPORT_CXX ImGuiRenderer : ManagedPtr<ImGuiRenderer, ImplImGuiRenderer *>
    {
        ImGuiRenderer() = default;

        ImGuiRenderer(ImGuiRendererInfo const & info);

        auto create_texture_id(ImGuiImageContext const & context) -> ImTextureID;

        void record_commands(ImGuiRecordCommandsInfo const & info);
      protected:
        template <typename T, typename H_T>
        friend struct ManagedPtr;
        static auto inc_refcnt(ImplHandle const * object) -> u64;
        static auto dec_refcnt(ImplHandle const * object) -> u64;
    };
} // namespace daxa
