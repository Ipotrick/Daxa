#if DAXA_BUILT_WITH_UTILS_TASK_GRAPH
#include <daxa/utils/task_graph_types.hpp>

#if DAXA_BUILT_WITH_UTILS_IMGUI && ENABLE_TASK_GRAPH_MK2

#include "impl_task_graph_mk2.hpp"
#include <daxa/utils/imgui.hpp>
#include <imgui_internal.h>
#include <set>

static inline ImVec2 operator+(ImVec2 const & lhs, ImVec2 const & rhs) { return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y); }
static inline ImVec2 operator-(ImVec2 const & lhs, ImVec2 const & rhs) { return ImVec2(lhs.x - rhs.x, lhs.y - rhs.y); }


namespace ImGui
{
    // Unlike TableHeadersRow() it is not expected that you can reimplement or customize this with custom widgets.
    // FIXME: No hit-testing/button on the angled header.
    void TableAngledHeadersRow2(ImU32 * colors, ImU32 * text_color)
    {
        ImGuiContext& g = *GImGui;
        ImGuiTable* table = g.CurrentTable;
        ImGuiTableTempData* temp_data = table->TempData;
        temp_data->AngledHeadersRequests.resize(0);
        temp_data->AngledHeadersRequests.reserve(table->ColumnsEnabledCount);

        // Which column needs highlight?
        const ImGuiID row_id = GetID("##AngledHeaders");
        ImGuiTableInstanceData* table_instance = TableGetInstanceData(table, table->InstanceCurrent);
        int highlight_column_n = table->HighlightColumnHeader;
        if (highlight_column_n == -1 && table->HoveredColumnBody != -1)
            if (table_instance->HoveredRowLast == 0 && table->HoveredColumnBorder == -1 && (g.ActiveId == 0 || g.ActiveId == row_id || (table->IsActiveIdInTable || g.DragDropActive)))
                highlight_column_n = table->HoveredColumnBody;

        // Build up request
        //ImU32 col_header_bg = GetColorU32(ImGuiCol_TableHeaderBg);
        // ImU32 col_text = GetColorU32(ImGuiCol_Text);
        for (int order_n = 0; order_n < table->ColumnsCount; order_n++)
            if (IM_BITARRAY_TESTBIT(table->EnabledMaskByDisplayOrder, order_n))
            {
                const int column_n = table->DisplayOrderToIndex[order_n];
                ImGuiTableColumn* column = &table->Columns[column_n];
                if ((column->Flags & ImGuiTableColumnFlags_AngledHeader) == 0) // Note: can't rely on ImGuiTableColumnFlags_IsVisible test here.
                    continue;
                ImGuiTableHeaderData request = { (ImGuiTableColumnIdx)column_n, text_color[order_n], colors[order_n], 0};
                temp_data->AngledHeadersRequests.push_back(request);
            }

        // Render row
        TableAngledHeadersRowEx(row_id, g.Style.TableAngledHeadersAngle, 0.0f, temp_data->AngledHeadersRequests.Data, temp_data->AngledHeadersRequests.Size);
    }
}

namespace daxa
{
    auto to_lower(std::string str) -> std::string
    {
        for(auto & c : str) { c = std::tolower(c); }
        return str;
    } 

    struct TaskGraphDebugContext
    {
        u32 last_exec_tg_unique_index = ~0u;
        ImplTaskGraph* impl = {};
        std::array<char, 256> resource_name_search = {};
        std::array<char, 256> task_name_search = {};
        std::unordered_map<std::string, u32> pinned_resources = {};
        std::vector<std::vector<u32>> task_resource_to_attachment_lookup = {};
        std::set<std::string> open_task_detail_windows = {};
        std::set<std::string> open_resource_detail_windows = {};
        std::unordered_map<std::string_view, u32> name_to_task_index = {};
        bool show_batch_barriers = true;
        u32 hovered_task = ~0u;
        u32 hovered_resource = ~0u;
        u32 hovered_attachment_index = ~0u;
    };

    void task_dependencies_ui(ImplTaskGraph & impl)
    {
        static float world_canvas_scale = 1.0f;
        static ImVec2 world_canvas_offset(500.0f, 500.0f);

        static bool adding_line = false;

        ImVec2 screen_canvas_top_left_corner = ImGui::GetCursorScreenPos(); // ImDrawList API uses screen coordinates!
        ImVec2 screen_canvas_size = ImGui::GetContentRegionAvail();         // Resize canvas to what's available
        if (screen_canvas_size.x < 50.0f)
            screen_canvas_size.x = 50.0f;
        if (screen_canvas_size.y < 50.0f)
            screen_canvas_size.y = 50.0f;

        ImVec2 screen_canvas_bottom_right_corner = ImVec2(screen_canvas_top_left_corner.x + screen_canvas_size.x, screen_canvas_top_left_corner.y + screen_canvas_size.y);

        auto world_to_canvas = [&](ImVec2 position)
        {
            return ImVec2{
                (position.x - world_canvas_offset.x) * world_canvas_scale,
                (position.y - world_canvas_offset.y) * world_canvas_scale,
            };
        };

        auto canvas_to_world = [&](ImVec2 position)
        {
            return ImVec2{
                position.x / world_canvas_scale + world_canvas_offset.x,
                position.y / world_canvas_scale + world_canvas_offset.y,
            };
        };

        auto canvas_to_screen = [&](ImVec2 position)
        {
            return ImVec2{
                position.x + screen_canvas_top_left_corner.x,
                position.y + screen_canvas_top_left_corner.y,
            };
        };

        auto screen_to_canvas = [&](ImVec2 position)
        {
            return ImVec2{
                position.x - screen_canvas_top_left_corner.x,
                position.y - screen_canvas_top_left_corner.y,
            };
        };

        auto world_to_screen = [&](ImVec2 position)
        {
            return canvas_to_screen(world_to_canvas(position));
        };

        auto screen_to_world = [&](ImVec2 position)
        {
            return canvas_to_world(screen_to_canvas(position));
        };

        // Draw border and background color
        ImGuiIO & io = ImGui::GetIO();
        ImDrawList * draw_list = ImGui::GetWindowDrawList();
        draw_list->AddRectFilled(screen_canvas_top_left_corner, screen_canvas_bottom_right_corner, IM_COL32(50, 50, 50, 255));
        draw_list->AddRect(screen_canvas_top_left_corner, screen_canvas_bottom_right_corner, IM_COL32(255, 255, 255, 255));

        // This will catch our interactions
        ImGui::InvisibleButton("canvas", screen_canvas_size, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight | ImGuiButtonFlags_MouseButtonMiddle);
        bool const is_active = ImGui::IsItemActive();
        bool const is_hovered = ImGui::IsItemHovered();

        if (is_hovered)
        {
            ImVec2 const world_mouse_pos = screen_to_world(io.MousePos);
            ImVec2 const world_canvas_top_left_corner = screen_to_world(screen_canvas_top_left_corner);

            float const previous_world_canvas_scale = world_canvas_scale;
            world_canvas_scale = world_canvas_scale + world_canvas_scale * 0.05f * io.MouseWheel;

            float const scale_change_factor = previous_world_canvas_scale / world_canvas_scale;

            world_canvas_offset = ImVec2(
                world_canvas_top_left_corner.x * scale_change_factor + world_mouse_pos.x * (1.0f - scale_change_factor),
                world_canvas_top_left_corner.y * scale_change_factor + world_mouse_pos.y * (1.0f - scale_change_factor));
        }

        // Pan (we use a zero mouse threshold when there's no context menu)
        // You may decide to make that threshold dynamic based on whether the mouse is hovering something etc.
        float const mouse_threshold_for_pan = 0.0f;
        if (is_active && ImGui::IsMouseDragging(ImGuiMouseButton_Right, mouse_threshold_for_pan))
        {
            world_canvas_offset.x -= io.MouseDelta.x / world_canvas_scale;
            world_canvas_offset.y -= io.MouseDelta.y / world_canvas_scale;
        }

        // Context menu (under default mouse threshold)
        ImVec2 drag_delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);

