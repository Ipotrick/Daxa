#pragma once

#include "DaxaCore.hpp"
#include "gpu/Device.hpp"

#include <deque>

#include <imgui.h>
#include <imgui_impl_glfw.h>

namespace daxa {

    class ImGuiRenderer{
    public:
        ImGuiRenderer(DeviceHandle device, CommandQueueHandle queue, PipelineCompilerHandle& compiler);
        ~ImGuiRenderer();

        void recordCommands(ImDrawData* draw_data, daxa::CommandListHandle& cmdList, ImageViewHandle& target);
    private:
        daxa::DeviceHandle device               = {};
        daxa::PipelineHandle pipeline           = {};
        daxa::ImageViewHandle fontSheetId       = {};
        daxa::SamplerHandle samplerId             = {};

        struct PerFrameData{
            daxa::BufferHandle vertexBuffer     = {};
            daxa::BufferHandle indexBuffer      = {};
        };
        std::deque<PerFrameData> perFrameData   = {};

        void recreatePerFrameData(size_t newMinSizeVertex, size_t newMinSizeIndices);

        std::unordered_map<void*, size_t> texHandlePtrToReferencedImageIndex;
    };
}