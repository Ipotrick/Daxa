#pragma once
#if DAXA_BUILT_WITH_UTILS_TASK_GRAPH && DAXA_BUILT_WITH_UTILS_IMGUI
#include <daxa/utils/task_graph_types.hpp>

#if DAXA_ENABLE_TASK_GRAPH_MK2
#include "impl_resource_viewer.slang"

// 1mb
#define BUFFER_RESOURCE_VIEWER_READBACK_SIZE (1u << 22u)

namespace daxa
{
    union Vec4Union
    {
        daxa_f32vec4 _float = {0, 0, 0, 0};
        daxa_i32vec4 _int;
        daxa_u32vec4 _uint;
    };

    struct TgDebugStructFieldDefinition
    {
        std::string name = {};
        u64 in_struct_memory_offset = {};
        i32 type_index = {};            // negative indexes into primitives;
        i32 array_size = 1;
        i32 pointer_depth = 0;          // number of pointer levels (e.g., int** = 2)
    };

    struct TgDebugTypeDefinition
    {
        std::string name = {};
        u64 size = ~0ull;
        u64 alignment = ~0ull;
        std::vector<TgDebugStructFieldDefinition> fields = {};
    };

    enum TG_DEBUG_PRIMITIVE_TYPE_INDEX : i32
    {
        TG_DEBUG_PRIMITIVE_TYPE_INDEX_F16 = -1,
        TG_DEBUG_PRIMITIVE_TYPE_INDEX_F32 = -2,
        TG_DEBUG_PRIMITIVE_TYPE_INDEX_F64 = -3,
        TG_DEBUG_PRIMITIVE_TYPE_INDEX_I8 = -4,
        TG_DEBUG_PRIMITIVE_TYPE_INDEX_I16 = -5,
        TG_DEBUG_PRIMITIVE_TYPE_INDEX_I32 = -6,
        TG_DEBUG_PRIMITIVE_TYPE_INDEX_I64 = -7,
        TG_DEBUG_PRIMITIVE_TYPE_INDEX_U8 = -8,
        TG_DEBUG_PRIMITIVE_TYPE_INDEX_U16 = -9,
        TG_DEBUG_PRIMITIVE_TYPE_INDEX_U32 = -10,
        TG_DEBUG_PRIMITIVE_TYPE_INDEX_U64 = -11,
    };

    static inline std::array TG_DEBUG_PRIMITIVE_DEFINITIONS = std::array{
        TgDebugTypeDefinition{ "f16", 2, 2 },
        TgDebugTypeDefinition{ "f32", sizeof(f32), alignof(f32) },
        TgDebugTypeDefinition{ "f64", sizeof(f64), alignof(f64) },
        TgDebugTypeDefinition{ "i8" , 1, 1 },
        TgDebugTypeDefinition{ "i16", 2, 2 },
        TgDebugTypeDefinition{ "i32", sizeof(i32), alignof(i32) },
        TgDebugTypeDefinition{ "i64", sizeof(i64), alignof(i64) },
        TgDebugTypeDefinition{ "u8" , 1, 1 },
        TgDebugTypeDefinition{ "u16", 2, 2 },
        TgDebugTypeDefinition{ "u32", sizeof(u32), alignof(u32) },
        TgDebugTypeDefinition{ "u64", sizeof(u64), alignof(u64) },
    };

    struct ImGuiUiStorage
    {
        bool array_collapsed = {};
        bool array_hidden = {};
        f32 constrained_height = 500.0f;
    };

    struct ChildBufferResourceViewerState
    {
        TgDebugStructFieldDefinition field_def = {};

        BufferId clone_buffer = {};
        BufferId destroy_clone_buffer = {};

        BufferId buffer = {};
        u64 base_offset = {};
        i32 display_array_size = 1;
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
            BufferId clone_buffer = {};
            BufferId destroy_clone_buffer = {};
            std::string format_c_struct_code = {};
            std::string format_c_struct_code_compile_error_message = {};
            std::vector<std::string> custom_ui_id_stack = { "root" };
            std::unordered_map<u64, ImGuiUiStorage> ui_storage = {};
            std::vector<TgDebugTypeDefinition> tg_debug_struct_definitions = {};
            bool buffer_format_config_open = {};
            std::unordered_map<DeviceAddress, ChildBufferResourceViewerState> child_buffer_inspectors = {};
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