        // Draw grid + all lines in the canvas
        draw_list->PushClipRect(screen_canvas_top_left_corner, screen_canvas_bottom_right_corner, true);
        {
            draw_list->ChannelsSplit(2);
            /// ==============================
            /// ==== DRAW BACKGROUND GRID ====
            /// ==============================
            {
                draw_list->ChannelsSetCurrent(0); // Foreground
                float const GRID_STEP = 64.0f;
                ImVec2 canvas_world_p0 = screen_to_world(screen_canvas_top_left_corner);
                ImVec2 canvas_world_p1 = screen_to_world(screen_canvas_bottom_right_corner);
                float first_world_x = static_cast<f32>(align_up(static_cast<i32>(canvas_world_p0.x - GRID_STEP), static_cast<i32>(GRID_STEP)));
                float first_world_y = static_cast<f32>(align_up(static_cast<i32>(canvas_world_p0.y - GRID_STEP), static_cast<i32>(GRID_STEP)));
                u32 vertical_lines = static_cast<u32>(std::ceil((canvas_world_p1.x - canvas_world_p0.x) / GRID_STEP) + 1);
                u32 horizontal_lines = static_cast<u32>(std::ceil((canvas_world_p1.y - canvas_world_p0.y) / GRID_STEP) + 1);

                for (u32 i = 0; i < vertical_lines; ++i)
                {
                    ImVec2 start = {first_world_x + GRID_STEP * i, canvas_world_p0.y};
                    ImVec2 end = {first_world_x + GRID_STEP * i, canvas_world_p1.y};
                    draw_list->AddLine(world_to_screen(start), world_to_screen(end), IM_COL32(200, 200, 200, 40));
                }
                for (u32 i = 0; i < horizontal_lines; ++i)
                {
                    ImVec2 start = {canvas_world_p0.x, first_world_y + GRID_STEP * i};
                    ImVec2 end = {canvas_world_p1.x, first_world_y + GRID_STEP * i};
                    draw_list->AddLine(world_to_screen(start), world_to_screen(end), IM_COL32(200, 200, 200, 40));
                }
            }

            /// ====================
            /// ==== DRAW TASKS ====
            /// ====================
            {
                float const WS_BATCH_HORIZONTAL_DISTANCE = 256.0f;
                float const WS_TASK_VERTICAL_DISTANCE = 256.0f;
                ImVec2 const NODE_WINDOW_PADDING(8.0f, 8.0f);

                float acc_submit_hor_offset = 0.0f;
                for (u32 submit_index = 0; submit_index < impl.submits.size(); ++submit_index)
                {
                    TasksSubmit const & submit = impl.submits[submit_index];

                    float acc_queues_vert_offset = 0.0f;
                    for (u32 q = 0; q < submit.queue_indices.size(); ++q)
                    {
                        u32 queue_index = submit.queue_indices[q];
                        std::span<TasksBatch> queue_batches = submit.queue_batches[queue_index];

                        u64 max_tasks_in_batches = 0;
                        for (u32 queue_batch_i = 0; queue_batch_i < queue_batches.size(); ++queue_batch_i)
                        {
                            TasksBatch const & batch = queue_batches[queue_batch_i];

                            max_tasks_in_batches = std::max(max_tasks_in_batches, batch.tasks.size());
                        }

                        // Draw Queue Submit Background
                        {
                            ImVec2 start = {
                                acc_submit_hor_offset,
                                acc_queues_vert_offset,
                            };
                            ImVec2 end = {
                                acc_submit_hor_offset + WS_BATCH_HORIZONTAL_DISTANCE * queue_batches.size(),
                                acc_queues_vert_offset + WS_TASK_VERTICAL_DISTANCE * max_tasks_in_batches,
                            };
                            draw_list->ChannelsSetCurrent(0); // Background
                            draw_list->AddRectFilled(world_to_screen(start), world_to_screen(end), IM_COL32(80, 80, 100, 127), 8.0f);
                        }

                        for (u32 queue_batch_i = 0; queue_batch_i < queue_batches.size(); ++queue_batch_i)
                        {
                            TasksBatch const & batch = queue_batches[queue_batch_i];

                            for (u32 queue_batch_task_i = 0; queue_batch_task_i < batch.tasks.size(); ++queue_batch_task_i)
                            {
                                auto [task, task_i] = batch.tasks[queue_batch_task_i];

                                ImVec2 task_cell_center_position = {
                                    acc_submit_hor_offset + (queue_batch_i + 0.5f) * WS_BATCH_HORIZONTAL_DISTANCE,
                                    acc_queues_vert_offset + (queue_batch_task_i + 0.5f) * WS_TASK_VERTICAL_DISTANCE,
                                };

                                ImGui::PushID(task);
                                draw_list->ChannelsSetCurrent(1); // Foreground
                                float const DETAIL_VIEW_MIN_SCALE = 0.6f;
                                if (world_canvas_scale > DETAIL_VIEW_MIN_SCALE)
                                {
                                    ImVec2 node_rect_min = world_to_screen(task_cell_center_position) - ImVec2(
                                                                                                            WS_BATCH_HORIZONTAL_DISTANCE * 0.5f * DETAIL_VIEW_MIN_SCALE,
                                                                                                            WS_TASK_VERTICAL_DISTANCE * 0.5f * DETAIL_VIEW_MIN_SCALE);

                                    // Display node contents first
                                    ImGui::SetCursorScreenPos(node_rect_min + NODE_WINDOW_PADDING);
                                    ImGui::BeginGroup(); // Lock horizontal position
                                    draw_list->AddRectFilled(
                                        node_rect_min,
                                        node_rect_min + ImVec2(WS_BATCH_HORIZONTAL_DISTANCE * DETAIL_VIEW_MIN_SCALE, WS_TASK_VERTICAL_DISTANCE * DETAIL_VIEW_MIN_SCALE),
                                        IM_COL32(100, 100, 100, 255),
                                        8.0f);

                                    ImGui::Text("%s", std::string(task->name).c_str());
                                    ImGui::Text("Attachment Count %i", task->attachments.size());
                                    ImGui::EndGroup();
                                }
                                else
                                {
                                    draw_list->AddRectFilled(
                                        world_to_screen(task_cell_center_position - ImVec2{20 * 0.5f, 20 * 0.5f}),
                                        world_to_screen(task_cell_center_position + ImVec2{20 * 0.5f, 20 * 0.5f}),
                                        IM_COL32(100, 100, 100, 255),
                                        8.0f);
                                }
                                ImGui::PopID();
                            }
                        }

                        acc_queues_vert_offset += (max_tasks_in_batches + 1) * WS_TASK_VERTICAL_DISTANCE;
                    }

                    acc_submit_hor_offset += (submit.batch_count + 1) * WS_BATCH_HORIZONTAL_DISTANCE;
                }

                draw_list->ChannelsMerge();
            }
        }
        draw_list->PopClipRect();
    }

    struct CellUiData
    {
        u32 local_batch_index = {};
        u32 batch_local_task_index = {};
        u32 global_task_index = {};
        ImplTask* task = {};
    };
    struct RowUiData
    {
        u32 global_resource_index = {};
        ImplTaskResource* resource = {};
    };
    struct ColUiData
    {
        u32 submit_index = {};
        u32 queue_index = {};
        u32 local_batch_index = {};
        u32 batch_local_task_index = {};
        u32 global_task_index = {};
        bool is_submit_border = {};
        bool is_queue_submit_border = {};
        ImplTask* task = {};
        u32 task_index = ~0u;
    };
    struct BatchUiData
    {
        AccessGroup const * access_group = {};
    };
    auto task_type_to_str(TaskType task_type) -> char const *
    {
        switch(task_type)
        {
            case TaskType::GENERAL: return "GN";
            case TaskType::RASTER: return "RS";
            case TaskType::COMPUTE: return "CT";
            case TaskType::RAY_TRACING: return "RT";
            case TaskType::TRANSFER: return "TF";
            default: return "UNKNOWN";
        }
    }

    auto queue_family_to_str(QueueFamily queue_family) -> char const *
    {
        switch(queue_family)
        {
            case QueueFamily::MAIN: return "GN";
            case QueueFamily::COMPUTE: return "CT";
            case QueueFamily::TRANSFER: return "TF";
            default: return "UNKNOWN";
        }
    }

    auto access_type_to_color(TaskAccessType access) -> ImVec4
    {
        // yellow = ImVec4(0.95500f, 0.58500f, 0.09300f, 1.0f); break; //                  #F9C74F
        ImVec4 ret = {};
        switch (access)
        {
            case TaskAccessType::NONE:                  ret = ImVec4(0.35638f, 0.37626f, 0.40198f, 1.0f); break; // #9aa0a6ff
            case TaskAccessType::WRITE:                 ret = ImVec4(0.90590f, 0.29800f, 0.23530f, 1.0f); break; // #E74C3C
            case TaskAccessType::READ:                  ret = ImVec4(0.18040f, 0.80000f, 0.44310f, 1.0f); break; // #2ECC71
            case TaskAccessType::SAMPLED:               ret = ImVec4(0.18040f, 0.80000f, 0.44310f, 1.0f); break; // #2ECC71
            case TaskAccessType::READ_WRITE:            ret = ImVec4(0.20390f, 0.59610f, 0.85880f, 1.0f); break; // #3498DB
            case TaskAccessType::WRITE_CONCURRENT:      ret = ImVec4(0.53600f, 0.03700f, 0.02000f, 1.0f); break; // #E63946
            case TaskAccessType::READ_WRITE_CONCURRENT: ret = ImVec4(0.05490f, 0.32941f, 0.96470f, 1.0f); break; // #0e54f6
        }
        // ret.x = std::sqrt(ret.x);
        // ret.y = std::sqrt(ret.y);
        // ret.z = std::sqrt(ret.z);
        return ret;
    }

    static constexpr ImVec4 LAYOUT_INITIALIZATION_COLOR = ImVec4(0.95500f, 0.58500f, 0.09300f, 1.0f);
    static constexpr ImVec4 BARRIER_COLOR = ImVec4(0.95500f, 0.700f, 0.2f, 1.0f);
    static constexpr ImVec4 SUBMIT_COLOR = ImVec4(0.8f, 0.8f, 0.8f, 1.0f);
    static constexpr ImVec4 QUEUE_SUBMIT_COLOR = ImVec4(0.6f, 0.6f, 0.6f, 1.0f);
    static constexpr ImVec4 RESOURCE_ALIVE_COLOR = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);

    // Formats the header names to always be exactly 36 chars wide
    static constexpr char const * TIMELINE_SUBMIT_OR_QUEUE_SUBMIT_HEADER_STR_FORMAT = "{:^36.36}";
    static constexpr char const * TIMELINE_TASK_HEADER_STR_FORMAT = "{:<36.36}";

    auto task_type_to_color(TaskType type) -> ImVec4
    {        
        ImVec4 ret = {};
        switch(type)
        {
            case TaskType::GENERAL: ret = ImVec4(0.47060f, 0.52941f, 0.61960f, 1.0f); break; //        #8D99AE 
            case TaskType::RASTER: ret = ImVec4(0.70110f, 0.21223f, 0.13287f, 1.0f); break; //         #e07a5fff
            case TaskType::COMPUTE: ret = ImVec4(0.95500f, 0.58500f, 0.09300f, 1.0f); break; //        #F9C74F
            case TaskType::RAY_TRACING: ret = ImVec4(0.70110f, 0.02113f, 0.04362f, 1.0f); break; //    #EF233C 
            case TaskType::TRANSFER: ret = ImVec4(0.03112f, 0.78413f, 0.60552f, 1.0f); break; //       #06D6A0 
        }
        // ret.x = std::sqrt(ret.x);
        // ret.y = std::sqrt(ret.y);
        // ret.z = std::sqrt(ret.z);
        return ret;
    }
    
    auto queue_family_to_color(QueueFamily type) -> ImVec4
    {        
        ImVec4 ret = {};
        switch(type)
        {
            case QueueFamily::MAIN: ret = ImVec4(0.47060f, 0.52941f, 0.61960f, 1.0f); break; //           #8D99AE 
            case QueueFamily::COMPUTE: ret = ImVec4(0.95500f, 0.58500f, 0.09300f, 1.0f); break; //        #F9C74F
            case QueueFamily::TRANSFER: ret = ImVec4(0.03112f, 0.78413f, 0.60552f, 1.0f); break; //       #06D6A0 
        }
        // ret.x = std::sqrt(ret.x);
        // ret.y = std::sqrt(ret.y);
        // ret.z = std::sqrt(ret.z);
        return ret;
    }
    
    auto task_popup_context_ui(
        TaskGraphDebugContext & context,
        u32 task_index)
    {
        ImplTask const & task = context.impl->tasks[task_index];
        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
        {
            context.open_task_detail_windows.insert(std::string(task.name));
        }
        if (ImGui::BeginPopupContextItem(task.name.data()))
        {
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, ImGui::GetStyle().FramePadding.y));

            bool const task_forbidden = (context.name_to_task_index[task.name] == ~0u);
            ImGui::BeginDisabled(task_forbidden);
            if (ImGui::Button("Open Detail Ui (Double Click)"))
            {
                context.open_task_detail_windows.insert(std::string(task.name));
            }
            if (task_forbidden)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 0.0f, 0.0f, 1.0f)));
                ImGui::SetItemTooltip("Task has duplicate name with other task! Detail ui requires unique task names!");
                ImGui::PopStyleColor();
            }
            ImGui::EndDisabled();

            if (ImGui::Button("Pin All Attachment Resources"))
            {
                for (u32 attach_i = 0; attach_i < task.attachment_resources.size(); ++attach_i)
                {
                    auto const & resource_pair = task.attachment_resources[attach_i];
                    if (resource_pair.first != nullptr)
                    {
                        context.pinned_resources[std::string(context.impl->resources[resource_pair.second].name)] = resource_pair.second;
                    }
                }
            }

            if (ImGui::Button("Filter For Resource In Timeline"))
            {
                for (u32 i = 0; i < std::min(255ull, task.name.size()); ++i)
                {
                    context.task_name_search[i] = task.name[i];
                }
                context.task_name_search[std::min(255ull, task.name.size())] = 0;
            }
            ImGui::PopStyleVar();
            ImGui::EndPopup();
        }
    }

    auto resource_popup_context_ui(
        TaskGraphDebugContext & context,
        u32 resource_index)
    {
        if (resource_index == ~0u)
        {
            return;
        }
        ImplTaskResource const & resource = context.impl->resources[resource_index];
        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
        {
            context.open_resource_detail_windows.insert(std::string(resource.name));
        }
        if (ImGui::BeginPopupContextItem(resource.name.data()))
        {
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, ImGui::GetStyle().FramePadding.y));
            if (ImGui::Button("Open Detail Ui (Double Click)"))
            {
                context.open_resource_detail_windows.insert(std::string(resource.name));
            }
            if (ImGui::Button("Filter For Resource In Timeline"))
            {
                for (u32 i = 0; i < std::min(255ull, resource.name.size()); ++i)
                {
                    context.resource_name_search[i] = resource.name[i];
                }
                context.resource_name_search[std::min(255ull, resource.name.size())] = 0;
            }
            if (ImGui::Button("Pin Resource In Timeline"))
            {
                context.pinned_resources[std::string(resource.name)] = resource_index;
            }
            ImGui::PopStyleVar();
            ImGui::EndPopup();
        }
    }

    // struct ImplTask
    // {
    //     void (*task_callback)(daxa::TaskInterface, void*) = {};
    //     u64* task_callback_memory = {};                             // holds captured variables
    //     std::span<TaskAttachmentInfo> attachments = {};
    //     u32 attachment_shader_blob_size = {};
    //     u32 attachment_shader_blob_alignment = {};
    //     TaskType task_type = {};
    //     std::string_view name = {};
    //     Queue queue = {};
    //     std::span<std::span<ImageViewId>> image_view_cache = {};
    //     // Used to verify image view cache:
    //     std::span<ArenaDynamicArray8k<ImageId>> runtime_images_last_execution = {};
    // };
    auto task_details_ui(TaskGraphDebugContext & context, ImplTask const & task) -> bool
    {
        ImGui::PushID(&task);
        bool open = true;
        ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
        u32 flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;
        if (ImGui::Begin(std::format("{} detail debug UI", task.name.data()).c_str(), &open, 0u))
        {
            ImVec4 default_color = ImGui::GetStyle().Colors[ImGuiCol_Text];
            static const std::array columns = {"Name", "Type", "Access Stage", "Access", "Resource"};
            if(ImGui::CollapsingHeader("Attachments"))
            {
                if(ImGui::BeginTable("", static_cast<int>(columns.size()), flags))
                {
                    for(auto const & column : columns)
                    {
                        ImGui::TableSetupColumn(column);
                    }
                    ImGui::TableHeadersRow();
                    for (u32 attach_i = 0u; attach_i < static_cast<u32>(task.attachments.size()); ++attach_i)
                    {
                        auto const & attachment_info = task.attachments[attach_i];

                        struct RowInfo
                        {
                            std::array<std::string, columns.size()> cell_text; 
                            u32 resource_index = {};
                            ImVec4 color;
                        };

                        RowInfo row_info;

                        switch(attachment_info.type)
                        {
                            case TaskAttachmentType::BUFFER:
                            case TaskAttachmentType::TLAS:
                            case TaskAttachmentType::BLAS:
                            {
                                auto const & buffer_attach_info = attachment_info.value.buffer;
                                row_info.cell_text = { std::string(buffer_attach_info.name), std::string("Buffer"), std::string(to_string(buffer_attach_info.task_access.stage)), std::string(to_string(buffer_attach_info.task_access.type))};
                                row_info.color = access_type_to_color(buffer_attach_info.task_access.type);
                                row_info.resource_index = buffer_attach_info.translated_view.index;
                                break;
                            }
                            case TaskAttachmentType::IMAGE:
                            {
                                auto const & image_attach_info = attachment_info.value.image;
                                row_info.cell_text = { std::string(image_attach_info.name), std::string("Image"), std::string(to_string(image_attach_info.task_access.stage)), std::string(to_string(image_attach_info.task_access.type))};
                                row_info.color = access_type_to_color(image_attach_info.task_access.type);
                                row_info.resource_index = image_attach_info.translated_view.index;
                                break;
                            }
                        }

                        for (u32 cell_idx = 0; cell_idx < static_cast<u32>(row_info.cell_text.size()); ++cell_idx)
                        {
                            ImGui::TableNextColumn();
                            if(cell_idx == 0)
                            {
                                bool idk = false;
                                ImGui::Selectable(row_info.cell_text.at(cell_idx).c_str(), &idk, ImGuiSelectableFlags_SpanAllColumns);
                                resource_popup_context_ui(context, row_info.resource_index);
                            }
                            else if(cell_idx == 3)
                            {
                                float const text_height = ImGui::CalcTextSize("X").y;
                                
                                ImDrawList * draw_list = ImGui::GetWindowDrawList();
                                draw_list->ChannelsSplit(2);

                                auto const pre_text_pos = ImGui::GetCursorPos();
                                {
                                    draw_list->ChannelsSetCurrent(1);
                                    draw_list->AddCircleFilled(ImVec2(ImGui::GetCursorScreenPos().x + 10.0f, ImGui::GetCursorScreenPos().y + text_height / 2.0f), 6.0f, ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 0.0f, 0.0f, 1.0f)));
                                    ImGui::SameLine(10, 10);
                                    ImGui::Text(row_info.cell_text.at(cell_idx).c_str());
                                    ImGui::SameLine();
                                }
                                auto const post_text_pos = ImGui::GetCursorPos();
                                {
                                    draw_list->ChannelsSetCurrent(0);
                                    ImGui::SetCursorPos(pre_text_pos);
                                    draw_list->AddRectFilled(ImGui::GetCursorScreenPos(), ImVec2((ImGui::GetCursorScreenPos().x + post_text_pos.x - pre_text_pos.x), ImGui::GetCursorScreenPos().y + text_height), ImGui::ColorConvertFloat4ToU32(row_info.color), 5.0f);
                                    draw_list->ChannelsMerge();
                                }
                                //ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::ColorConvertFloat4ToU32(row_info.color));
                            }
                            else if (cell_idx == 4)
                            {
                                if (row_info.resource_index == ~0u)
                                {
                                    ImGui::Text("NullResource");
                                }
                                else
                                {
                                    ImGui::Text(context.impl->resources[row_info.resource_index].name.data());
                                }
                            }
                            else
                            {
                                ImGui::Text(row_info.cell_text.at(cell_idx).c_str());
                            }
                        }
                        ImGui::TableNextRow();
                    }
                    ImGui::EndTable();
                }
            }
        }
        ImGui::End();
        ImGui::PopID();
        return open;
    }
    
    auto resource_detail_ui(
        TaskGraphDebugContext & context,
        u32 resource_index) -> bool
    {
        ImplTaskResource const & resource = context.impl->resources[resource_index];
        bool open = true;
        ImGui::SetNextWindowSize(ImVec2(500, 500), ImGuiCond_FirstUseEver);
        if (ImGui::Begin(std::format("Resource Detail Ui \"{}\"", resource.name.data()).c_str(), &open, {}))
        {
            ImGui::Separator();

            /// ==============================================
            /// ======= RESOURCE TIMELLINE INTERACTION =======
            /// ==============================================

            bool const search_contains_resource_name = 
                strstr(to_lower(std::string(context.resource_name_search.data())).c_str(), to_lower(std::string(resource.name)).c_str()) != nullptr;
            bool gui_contains_name = search_contains_resource_name;
            if (ImGui::Checkbox("Filter", &gui_contains_name))
            {
                if (search_contains_resource_name)
                {
                    context.resource_name_search = {};
                }
                else
                {
                    for (u32 i = 0; i < std::min(255ull, resource.name.size()); ++i)
                    {
                        context.resource_name_search[i] = resource.name[i];
                    }
                    context.resource_name_search[std::min(255ull, resource.name.size())] = 0;
                }
            }
            ImGui::SameLine();
            bool const pinned = context.pinned_resources.contains(std::string(resource.name));
            bool gui_pinned = pinned;
            if (ImGui::Checkbox("Pin", &gui_pinned))
            {
                if (pinned)
                {
                    context.pinned_resources.erase(std::string(resource.name));
                }
                else
                {
                    context.pinned_resources[std::string(resource.name)] = resource_index;
                }
            }

            /// ===========================================
            /// ======= GENERAL RESOURCE ATTRIBUTES =======
            /// ===========================================
            ImGui::SeparatorText("Attributes");
            static constexpr char const * ATTRIBUTE_FORMATTING = "- {:<28.28} ";
            ImGui::Text("%s %s", std::format(ATTRIBUTE_FORMATTING, "Resource Kind:").c_str(), to_string(resource.kind).data());
            if (resource.external)
            {
                ImGui::Text("%s %s", std::format(ATTRIBUTE_FORMATTING, "External Resource Name:").c_str(), resource.external->name.data());
            }
            switch(resource.kind)
            {
                case TaskResourceKind::BUFFER:
                case TaskResourceKind::BLAS:
                case TaskResourceKind::TLAS:
                {
                    u64 size = {};
                    MemoryFlags memory_flags = {};
                    u64 device_address = {};
                    u64 host_address = {};
                    SmallString external_resource_name = {};
                    if (resource.external)
                    {
                        switch(resource.kind)
                        {
                            case TaskResourceKind::BUFFER:
                            {
                                BufferId id = resource.external->id.buffer;
                                size = context.impl->info.device.buffer_info(id).value().size;
                                device_address = context.impl->info.device.buffer_device_address(id).value_or(0);
                                host_address = std::bit_cast<u64>(context.impl->info.device.buffer_host_address(id).value_or(0));
                                external_resource_name = context.impl->info.device.buffer_info(id).value().name;
                                memory_flags = context.impl->info.device.buffer_info(id).value().allocate_info;
                                break;
                            }
                            case TaskResourceKind::BLAS:
                            {
                                BlasId id = resource.external->id.blas;
                                size = context.impl->info.device.blas_info(id).value().size;
                                device_address = context.impl->info.device.blas_device_address(id).value_or(0);
                                external_resource_name = context.impl->info.device.blas_info(id).value().name;
                                memory_flags = MemoryFlagBits::NONE;
                                host_address = 0;
                                break;
                            }
                            case TaskResourceKind::TLAS:
                            {
                                TlasId id = resource.external->id.tlas;
                                size = context.impl->info.device.tlas_info(id).value().size;
                                device_address = context.impl->info.device.tlas_device_address(id).value_or(0);
                                external_resource_name = context.impl->info.device.tlas_info(id).value().name;
                                memory_flags = MemoryFlagBits::NONE;
                                host_address = 0;
                                break;
                            }
                        }
                    }
                    else
                    {
                        size = resource.info.buffer.size;
                    }

                    ImGui::Text("%s %s", std::format(ATTRIBUTE_FORMATTING, "Size:").c_str(), std::to_string(size).c_str());
                    ImGui::Text("%s %s", std::format(ATTRIBUTE_FORMATTING, "MemoryFlags:").c_str(), to_string(memory_flags).data());
                    ImGui::Text("%s %s", std::format(ATTRIBUTE_FORMATTING, "Device Address:").c_str(), std::to_string(device_address).c_str());
                    ImGui::Text("%s %s", std::format(ATTRIBUTE_FORMATTING, "Host Address:").c_str(), std::to_string(host_address).c_str());

                    break;
                }
                case TaskResourceKind::IMAGE:
                {
                    break;
                }
            }

            /// ==============================
            /// ======= USED IN STAGES =======
            /// ==============================
            {
                u64 pipeline_stage_bits = 0ull;
                for (u32 agi = 0; agi < resource.access_timeline.size(); ++agi)
                {
                    pipeline_stage_bits = pipeline_stage_bits | std::bit_cast<u64>(resource.access_timeline[agi].stages);
                }
                ImGui::Text("%s", std::format(ATTRIBUTE_FORMATTING, "Used in Pipeline Stages:").c_str());
                u64 stage_iter = pipeline_stage_bits;
                while(stage_iter)
                {
                    u64 index = 63ull - static_cast<u64>(std::countl_zero(stage_iter));
                    stage_iter &= ~(1ull << index);

                    PipelineStageFlags stage = std::bit_cast<PipelineStageFlags>(1ull << index);
                    auto str = to_string(stage);
                    if (str.size() > 0)
                    {
                        ImGui::Text("  - %s", str.c_str());
                    }
                }
            }

            /// ==============================
            /// ======= USED IN QUEUES =======
            /// ==============================
            {
                ImGui::Text("%s", std::format(ATTRIBUTE_FORMATTING, "Used in Queues:").c_str());
                u32 iter = resource.queue_bits;
                while(iter)
                {
                    u32 index = queue_bits_to_first_queue_index(iter);
                    iter &= ~(1ull << index);

                    Queue queue = queue_index_to_queue(index);
                    ImGui::Text("  - %s", to_string(queue).data());
                }
            }


            /// ===============================
            /// ======= ACCESS TIMELINE =======
            /// ===============================
            ImGui::SeparatorText("Access Timeline");
            static constexpr u32 ACCESS_GROUP_TABLE_FLAGS = ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_SizingFixedFit;
            static constexpr u32 BARRIER_TABLE_FLAGS = ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable;
            for (u32 agi = 0; agi < resource.access_timeline.size(); ++agi)
            {
                AccessGroup const & ag = resource.access_timeline[agi];
                if (agi == 0)
                {
                    if (ImGui::BeginTable("Access Group Table", 4, ACCESS_GROUP_TABLE_FLAGS))
                    {
                        ImGui::TableSetupColumn("Task          ");
                        ImGui::TableSetupColumn("Batch");
                        ImGui::TableSetupColumn("Access        ");
                        ImGui::TableSetupColumn("Attachment    ");
                        ImGui::TableHeadersRow();
                        ImGui::EndTable();
                    }
                }
                if (ag.final_schedule_pre_barrier)
                {
                    if (ImGui::BeginTable("Barrier Table", 1, BARRIER_TABLE_FLAGS))
                    {
                        ImGui::TableNextColumn();
                        // ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::ColorConvertFloat4ToU32(BARRIER_COLOR));
                        // ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0,0,0,1));
                        ImGui::Text(std::format("Barrier. Src: {} Dst: {}", to_string(ag.final_schedule_pre_barrier->src_access), to_string(ag.final_schedule_pre_barrier->dst_access)).c_str());
                        // ImGui::PopStyleColor();
                        ImGui::EndTable();
                    }
                }
                if (ImGui::BeginTable("Access Group Table", 4, ACCESS_GROUP_TABLE_FLAGS))
                {
                    for (u32 t = 0; t < ag.tasks.size(); ++t)
                    {
                        u32 task_index = ag.tasks[t].task_index;
                        ImplTask* task = ag.tasks[t].task;
                        u32 attach_i = context.task_resource_to_attachment_lookup[resource_index][task_index];
                        ImGui::TableNextColumn();
                        {
                            ImGui::PushStyleColor(ImGuiCol_Header, ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 1.0f, 1.0f, 0.2f)));
                            bool selected = context.hovered_task == task_index;
                            ImGui::Selectable(task->name.data(), &selected, ImGuiSelectableFlags_SpanAllColumns);
                            task_popup_context_ui(context, task_index);
                            ImGui::PopStyleColor();
                        }
                        ImGui::TableNextColumn();
                        ImGui::Text("%i", task->final_schedule_batch);
                        ImGui::TableNextColumn();
                        ImGui::Text(to_string(task->attachments[attach_i].value.buffer.task_access).data());
                        ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::ColorConvertFloat4ToU32(access_type_to_color(resource.access_timeline[agi].type)));
                        ImGui::TableNextColumn();
                        ImGui::Text(task->attachments[attach_i].value.buffer.name);
                    }
                    ImGui::EndTable();
                }
            }
        }
        ImGui::End();
        return open;
    }

    void timeline_legend()
    {
        ImGui::ColorButton("NONE", access_type_to_color(TaskAccessType::NONE)); ImGui::SameLine(); ImGui::Text("NONE"); 
        ImGui::ColorButton("WRITE", access_type_to_color(TaskAccessType::WRITE)); ImGui::SameLine(); ImGui::Text("WRITE"); 
        ImGui::ColorButton("READ", access_type_to_color(TaskAccessType::READ)); ImGui::SameLine(); ImGui::Text("READ"); 
        ImGui::ColorButton("READ_WRITE", access_type_to_color(TaskAccessType::READ_WRITE)); ImGui::SameLine(); ImGui::Text("READ_WRITE"); 
        ImGui::ColorButton("WRITE_CONCURRENT", access_type_to_color(TaskAccessType::WRITE_CONCURRENT)); ImGui::SameLine(); ImGui::Text("WRITE_CONCURRENT"); 
        ImGui::ColorButton("READ_WRITE_CONCURRENT", access_type_to_color(TaskAccessType::READ_WRITE_CONCURRENT)); ImGui::SameLine(); ImGui::Text("READ_WRITE_CONCURRENT"); 
        ImGui::ColorButton("IMAGE LAYOUT INIT", LAYOUT_INITIALIZATION_COLOR); ImGui::SameLine(); ImGui::Text("IMAGE LAYOUT INIT"); 
        ImGui::ColorButton("BARRIER", BARRIER_COLOR); ImGui::SameLine(); ImGui::Text("BARRIER"); 
    }

    void barrier_tooltip_ui(BatchUiData const & batch_ui)
    {
        TaskBarrier const & barrier = *batch_ui.access_group->final_schedule_pre_barrier;
        if (ImGui::BeginItemTooltip())
        {
            ImGui::Text("Barrier:");
            if (batch_ui.access_group->final_schedule_pre_barrier->src_access.stages != PipelineStageFlagBits::NONE)
            {
                ImGui::Text("Src:"); 
                ImGui::Text(std::format("  Access: {}", to_string(batch_ui.access_group->final_schedule_pre_barrier->src_access)).c_str());
                ImGui::Text("  Tasks:"); 
                ImGui::SameLine();
                auto const & src_tasks = batch_ui.access_group->final_schedule_pre_barrier->src_access_group->tasks;
                for (u32 t = 0; t < src_tasks.size(); ++t)
                {
                    ImGui::Text(src_tasks[t].task->name.data());
                    if (t < src_tasks.size() - 1)
                    {
                        ImGui::SameLine();
                        ImGui::Text(", ");
                        ImGui::SameLine();
                    }
                }
            }
            ImGui::Text("Dst:"); 
            ImGui::Text(std::format("  Access: {}", to_string(batch_ui.access_group->final_schedule_pre_barrier->dst_access)).c_str());
            ImGui::Text("  Tasks:"); 
            ImGui::SameLine();
            auto const & dst_tasks = batch_ui.access_group->tasks;
            for (u32 t = 0; t < dst_tasks.size(); ++t)
            {
                ImGui::Text(dst_tasks[t].task->name.data());
                if (t < dst_tasks.size() - 1)
                {
                    ImGui::SameLine();
                    ImGui::Text(", ");
                    ImGui::SameLine();
                }
            }
            bool const to_general = barrier.layout_operation == daxa::ImageLayoutOperation::TO_GENERAL;
            if (to_general)
            {
                ImGui::Text("Layout Transition: Undefined -> General");
            }
            ImGui::EndTooltip();
        }
    }

    void task_timeline_ui(ImplTaskGraph & impl)
    {
        static TaskGraphDebugContext context = {};
        const bool tg_changed = context.last_exec_tg_unique_index != impl.unique_index;
        context.last_exec_tg_unique_index = impl.unique_index;
        context.impl = &impl;

        /// =========================
        /// ======= HEADER UI =======
        /// =========================

        if (ImGui::Button("X"))
        {
            context.resource_name_search = {};
        }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(128);
        ImGui::InputText("Filter Resource", context.resource_name_search.data(), context.resource_name_search.size());
        ImGui::SameLine();
        if (ImGui::Button("X##1"))
        {
            context.task_name_search = {};
        }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(128);
        ImGui::InputText("Filter Task", context.task_name_search.data(), context.task_name_search.size());
        ImGui::SameLine();
        ImGui::Checkbox("Show Batch Barriers", &context.show_batch_barriers);

        static ImGuiTableFlags table_flags = ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY;
        static ImGuiTableColumnFlags column_flags = ImGuiTableColumnFlags_AngledHeader | ImGuiTableColumnFlags_WidthFixed;

        const float TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();

        if(tg_changed)
        {
            // find the resource indices based on names
            for (auto pr = context.pinned_resources.begin(); pr != context.pinned_resources.end(); ++pr)
            {
                if (impl.name_to_resource_table.contains(pr->first.c_str()))
                {
                    pr->second = impl.name_to_resource_table[pr->first.c_str()].second;
                }
                else
                {
                    pr = context.pinned_resources.erase(pr);
                }
            }
        }

        // Construct Row Ui Data
        std::vector<RowUiData> row_ui_data = {};
        std::vector<u32> resource_to_row_lookup = std::vector<u32>(impl.resources.size(), ~0u);
        for (auto const &[name, resource_index] : context.pinned_resources)
        {
            row_ui_data.push_back({
                .global_resource_index = resource_index,
                .resource = &impl.resources[resource_index],
            });
            resource_to_row_lookup[resource_index] = static_cast<u32>(row_ui_data.size() - 1);
        }
        for (u32 r = 0; r < impl.resources.size(); ++r)
        {
            ImplTaskResource & resource = impl.resources[r];

            std::string resource_name = std::string(resource.name);
            bool const search_substring_match = 
                strstr(to_lower(std::string(resource.name)).c_str(), to_lower(std::string(context.resource_name_search.data())).c_str()) != nullptr;
            if (!context.pinned_resources.contains(std::string(resource.name)) && search_substring_match)
            {
                row_ui_data.push_back({
                    .global_resource_index = r,
                    .resource = &resource,
                });
                resource_to_row_lookup[r] = static_cast<u32>(row_ui_data.size() - 1);
            }
        }

        /// ====================================================
        /// ======= BUILD COLUMNS AND BATCH INDIRECTIONS =======
        /// ====================================================

        std::vector<char const*> col_names = {}; // get rid of this
        std::vector<ColUiData> col_ui_data = {};

        col_ui_data.push_back({
            .submit_index = ~0u,
            .queue_index = ~0u,
            .local_batch_index = ~0u,
        }); // row names column
        col_names.push_back("");

        struct BatchToColumn
        {
            u64 first_col = ~0u;
            u64 col_count = ~0u;
        };
        u32 batch_count = 0u;
        for (u32 submit_index = 0; submit_index < impl.submits.size(); ++submit_index)
        {
            TasksSubmit const & submit = impl.submits[submit_index];

            col_ui_data.push_back({
                .submit_index = submit_index,
                .queue_index = ~0u,
                .local_batch_index = ~0u,
                .is_submit_border = true,
            });
            col_names.push_back("");

            for (u32 q = 0; q < submit.queue_indices.size(); ++q)
            {
                u32 queue_index = submit.queue_indices[q];
                Queue queue = queue_index_to_queue(queue_index);
                std::span<TasksBatch> queue_batches = submit.queue_batches[queue_index];

                col_ui_data.push_back({
                    .submit_index = submit_index,
                    .queue_index = queue_index,
                    .local_batch_index = ~0u,
                    .is_queue_submit_border = true,
                });
                col_names.push_back("");

                std::vector<BatchUiData> batch_ui_data = {};

                for (u32 batch_i = 0; batch_i < queue_batches.size(); ++batch_i)
                {
                    u32 const global_batch_index = submit.first_batch + batch_i;
                    batch_count = std::max(batch_count, global_batch_index + 1);
                    
                    if (context.show_batch_barriers)
                    {
                        col_ui_data.push_back({
                            .submit_index = submit_index,
                            .queue_index = queue_index,
                            .local_batch_index = batch_i,
                        });
                        col_names.push_back("");
                    }
                    batch_ui_data.push_back({});

                    for (u32 task_i = 0; task_i < queue_batches[batch_i].tasks.size(); ++task_i)
                    {
                        auto task_pair = queue_batches[batch_i].tasks[task_i];

                        bool const search_substring_match = 
                            strstr(to_lower(std::string(task_pair.first->name)).c_str(), to_lower(std::string(context.task_name_search.data())).c_str()) != nullptr;

                        if (search_substring_match)
                        {
                            col_ui_data.push_back({
                                .submit_index = submit_index,
                                .queue_index = queue_index,
                                .local_batch_index = batch_i,
                                .batch_local_task_index = task_i,
                                .global_task_index = task_pair.second,
                                .task = task_pair.first,
                                .task_index = task_pair.second,
                            });
                            col_names.push_back(task_pair.first->name.data());
                        }
                    }
                }
            }
        }

        u32 const col_count = static_cast<u32>(col_ui_data.size());
        u32 const row_count = static_cast<u32>(row_ui_data.size());

        /// ===============================================
        /// ======= BUILD CELL TO ATTACHMENT MATRIX =======
        /// ===============================================

        if (tg_changed)
        {
            context.task_resource_to_attachment_lookup.clear();
            context.task_resource_to_attachment_lookup.resize(impl.resources.size(), std::vector<u32>(impl.tasks.size(), ~0u)); // first row index, second column index
            
            // go over all columns (tasks) and write conversion attachment indices when applicable
            for (u32 t = 0; t < impl.tasks.size(); ++t)
            {
                ImplTask const & task = impl.tasks[t];
                for (u32 attach_i = 0; attach_i < task.attachments.size(); ++attach_i)
                {
                    bool const null_attachment = task.attachment_resources[attach_i].first == nullptr;
                    if (null_attachment)
                    {
                        continue;
                    }
                    u32 const global_resource_index = task.attachment_resources[attach_i].second;
                    context.task_resource_to_attachment_lookup[global_resource_index][t] = attach_i;
                }
            }
        }

        /// ==============================================
        /// ======= DETAIL RESOURCE DETAIL TASK UI =======
        /// ==============================================

        if (tg_changed)
        {
            context.name_to_task_index.clear();
            for (u32 t = 0; t < impl.tasks.size(); ++t)
            {
                if (context.name_to_task_index.contains(impl.tasks[t].name))
                {
                    // we do not allow colliding task names for ui
                    context.name_to_task_index[impl.tasks[t].name] = ~0u;
                }
                else
                {
                    context.name_to_task_index[impl.tasks[t].name] = t;
                }
            }
        }

        for (auto res_detail_iter = context.open_resource_detail_windows.begin(); res_detail_iter != context.open_resource_detail_windows.end(); )
        {
            if (!impl.name_to_resource_table.contains(*res_detail_iter))
            {
                res_detail_iter = context.open_resource_detail_windows.erase(res_detail_iter);
                continue;
            }

            if (!resource_detail_ui(context, impl.name_to_resource_table[*res_detail_iter].second))
            {
                res_detail_iter = context.open_resource_detail_windows.erase(res_detail_iter);
                continue;
            }
            
            ++res_detail_iter;
        }

        for (auto task_detail_iter = context.open_task_detail_windows.begin(); task_detail_iter != context.open_task_detail_windows.end(); )
        {
            if (!context.name_to_task_index.contains(*task_detail_iter))
            {
                task_detail_iter = context.open_task_detail_windows.erase(task_detail_iter);
                continue;
            }
            
            bool const task_forbidden = (context.name_to_task_index[std::string_view{*task_detail_iter}] == ~0u);
            if (task_forbidden)
            {
                task_detail_iter = context.open_task_detail_windows.erase(task_detail_iter);
                continue;
            }

            if (!task_details_ui(context, impl.tasks.at(context.name_to_task_index[std::string_view{*task_detail_iter}])))
            {
                task_detail_iter = context.open_task_detail_windows.erase(task_detail_iter);
                continue;
            }

            ++task_detail_iter;
        }

        /// ========================
        /// ======= UI TABLE =======
        /// ========================

        ImVec2 outer_size = ImVec2(0, 0);
        float const batch_border_cell_size = 0.35f;

        // Ui Table
        // Before every table we draw the legend in the top left
        ImVec2 cursor_before_table = ImGui::GetCursorPos();
        const ImVec2 scale = ImVec2(0.8f, 0.0f);

        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(batch_border_cell_size * scale.x, scale.y));
        if (ImGui::BeginTable(std::format("task_timeline_ui_table").c_str(), col_count, table_flags, outer_size))
        {
            /// =================================
            /// ======= MOUSE DRAG SCROLL =======
            /// =================================

            static bool is_dragging = false;
            static ImVec2 drag_start_pos;
            static ImVec2 scroll_start;
            
            // Check if middle mouse button is pressed while hovering the table
            if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Middle))
            {
                is_dragging = true;
                drag_start_pos = ImGui::GetMousePos();
                scroll_start = ImVec2(ImGui::GetScrollX(), ImGui::GetScrollY());
            }
            
            // Handle dragging
            if (is_dragging)
            {
                if (ImGui::IsMouseDown(ImGuiMouseButton_Middle))
                {
                    ImVec2 current_pos = ImGui::GetMousePos();
                    ImVec2 delta = ImVec2(drag_start_pos.x - current_pos.x, 
                                        drag_start_pos.y - current_pos.y);
                    
                    ImGui::SetScrollX(scroll_start.x + delta.x);
                    ImGui::SetScrollY(scroll_start.y + delta.y);
                }
                else
                {
                    is_dragging = false;
                }
            }

            /// =============================
            /// ======= SETUP COLUMNS =======
            /// =============================

            ImGui::TableSetupScrollFreeze(1, static_cast<u32>(2 + context.pinned_resources.size()));
            ImGui::TableSetupColumn("Resource", ImGuiTableColumnFlags_WidthFixed, ImGui::CalcTextSize("a").x * 24);
            std::vector<ImU32> colors = {};
            std::vector<ImU32> text_colors = {};
            colors.push_back(0);
            text_colors.push_back(ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Text]));
            for (i32 n = 1; n < static_cast<i32>(col_count); n++) 
            {
                bool batch_border_cell = false;
                std::string name = {};
                ImVec4 background_color = ImGui::GetStyle().Colors[ImGuiCol_TableHeaderBg];
                ImVec4 text_color = ImGui::GetStyle().Colors[ImGuiCol_Text];
                if (col_ui_data[n].task != nullptr)
                {
                    name = std::format(TIMELINE_TASK_HEADER_STR_FORMAT, col_ui_data[n].task->name);
                }
                else if (col_ui_data[n].is_submit_border)
                {
                    background_color = SUBMIT_COLOR;
                    text_color = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
                    name = std::format(TIMELINE_SUBMIT_OR_QUEUE_SUBMIT_HEADER_STR_FORMAT, std::format("Submit {}", col_ui_data[n].submit_index));
                }
                else if (col_ui_data[n].is_queue_submit_border)
                {
                    background_color = QUEUE_SUBMIT_COLOR;
                    text_color = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
                    name = std::format(TIMELINE_SUBMIT_OR_QUEUE_SUBMIT_HEADER_STR_FORMAT, to_string(queue_index_to_queue(col_ui_data[n].queue_index)));
                }
                else
                {
                    batch_border_cell = true;
                }
                colors.push_back(ImGui::ColorConvertFloat4ToU32(background_color));
                text_colors.push_back(ImGui::ColorConvertFloat4ToU32(text_color));
                ImGui::TableSetupColumn(
                    name.c_str(),
                    batch_border_cell ? (column_flags | ImGuiTableColumnFlags_WidthFixed) : column_flags,
                    batch_border_cell ? batch_border_cell_size : TEXT_BASE_HEIGHT * scale.x
                );
            }

            ImGui::TableAngledHeadersRow2(colors.data(), text_colors.data()); // Draw angled headers for all columns with the ImGuiTableColumnFlags_AngledHeader flag.
            ImGui::TableNextRow(ImGuiTableRowFlags_Headers);

            /// ======================================
            /// ======= FIRST ROW IS TASK DATA =======
            /// ======================================

            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Task Type");
            for (int column = 1; column < static_cast<i32>(col_count); column++)
            {
                ImGui::TableSetColumnIndex(column);
                ColUiData const & col_ui = col_ui_data[column]; // -1 as the first column is for the name.
                if (col_ui.is_submit_border)
                {
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::ColorConvertFloat4ToU32(SUBMIT_COLOR));
                }
                if (col_ui.task != nullptr || col_ui.is_queue_submit_border)
                {
                    char const * name = {};
                    ImVec4 color = {};
                    if (col_ui.is_queue_submit_border)
                    {
                        name = queue_family_to_str(queue_index_to_queue(col_ui.queue_index).family);
                        color = queue_family_to_color(queue_index_to_queue(col_ui.queue_index).family);
                        color.x *= 0.6f;
                        color.y *= 0.6f;
                        color.z *= 0.6f;
                        ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::ColorConvertFloat4ToU32(QUEUE_SUBMIT_COLOR));
                    }
                    else
                    {
                        name = task_type_to_str(col_ui.task->task_type);
                        color = task_type_to_color(col_ui.task->task_type);
                    }

                    ImGui::PushID(column);
                    ImGui::PushStyleColor(ImGuiCol_Text, color);
                    const float text_width = ImGui::CalcTextSize("XX").x;
                    const float column_width = ImGui::GetColumnWidth();
                    const float text_offset = (column_width - text_width) / 2;
                    ImGui::Dummy(ImVec2(0.0f, 0.0f));
                    ImGui::SameLine(text_offset);
                    ImGui::Text(name);
                    ImGui::PopStyleColor();
                    if (!col_ui.is_queue_submit_border && !col_ui.is_submit_border)
                    {
                        task_popup_context_ui(context, col_ui.global_task_index);
                    }
                    ImGui::PopID();
                }
            }

            /// ===================================
            /// ======= ROW BY ROW TABLE UI =======
            /// ===================================

            for (u32 row = 0; row < row_count; ++row)
            {
                RowUiData const & row_ui = row_ui_data[row];

                /// ========================================
                /// ======= SETUP BATCH DATA FOR ROW =======
                /// ========================================
                
                u32 resource_index = row_ui_data[row].global_resource_index;
                ImplTaskResource const& resource = impl.resources[resource_index];

                std::vector<std::array<BatchUiData, DAXA_QUEUE_COUNT>> batch_ui_data = {};
                batch_ui_data.resize(batch_count);

                // Fill this resources batch data:
                for (u32 agi = 0; agi < resource.access_timeline.size(); ++agi)
                {
                    AccessGroup const & ag = resource.access_timeline[agi];

                    u32 queue_iter = ag.queue_bits;
                    while (queue_iter)
                    {
                        u32 queue_index = queue_bits_to_first_queue_index(queue_iter);
                        queue_iter &= ~(1u << queue_index);

                        for (u32 b = ag.final_schedule_first_batch; b <= ag.final_schedule_last_batch; ++b)
                        {
                            batch_ui_data[b][queue_index].access_group = &ag;
                        }
                    }
                }

                /// ==================================================
                /// ======= FIRST COLUMN IS FOR RESOURCE NAMES =======
                /// ==================================================

                bool const is_pinned = context.pinned_resources.contains(std::string(resource.name));
                ImGui::PushID(&resource);
                ImGui::TableNextRow();
                ImGui::AlignTextToFramePadding();
                ImGui::TableSetColumnIndex(0);
                if (ImGui::Button(is_pinned ? "X" : "", ImVec2{TEXT_BASE_HEIGHT, TEXT_BASE_HEIGHT}))
                {
                    if (is_pinned)
                    {
                        if (ImGui::IsKeyDown(ImGuiKey_LeftShift))
                        {
                            context.pinned_resources.clear();
                        }
                        else
                        {
                            context.pinned_resources.erase(std::string(resource.name));
                        }
                    }
                    else
                    {
                        context.pinned_resources[std::string(resource.name)] = resource_index;
                    }
                }
                ImGui::SameLine();
                if (is_pinned) { ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::ColorConvertFloat4ToU32(ImVec4(0.5,0.5,0.5,1))); }
                ImGui::Text(resource.name.data());
                ImGui::SetItemTooltip(resource.name.data());
                resource_popup_context_ui(context, resource_index);

                /// =========================================
                /// ======= COLUMN BY COLUMN TABLE UI =======
                /// =========================================

                for (u32 column = 1; column < col_count; ++column)
                {
                    if (ImGui::TableSetColumnIndex(column))
                    {
                        ColUiData const & col_ui = col_ui_data[column]; // -1 as the first column is for the name.
                        const bool is_batch_border_cell = col_ui.task == nullptr;

                        u32 const global_batch_index = impl.submits[col_ui.submit_index].first_batch + col_ui.local_batch_index;

                        bool make_cell_darker = true;
                        ImVec4 cell_color = ImVec4(0.05f, 0.05f, 0.05f, 1.0f);
                        if (col_ui.is_submit_border)
                        {
                            cell_color = SUBMIT_COLOR;
                            make_cell_darker = false;
                        }
                        else if(col_ui.is_queue_submit_border)
                        {
                            cell_color = QUEUE_SUBMIT_COLOR;
                            make_cell_darker = false;
                        }
                        else
                        {
                            bool const is_resource_used_between_col_batches = 
                                (resource.queue_bits & (1u << col_ui.queue_index)) != 0 &&
                                global_batch_index >= resource.final_schedule_first_batch &&
                                global_batch_index <= resource.final_schedule_last_batch;
                            bool const is_resource_alive_at_col = 
                                (is_resource_used_between_col_batches &&
                                resource.access_timeline.size() > 0) ||
                                resource.external != 0;
                            if (is_resource_alive_at_col)
                            {
                                cell_color = RESOURCE_ALIVE_COLOR;
                            }


                            BatchUiData const & batch_ui = batch_ui_data[global_batch_index][col_ui.queue_index];

                            if (batch_ui.access_group != nullptr)
                            {
                                cell_color = access_type_to_color(batch_ui.access_group->type);
                            }

                            if (is_batch_border_cell) // Give Barrier Info in borders
                            {
                                if (
                                    batch_ui.access_group != nullptr && 
                                    batch_ui.access_group->final_schedule_pre_barrier != nullptr &&
                                    batch_ui.access_group->final_schedule_first_batch == global_batch_index
                                )
                                {
                                    bool const to_general = batch_ui.access_group->final_schedule_pre_barrier->layout_operation == daxa::ImageLayoutOperation::TO_GENERAL;
                                    if (to_general)
                                    {
                                        cell_color = LAYOUT_INITIALIZATION_COLOR;
                                        make_cell_darker = false;
                                    }
                                    else
                                    {
                                        cell_color = BARRIER_COLOR;
                                        make_cell_darker = false;
                                    }
                                    ImGui::Selectable("");
                                    barrier_tooltip_ui(batch_ui);
                                }
                            }
                            else // Give attachment info in Cells
                            {
                                u32 attachment_index = context.task_resource_to_attachment_lookup[resource_index][col_ui.task_index];
                                bool const resource_accessed_by_task = attachment_index != ~0u;
                                if (resource_accessed_by_task)
                                {
                                    ImGui::PushID(column + row * 60000);
                                    ImGui::Selectable("");

                                    /// ===========================
                                    /// ======= HOVER LOGIC =======
                                    /// ===========================

                                    bool const hovered_previously = 
                                        context.hovered_task == col_ui.task_index && 
                                        context.hovered_attachment_index == attachment_index &&
                                        context.hovered_resource == resource_index;

                                    bool const hovered_now = ImGui::IsItemHovered();
                                    if (hovered_now)
                                    {
                                        context.hovered_task = col_ui.task_index;
                                        context.hovered_attachment_index = attachment_index;
                                        context.hovered_resource = resource_index;
                                    }
                                    if (!hovered_now && hovered_previously)
                                    {
                                        context.hovered_task = {};
                                        context.hovered_attachment_index = {};
                                        context.hovered_resource = {};
                                    }

                                    /// ===============================
                                    /// ======= CELL POPUP MENU =======
                                    /// ===============================

                                    if (ImGui::BeginPopupContextItem(resource.name.data()))
                                    {
                                        if (ImGui::Button("Show Task Detail Ui"))
                                        {
                                            context.open_task_detail_windows.insert(std::string(col_ui.task->name));
                                        }
                                        if (ImGui::Button("Show Resource Detail Ui"))
                                        {
                                            context.open_resource_detail_windows.insert(std::string(resource.name));
                                        }
                                        ImGui::EndPopup();
                                    }
                                    if (ImGui::BeginItemTooltip())
                                    {
                                        TaskAttachmentInfo const & attach_info = col_ui.task->attachments[attachment_index];
                                        ImGui::Text(std::format("Attachment name: {}", attach_info.name()).c_str());
                                        ImGui::EndTooltip();
                                    }
                                    make_cell_darker = false;
                                    ImGui::PopID();
                                }
                            }
                        }

                        if (make_cell_darker)
                        {
                            cell_color.x = cell_color.x * 0.4f + 0.07f;
                            cell_color.y = cell_color.y * 0.4f + 0.07f;
                            cell_color.z = cell_color.z * 0.4f + 0.07f;
                        }

                        if (is_pinned)
                        {
                            cell_color.x *= 1.2f + 0.2f;
                            cell_color.y *= 1.2f + 0.2f;
                            cell_color.z *= 1.2f + 0.2f;
                        }




                        u32 const color = ImGui::ColorConvertFloat4ToU32(cell_color);

                        ImGui::PushID(column);
                        ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, color);
                        ImGui::PopID();
                    }
                }
                ImGui::PopID();
            }
            ImGui::EndTable();
        }
        ImGui::PopStyleVar();

        ImVec2 cursor_after_table = ImGui::GetCursorPos();
        ImGui::SetCursorPos(cursor_before_table);
        timeline_legend();
        ImGui::SetCursorPos(cursor_after_table);
    }

    void TaskGraph::imgui_ui()
    {
        auto & impl = *r_cast<ImplTaskGraph *>(this->object);
        if (ImGui::Begin("Taskgraph debug UI", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_HorizontalScrollbar))
        {
            ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
            if (ImGui::BeginTabBar("TgDebugUiTabBar", tab_bar_flags))
            {
                if (ImGui::BeginTabItem("Task Graph Dependencies"))
                {
                    task_dependencies_ui(impl);
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Task Timeline"))
                {
                    task_timeline_ui(impl);
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Resource Allocations"))
                {
                    ImGui::Text("This is the Cucumber tab!\nblah blah blah blah blah");
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
        }
        ImGui::End();
    }
} // namespace daxa

#else // #if DAXA_BUILT_WITH_UTILS_IMGUI && ENABLE_TASK_GRAPH_MK2

#include <daxa/utils/task_graph.hpp>

namespace daxa
{
    void TaskGraph::imgui_ui()
    {
    }
} // namespace daxa

#endif // #else // #if DAXA_BUILT_WITH_UTILS_IMGUI && ENABLE_TASK_GRAPH_MK2
#endif
