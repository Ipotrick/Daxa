#pragma once
#if DAXA_BUILT_WITH_UTILS_TASK_GRAPH && DAXA_BUILT_WITH_UTILS_IMGUI
#include <daxa/utils/task_graph_types.hpp>

#if DAXA_ENABLE_TASK_GRAPH_MK2
#include "impl_resource_viewer.slang"

#define BUFFER_RESOURCE_VIEWER_READBACK_SIZE (1u << 12u)

namespace daxa
{
    union Vec4Union
    {
        daxa_f32vec4 _float = {0, 0, 0, 0};
        daxa_i32vec4 _int;
        daxa_u32vec4 _uint;
    };

    // Blame imgui for this being i32
    enum TG_DEBUG_BUFFER_ENTRY_DATA_TYPE : i32
    {
        TG_DEBUG_BUFFER_ENTRY_DATA_TYPE_F32 = 0,
        TG_DEBUG_BUFFER_ENTRY_DATA_TYPE_I32 = 1,
        TG_DEBUG_BUFFER_ENTRY_DATA_TYPE_U32 = 2,
        TG_DEBUG_BUFFER_ENTRY_DATA_TYPE_STRUCT = 4,
    };

    struct TgDebugBufferEntry
    {
        std::string name = {};
        TG_DEBUG_BUFFER_ENTRY_DATA_TYPE data_type = {};
        i32 array_size = 1;
        u64 offset = {};
        std::vector<u32> child_indices;
        u32 parent_index;
    };

    struct ResourceViewerState
    {
        struct 
        {
            TaskGraphDebugUiImageReadbackStruct latest_readback = {};
            ImageId display_image = {};
            ImageId clone_image = {};
            ImageId destroy_display_image = {}; // mini deletion queue
            ImageId destroy_clone_image = {};   // mini deletion queue
            void * imgui_image_id = {};
            union { f32 _f32; i32 _i32; u32 _u32; } min_display_value = { ._u32 = {} };
            union { f32 _f32; i32 _i32; u32 _u32; } max_display_value = { ._u32 = {} };
            i32 rainbow_ints = false;
            i32 mip = {};
            i32 layer = {};
            daxa_i32vec4 enabled_channels = {true, true, true, false};
            daxa_i32vec2 mouse_texel_index = {};
            bool gamma_correct = false;
            bool display_value_range_initialized = {};
            BufferId readback_buffer = {};
        } image = {};
        struct
        {
            std::array<std::byte, BUFFER_RESOURCE_VIEWER_READBACK_SIZE> latest_readback = {};
            std::vector<TgDebugBufferEntry> tg_debug_buffer_structure = {};       
            u64 readback_offset = {};
            BufferId readback_buffer = {};   
            bool buffer_format_config_open = {};
        } buffer = {};

        u32 open_frame_count = {};
        i32 timeline_index = {};
        bool display_as_hexadecimal = false;
        bool clear_before_task = {};
        bool freeze_resource = {};
        bool pre_task = {};
        bool free_window = {};
        bool free_window_next_frame = {};
    };

    struct ImplTaskGraphDebugUi;
    struct ImplTaskGraph;

    void task_resource_viewer_debug_ui_hook(ImplTaskGraphDebugUi & context, ImplTaskGraph * impl, u32 task_index, daxa::TaskInterface & ti, bool is_pre_task);

    auto resource_viewer_ui(ImplTaskGraphDebugUi & context, ImplTaskGraph * impl, std::string const & resource_name, ResourceViewerState & state) -> bool;
}

#endif
#endif