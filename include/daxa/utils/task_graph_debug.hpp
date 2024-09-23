#pragma once

#include <daxa/daxa.hpp>
#include <daxa/utils/pipeline_manager.hpp>
#include <daxa/utils/task_graph.hpp>
#include <daxa/utils/imgui.hpp>

#include <unordered_map>
#include <set>

union Vec4Union
{
    daxa_f32vec4 _float = {0, 0, 0, 0};
    daxa_i32vec4 _int;
    daxa_u32vec4 _uint;
};

struct TgDebugImageInspectorState
{
    // Written by ui:
    daxa_f64 min_v = 0.0;
    daxa_f64 max_v = 1.0;
    daxa_u32 mip = 0u;
    daxa_u32 layer = 0u;
    daxa_i32 rainbow_ints = false;
    daxa_i32 nearest_filtering = true;
    daxa_i32vec4 enabled_channels = {true, true, true, true};
    daxa_i32vec2 mouse_pos_relative_to_display_image = {0, 0};
    daxa_i32vec2 mouse_pos_relative_to_image_mip0 = {0, 0};
    daxa_i32vec2 display_image_size = {0, 0};

    daxa_i32vec2 frozen_mouse_pos_relative_to_image_mip0 = {0, 0};
    Vec4Union frozen_readback_raw = {};
    daxa_f32vec4 frozen_readback_color = {0, 0, 0, 0};
    daxa_i32 resolution_draw_mode = 0;
    bool fixed_display_mip_sizes = true;
    bool freeze_image = false;
    bool active = false;
    bool display_image_hovered = false;
    bool freeze_image_hover_index = false;
    bool pre_task = false;
    // Written by tg:
    bool slice_valid = true;
    daxa::TaskImageAttachmentInfo attachment_info = {};
    daxa::BufferId readback_buffer = {};
    daxa::ImageInfo runtime_image_info = {};
    daxa::ImageId display_image = {};
    daxa::ImageId frozen_image = {};
    daxa::ImageId stale_image = {};
    daxa::ImageId stale_image1 = {};
};

struct TgDebugContext
{
    daxa_f32vec2 override_mouse_picker_uv = {};
    bool request_mouse_picker_override = {};
    bool override_mouse_picker = {};
    bool override_frozen_state = {};
    std::array<char, 256> search_substr = {};
    std::string task_image_name = "color_image";
    uint32_t readback_index = 0;

    struct TgDebugTask
    {
        std::string task_name = {};
        std::vector<daxa::TaskAttachmentInfo> attachments = {};
    };
    std::unordered_map<std::string, size_t> this_frame_duplicate_task_name_counter = {};
    std::vector<TgDebugTask> this_frame_task_attachments = {}; // cleared every frame.
    std::unordered_map<std::string, TgDebugImageInspectorState> inspector_states = {};
    std::set<std::string> active_inspectors = {};

    std::shared_ptr<daxa::ComputePipeline> debug_pipeline;
    daxa::Device device;

    void init(daxa::Device & device, daxa::PipelineManager & pipeline_manager);
    void cleanup();
    void debug_task(daxa::TaskInterface ti, bool pre_task);

    void debug_ui(daxa::ImGuiRenderer & imgui_renderer);
    auto debug_ui_image_inspector(daxa::ImGuiRenderer & imgui_renderer, std::string active_inspector_key) -> bool;
};
