#pragma once
#if DAXA_BUILT_WITH_UTILS_TASK_GRAPH && DAXA_BUILT_WITH_UTILS_IMGUI
#include <daxa/utils/task_graph_types.hpp>

#if ENABLE_TASK_GRAPH_MK2
#include "task_graph_debug_ui.slang"

namespace daxa
{
    union Vec4Union
    {
        daxa_f32vec4 _float = {0, 0, 0, 0};
        daxa_i32vec4 _int;
        daxa_u32vec4 _uint;
    };

    struct ResourceViewerState
    {
        TaskGraphDebugUiReadbackStruct latest_readback = {};
        ImageId display_image = {};
        ImageId clone_image = {};
        ImageId destroy_display_image = {}; // mini deletion queue
        ImageId destroy_clone_image = {};   // mini deletion queue

        BufferId readback_buffer = {};
        u32 open_frame_count = {};

        void * imgui_image_id = {};
        bool free_window = {};
        bool free_window_next_frame = {};

        // Settings:
        i32 mip = {};
        i32 layer = {};
        bool freeze_resource = {};
        bool pre_task = {};
        bool clear_before_task = {};

        // Written by ui:
        bool display_value_range_initialized = {};
        union { f32 _f32; i32 _i32; u32 _u32; } min_display_value = { ._u32 = {} };
        union { f32 _f32; i32 _i32; u32 _u32; } max_display_value = { ._u32 = {} };
        i32 rainbow_ints = false;
        i32 nearest_filtering = true;
        daxa_i32vec4 enabled_channels = {true, true, true, false};
        daxa_i32vec2 mouse_texel_index = {};
        f32 inspector_image_draw_scale = -1.0f;
        i32 timeline_index = {};
        bool fixed_display_mip_sizes = true;
        bool active = false;
        bool display_image_hovered = false;
        bool freeze_image_hover_index = false;
        bool gamma_correct = false;
        bool display_as_hexadecimal = false;
    };

    struct TaskGraphDebugContext;

    void task_resource_viewer_debug_ui_hook(TaskGraphDebugContext & context, u32 task_index, daxa::TaskInterface & ti, bool is_pre_task);

    auto resource_viewer_ui(TaskGraphDebugContext & context, std::string const & resource_name, ResourceViewerState & state) -> bool;
}

#endif
#endif