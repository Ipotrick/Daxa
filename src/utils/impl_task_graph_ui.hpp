#pragma once
#if DAXA_BUILT_WITH_UTILS_TASK_GRAPH && DAXA_BUILT_WITH_UTILS_IMGUI
#include <daxa/utils/task_graph_types.hpp>

#if ENABLE_TASK_GRAPH_MK2

#include "impl_resource_viewer.hpp"

#include <set>

#define TASK_GRAPH_resource_viewer_ONLINE_COMPILE_SHADERS 1

#if TASK_GRAPH_resource_viewer_ONLINE_COMPILE_SHADERS
#include <daxa/utils/pipeline_manager.hpp>
#include "task_graph_debug_ui.slang"
#endif

namespace daxa
{
    struct ImplTaskGraph;
    struct ImGuiRenderer;

    struct TaskGraphDebugContext
    {
        u32 last_exec_tg_unique_index = ~0u;
        ImplTaskGraph * impl = {};
        std::array<char, 256> resource_name_search = {};
        std::array<char, 256> task_name_search = {};
        std::unordered_map<std::string, u32> pinned_resources = {};
        std::set<std::string> extra_highlighted_tasks = {};
        std::vector<std::vector<u32>> task_resource_to_attachment_lookup = {};
        std::set<std::string> open_task_detail_windows = {};
        std::set<std::string> open_resource_detail_windows = {};
        bool show_batch_borders = true;
        bool show_transfer_tasks = true;
        bool show_async_queues = true;
        bool recreated = true;
        u32 hovered_task = ~0u;
        u32 hovered_resource = ~0u;
        u32 hovered_attachment_index = ~0u;

#if TASK_GRAPH_resource_viewer_ONLINE_COMPILE_SHADERS
        daxa::PipelineManager pipeline_manager = {};
#endif
        std::unordered_map<std::string, ResourceViewerState> resource_viewer_states = {};
        std::shared_ptr<daxa::ComputePipeline> resource_viewer_pipeline = {};
        SamplerId resource_viewer_sampler_id = {};
        // Needed to create image ids for imgui.
        ImGuiRenderer * imgui_renderer = {};
    };

    auto get_debug_ui_context(ImplTaskGraph & impl) -> TaskGraphDebugContext*;
}

#endif
#endif