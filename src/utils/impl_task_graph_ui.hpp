#pragma once
#if DAXA_BUILT_WITH_UTILS_TASK_GRAPH && DAXA_BUILT_WITH_UTILS_IMGUI
#include <daxa/utils/task_graph_types.hpp>
#include <daxa/utils/imgui.hpp>

#if DAXA_ENABLE_TASK_GRAPH_MK2

#include "impl_resource_viewer.hpp"
#include <set>
#include <optional>
#include <filesystem>


#define TASK_GRAPH_RESOURCE_VIEWER_ONLINE_COMPILE_SHADERS 0 // FOR DEVELOPMENT ONLY

#if TASK_GRAPH_RESOURCE_VIEWER_ONLINE_COMPILE_SHADERS
#include <daxa/utils/pipeline_manager.hpp>
#endif

namespace daxa
{
    struct ImplTaskGraph;
    struct ImGuiRenderer;

    namespace ColorPalette
    {
        static constexpr ImVec4 GREY = ImVec4(0.35638f, 0.37626f, 0.40198f, 1.0f);

        static constexpr ImVec4 RED = ImVec4(0.90590f, 0.29800f, 0.23530f, 1.0f);
        static constexpr ImVec4 GREEN = ImVec4(0.18040f, 0.80000f, 0.44310f, 1.0f);
        static constexpr ImVec4 BLUE = ImVec4(0.20390f, 0.59610f, 0.85880f, 1.0f);
        static constexpr ImVec4 YELLOW = ImVec4(0.94510f, 0.76860f, 0.05880f, 1.0f);
        static constexpr ImVec4 ORANGE = ImVec4(0.90200f, 0.49410f, 0.13330f, 1.0f);
        static constexpr ImVec4 PURPLE = ImVec4(0.61180f, 0.34900f, 0.71370f, 1.0f);
        static constexpr ImVec4 CYAN = ImVec4(0.10200f, 0.73730f, 0.61180f, 1.0f);

        static constexpr ImVec4 DARK_RED = ImVec4(0.53600f, 0.03700f, 0.02000f, 1.0f);
        static constexpr ImVec4 DARK_BLUE = ImVec4(0.05490f, 0.32941f, 0.96470f, 1.0f);
    };

    struct ImplTaskGraphDebugUi final : ImplHandle
    {
        ImplTaskGraphDebugUi() = default;
        ~ImplTaskGraphDebugUi();

        Device device = {};
        ImGuiRenderer imgui_renderer = {};
        u32 last_exec_tg_unique_index = ~0u;
        std::array<char, 256> resource_name_search = {};
        std::array<char, 256> task_name_search = {};
        std::unordered_map<std::string, u32> pinned_resources = {};
        std::set<std::string> extra_highlighted_tasks = {};
        std::vector<std::vector<u32>> task_resource_to_attachment_lookup = {};
        std::set<std::string> open_task_detail_windows = {};
        std::set<std::string> open_resource_detail_windows = {};
        std::optional<std::filesystem::path> buffer_layout_cache_folder = {};
        bool show_batch_borders = true;
        bool show_transfer_tasks = true;
        bool show_async_queues = true;
        bool recreated = true;
        u32 hovered_task = ~0u;
        u32 hovered_resource = ~0u;
        u32 hovered_attachment_index = ~0u;

#if TASK_GRAPH_RESOURCE_VIEWER_ONLINE_COMPILE_SHADERS
        daxa::PipelineManager pipeline_manager = {};
#endif
        std::unordered_map<std::string, ResourceViewerState> resource_viewer_states = {};
        std::shared_ptr<daxa::ComputePipeline> resource_viewer_pipeline = {};
        SamplerId resource_viewer_sampler_id = {};
        
        static void zero_ref_callback(ImplHandle const * handle);
    };
}

#endif
#endif