#if DAXA_BUILT_WITH_UTILS_TASK_GRAPH
#include <daxa/utils/task_graph_types.hpp>

#if DAXA_BUILT_WITH_UTILS_IMGUI && ENABLE_TASK_GRAPH_MK2

#include "impl_task_graph_mk2.hpp"
#include <daxa/utils/imgui.hpp>
#include <imgui_internal.h>

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
    };
    struct BatchUiData
    {
        AccessGroup const * access_group = {};
    };
    struct CellToAttachment
    {
        u32 attachment_index = ~0u;
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

    auto to_lower(std::string str) -> std::string
    {
        for(auto & c : str) { c = std::tolower(c); }
        return str;
    } 

    void task_timeline_ui(ImplTaskGraph & impl)
    {
        /// =========================
        /// ======= HEADER UI =======
        /// =========================

        static std::array<char, 256> resource_name_search = {};
        ImGui::SetNextItemWidth(128);
        ImGui::InputText("Filter Resource", resource_name_search.data(), resource_name_search.size());
        ImGui::SameLine();
        static std::array<char, 256> task_name_search = {};
        ImGui::SetNextItemWidth(128);
        ImGui::InputText("Filter Task", task_name_search.data(), task_name_search.size());

        static ImGuiTableFlags table_flags = ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY;
        static ImGuiTableColumnFlags column_flags = ImGuiTableColumnFlags_AngledHeader | ImGuiTableColumnFlags_WidthFixed;

        const float TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();

        static u32 last_exec_tg_unique_index = {};
        const bool tg_changed = last_exec_tg_unique_index != impl.unique_index;
        last_exec_tg_unique_index = impl.unique_index;

        static std::unordered_map<std::string, u32> pinned_resources = {};
        if(tg_changed)
        {
            // find the resource indices based on names
            for (auto pr = pinned_resources.begin(); pr != pinned_resources.end(); ++pr)
            {
                if (impl.name_to_resource_table.contains(pr->first.c_str()))
                {
                    pr->second = impl.name_to_resource_table[pr->first.c_str()].second;
                }
                else
                {
                    pr = pinned_resources.erase(pr);
                }
            }
        }

        // Construct Row Ui Data
        std::vector<RowUiData> row_ui_data = {};
        std::vector<u32> resource_to_row_lookup = std::vector<u32>(impl.resources.size(), ~0u);
        for (auto const &[name, resource_index] : pinned_resources)
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
                strstr(to_lower(std::string(resource.name)).c_str(), to_lower(std::string(resource_name_search.data())).c_str()) != nullptr;
            if (!pinned_resources.contains(std::string(resource.name)) && search_substring_match)
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
                    
                    col_ui_data.push_back({
                        .submit_index = submit_index,
                        .queue_index = queue_index,
                        .local_batch_index = batch_i,
                    });
                    col_names.push_back("");
                    batch_ui_data.push_back({});
                    for (u32 task_i = 0; task_i < queue_batches[batch_i].tasks.size(); ++task_i)
                    {
                        auto task_pair = queue_batches[batch_i].tasks[task_i];

                        bool const search_substring_match = 
                            strstr(to_lower(std::string(task_pair.first->name)).c_str(), to_lower(std::string(task_name_search.data())).c_str()) != nullptr;

                        if (search_substring_match)
                        {
                            col_ui_data.push_back({
                                .submit_index = submit_index,
                                .queue_index = queue_index,
                                .local_batch_index = batch_i,
                                .batch_local_task_index = task_i,
                                .global_task_index = task_pair.second,
                                .task = task_pair.first,
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

        std::vector<std::vector<CellToAttachment>> cell_to_attachment_lookup = {};
        cell_to_attachment_lookup.resize(row_count, std::vector<CellToAttachment>(col_count, CellToAttachment{})); // first row index, second column index
        
        // go over all columns (tasks) and write conversion attachment indices when applicable
        for (u32 col = 0; col < col_count; ++col)
        {
            if (col_ui_data[col].task != nullptr)
            {
                ImplTask* task = col_ui_data[col].task;
                for (u32 attach_i = 0; attach_i < task->attachments.size(); ++attach_i)
                {
                    bool const null_attachment = task->attachment_resources[attach_i].first == nullptr;
                    if (null_attachment)
                    {
                        continue;
                    }
                    u32 const global_resource_index = task->attachment_resources[attach_i].second;
                    u32 const row = resource_to_row_lookup[global_resource_index];
                    bool const resource_present = row != ~0u;
                    if (resource_present)
                    {
                        cell_to_attachment_lookup[row][col] = {.attachment_index = attach_i };
                    }
                }
            }
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
            /// =============================
            /// ======= SETUP COLUMNS =======
            /// =============================

            ImGui::TableSetupScrollFreeze(1, static_cast<u32>(2 + pinned_resources.size()));
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
                    name = col_ui_data[n].task->name;
                }
                else if (col_ui_data[n].is_submit_border)
                {
                    background_color = SUBMIT_COLOR;
                    text_color = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
                    name = std::format("Submit {}", col_ui_data[n].submit_index);
                }
                else if (col_ui_data[n].is_queue_submit_border)
                {
                    background_color = QUEUE_SUBMIT_COLOR;
                    text_color = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
                    name = to_string(queue_index_to_queue(col_ui_data[n].queue_index));
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
                    if (ImGui::BeginPopupContextItem(name))
                    {
                        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, ImGui::GetStyle().FramePadding.y));
                        if (!col_ui.is_queue_submit_border && ImGui::Button("Pin All Attachment Resources"))
                        {
                            for (u32 attach_i = 0; attach_i < col_ui.task->attachment_resources.size(); ++attach_i)
                            {
                                auto const & resource_pair = col_ui.task->attachment_resources[attach_i];
                                if (resource_pair.first != nullptr)
                                {
                                    pinned_resources[std::string(impl.resources[resource_pair.second].name)] = resource_pair.second;
                                }
                            }
                        }
                        ImGui::PopStyleVar();
                        ImGui::EndPopup();
                    }
                    //ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::ColorConvertFloat4ToU32(task_type_to_color(col_ui.task->task_type)));
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

                bool const is_pinned = pinned_resources.contains(std::string(resource.name));
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
                            pinned_resources.clear();
                        }
                        else
                        {
                            pinned_resources.erase(std::string(resource.name));
                        }
                    }
                    else
                    {
                        pinned_resources[std::string(resource.name)] = resource_index;
                    }
                }
                ImGui::SameLine();
                if (is_pinned) { ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::ColorConvertFloat4ToU32(ImVec4(0.5,0.5,0.5,1))); }
                ImGui::Text(resource.name.data());
                ImGui::SetItemTooltip(resource.name.data());

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
                                u32 attachment_index = cell_to_attachment_lookup[row][column].attachment_index;
                                bool const resource_accessed_by_task = attachment_index != ~0u;
                                if (resource_accessed_by_task)
                                {
                                    ImGui::Selectable("");
                                    if (ImGui::BeginItemTooltip())
                                    {
                                        TaskAttachmentInfo const & attach_info = col_ui.task->attachments[attachment_index];
                                        ImGui::Text(std::format("Attachment name: {}", attach_info.name()).c_str());
                                        ImGui::EndTooltip();
                                    }
                                    make_cell_darker = false;
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
                            cell_color.x *= 1.5f;
                            cell_color.y *= 1.5f;
                            cell_color.z *= 1.5f;
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
