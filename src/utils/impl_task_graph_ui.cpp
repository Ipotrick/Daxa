#if DAXA_BUILT_WITH_UTILS_TASK_GRAPH
#include <daxa/utils/task_graph_types.hpp>

#if DAXA_BUILT_WITH_UTILS_IMGUI && ENABLE_TASK_GRAPH_MK2

#include "impl_task_graph_mk2.hpp"
#include <daxa/utils/imgui.hpp>

static inline ImVec2 operator+(ImVec2 const & lhs, ImVec2 const & rhs) { return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y); }
static inline ImVec2 operator-(ImVec2 const & lhs, ImVec2 const & rhs) { return ImVec2(lhs.x - rhs.x, lhs.y - rhs.y); }

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

    auto access_type_to_color(TaskAccessType access) -> ImVec4
    {
        ImVec4 ret = {};
        switch (access)
        {
            case TaskAccessType::NONE: ret = ImVec4(0.35638f, 0.37626f, 0.40198f, 1.0f); break; //                     #9aa0a6ff
            case TaskAccessType::WRITE: ret = ImVec4(0.70110f, 0.21223f, 0.13287f, 1.0f); break; //                    #e07a5fff
            case TaskAccessType::READ: ret = ImVec4(0.07036f, 0.23799f, 1.00000f, 1.0f); break; //                     #3A86FF
            case TaskAccessType::SAMPLED: ret = ImVec4(0.95500f, 0.58500f, 0.09300f, 1.0f); break; //                  #F9C74F
            case TaskAccessType::READ_WRITE: ret = ImVec4(0.33482f, 0.10115f, 0.78354f, 1.0f); break; //               #9B5DE5
            case TaskAccessType::WRITE_CONCURRENT: ret = ImVec4(0.78354f, 0.06301f, 0.07324f, 1.0f); break; //         #E63946
            case TaskAccessType::READ_WRITE_CONCURRENT: ret = ImVec4(0.87056f, 0.10520f, 0.48453f, 1.0f); break; //    #F15BB5
        }
        ret.x = std::sqrt(ret.x);
        ret.y = std::sqrt(ret.y);
        ret.z = std::sqrt(ret.z);
        return ret;
    }

    static constexpr ImVec4 LayoutInitializationColor = ImVec4(0.5, 1.0f, 0.5f, 1.0f);

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
        ret.x = std::sqrt(ret.x);
        ret.y = std::sqrt(ret.y);
        ret.z = std::sqrt(ret.z);
        return ret;
    }

    void access_type_color_legend()
    {
        ImGui::ColorButton("NONE", access_type_to_color(TaskAccessType::NONE)); ImGui::SameLine(); ImGui::Text("NONE"); 
        ImGui::ColorButton("WRITE", access_type_to_color(TaskAccessType::WRITE)); ImGui::SameLine(); ImGui::Text("WRITE"); 
        ImGui::ColorButton("READ", access_type_to_color(TaskAccessType::READ)); ImGui::SameLine(); ImGui::Text("READ"); 
        ImGui::ColorButton("SAMPLED", access_type_to_color(TaskAccessType::SAMPLED)); ImGui::SameLine(); ImGui::Text("SAMPLED"); 
        ImGui::ColorButton("READ_WRITE", access_type_to_color(TaskAccessType::READ_WRITE)); ImGui::SameLine(); ImGui::Text("READ_WRITE"); 
        ImGui::ColorButton("WRITE_CONCURRENT", access_type_to_color(TaskAccessType::WRITE_CONCURRENT)); ImGui::SameLine(); ImGui::Text("WRITE_CONCURRENT"); 
        ImGui::ColorButton("READ_WRITE_CONCURRENT", access_type_to_color(TaskAccessType::READ_WRITE_CONCURRENT)); ImGui::SameLine(); ImGui::Text("READ_WRITE_CONCURRENT"); 
        ImGui::ColorButton("IMAGE LAYOUT INIT", LayoutInitializationColor); ImGui::SameLine(); ImGui::Text("IMAGE LAYOUT INIT"); 
    }

    void task_timeline_ui(ImplTaskGraph & impl)
    {
        static ImGuiTableFlags table_flags = ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY;
        static ImGuiTableColumnFlags column_flags = ImGuiTableColumnFlags_AngledHeader | ImGuiTableColumnFlags_WidthFixed;

        const float TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();


        for (u32 submit_index = 0; submit_index < impl.submits.size(); ++submit_index)
        {
            TasksSubmit const & submit = impl.submits[submit_index];

            for (u32 q = 0; q < submit.queue_indices.size(); ++q)
            {
                u32 queue_index = submit.queue_indices[q];
                Queue queue = queue_index_to_queue(queue_index);
                std::span<TasksBatch> queue_batches = submit.queue_batches[queue_index];
                
                std::vector<char const*> col_names = {};
                struct ColUiData
                {
                    u32 local_batch_index = {};
                    u32 batch_local_task_index = {};
                    u32 global_task_index = {};
                    ImplTask* task = {};
                };
                std::vector<ColUiData> col_ui_data = {};

                struct BatchUiData
                {
                    TaskAccessType access_type = {};
                    TaskStage stages = {};
                };
                std::vector<BatchUiData> batch_ui_data = {};

                for (u32 batch_i = 0; batch_i < queue_batches.size(); ++batch_i)
                {
                    col_ui_data.push_back({
                        .local_batch_index = batch_i,
                    });
                    batch_ui_data.push_back({});
                    col_names.push_back("");
                    for (u32 task_i = 0; task_i < queue_batches[batch_i].tasks.size(); ++task_i)
                    {
                        auto task_pair = queue_batches[batch_i].tasks[task_i];
                        col_ui_data.push_back({
                            .local_batch_index = batch_i,
                            .batch_local_task_index = task_i,
                            .global_task_index = task_pair.second,
                            .task = task_pair.first,
                        });
                        col_names.push_back(task_pair.first->name.data());
                    }
                }

                u32 const col_count = static_cast<u32>(col_ui_data.size() + 1);
                u32 const row_count = static_cast<u32>(impl.resources.size());
                ImVec2 outer_size = ImVec2(0, 1000);
                float const batch_border_cell_size = 1.0f;

                // Before every table we draw the legend in the top left
                ImVec2 cursor_before_table = ImGui::GetCursorPos();
                                
                const float y_padding = ImGui::GetStyle().CellPadding.y;
                ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(batch_border_cell_size, y_padding));
                if (ImGui::BeginTable(std::format("task_timeline_ui_table {} {}", submit_index, queue_index).c_str(), col_count, table_flags, outer_size))
                {
                    ImGui::TableSetupScrollFreeze(1, 2);
                    ImGui::TableSetupColumn("Resource", ImGuiTableColumnFlags_WidthFixed, ImGui::CalcTextSize("a").x * 24);
                    for (i32 n = 1; n < static_cast<i32>(col_count); n++) 
                    {
                        bool const batch_border_cell = col_ui_data[n - 1].task == nullptr;
                        ImGui::TableSetupColumn(
                            col_names[n-1],
                            batch_border_cell ? (column_flags | ImGuiTableColumnFlags_WidthFixed) : column_flags,
                            batch_border_cell ? batch_border_cell_size : TEXT_BASE_HEIGHT * 1.3f
                        );
                    }

                    ImGui::TableAngledHeadersRow(); // Draw angled headers for all columns with the ImGuiTableColumnFlags_AngledHeader flag.
                    ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("Task Type");
                    for (int column = 1; column < col_count; column++)
                    {
                        ImGui::TableSetColumnIndex(column);
                        ColUiData const & col_ui = col_ui_data[column - 1]; // -1 as the first column is for the name.
                        const bool is_batch_border_cell = col_ui.task == nullptr;
                        if (!is_batch_border_cell)
                        {
                            ImGui::PushID(column);
                            ImGui::PushStyleColor(ImGuiCol_Text, task_type_to_color(col_ui.task->task_type));
                            const float text_width = ImGui::CalcTextSize("XX").x;
                            const float column_width = ImGui::GetColumnWidth();
                            const float text_offset = (column_width - text_width) / 2;
                            ImGui::Dummy(ImVec2(0.0f, 0.0f));
                            ImGui::SameLine(text_offset);
                            switch(col_ui.task->task_type)
                            {
                                case TaskType::GENERAL: ImGui::Text("GN"); break;
                                case TaskType::RASTER: ImGui::Text("RS"); break;
                                case TaskType::COMPUTE: ImGui::Text("CT"); break;
                                case TaskType::RAY_TRACING: ImGui::Text("RT"); break;
                                case TaskType::TRANSFER: ImGui::Text("TF"); break;
                            }
                            ImGui::PopStyleColor();
                            //ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::ColorConvertFloat4ToU32(task_type_to_color(col_ui.task->task_type)));
                            ImGui::PopID();
                        }
                    }
                    //ImGui::TableHeadersRow();       // Draw remaining headers and allow access to context-menu and other functions.
                    for (i32 row = 0; row < static_cast<i32>(row_count); row++)
                    {
                        ImplTaskResource const& resource = impl.resources[row];

                        // Clear Batch Ui Data:
                        for (auto& b : batch_ui_data)
                        {
                            b = {};
                        }
                        
                        // Fill this resources batch data:
                        for (u32 agi = 0; agi < resource.access_timeline.size(); ++agi)
                        {
                            AccessGroup const & ag = resource.access_timeline[agi];

                            if (ag.tasks[0].task->submit_index != submit_index) { continue; }
                            if ((ag.queue_bits & queue_index_to_queue_bit(queue_to_queue_index(queue))) == 0) { continue; }
                            
                            u32 const local_first_batch_idx = ag.final_schedule_first_batch - submit.first_batch;
                            // Need to min here as some queues have fewer batches than the access group.
                            // The access group can have more batches as it can be on different queues with more batches.
                            u32 const local_last_batch_idx = std::min(ag.final_schedule_last_batch - submit.first_batch, static_cast<u32>(batch_ui_data.size() - 1));
                            for (u32 b = local_first_batch_idx; b <= local_last_batch_idx; ++b)
                            {
                                batch_ui_data[b].access_type = ag.type;
                                batch_ui_data[b].stages = ag.stages;
                            }
                        }

                        
                        ImGui::PushID(&resource);
                        ImGui::TableNextRow();
                        ImGui::AlignTextToFramePadding();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text(resource.name.data());
                        ImGui::SetItemTooltip(resource.name.data());
                        for (i32 column = 1; column < static_cast<i32>(col_count); column++)
                        {
                            if (ImGui::TableSetColumnIndex(column))
                            {
                                ColUiData const & col_ui = col_ui_data[column - 1]; // -1 as the first column is for the name.
                                const bool is_batch_border_cell = col_ui.task == nullptr;
                                const bool is_last_cell = column == col_count - 1;

                                ImVec4 cell_color = ImVec4(0,0,0,1);
                                if ((col_ui.local_batch_index + submit.first_batch) >= resource.final_schedule_first_batch &&
                                    (col_ui.local_batch_index + submit.first_batch) <= resource.final_schedule_last_batch && 
                                    resource.access_timeline.size() > 0)
                                {
                                    cell_color = ImVec4(0.15,0.15,0.15,1);
                                }
                                
                                if (is_batch_border_cell)
                                {
                                    const bool is_not_border = col_ui.local_batch_index > 0 && col_ui.local_batch_index < static_cast<i32>(col_count) - 1;

                                    // Connect successive concurrent access batches
                                    if (is_not_border)
                                    {
                                        BatchUiData const & a = batch_ui_data[col_ui.local_batch_index - 1];
                                        BatchUiData const & b = batch_ui_data[col_ui.local_batch_index];
                                        if (a.access_type == b.access_type && is_access_concurrent(a.access_type))
                                        {
                                            cell_color = access_type_to_color(a.access_type);
                                            cell_color.x *= 0.9f;
                                            cell_color.y *= 0.9f;
                                            cell_color.z *= 0.9f;
                                        }
                                    }
                                    const bool first_transient_use = resource.external == nullptr && col_ui.local_batch_index + submit.first_batch == resource.final_schedule_first_batch;
                                    const bool is_to_general_batch = first_transient_use;
                                    if (is_to_general_batch)
                                    {
                                        cell_color = LayoutInitializationColor;
                                        ImGui::SetItemTooltip("Image Layout Undefined -> General Initialization");
                                    }
                                }
                                else
                                {
                                    BatchUiData const & batch_ui = batch_ui_data[col_ui.local_batch_index];
                                    const bool is_cell_part_of_access_group = batch_ui.access_type != TaskAccessType::NONE;
                                    if (is_cell_part_of_access_group)
                                    {
                                        cell_color = access_type_to_color(batch_ui.access_type);
                                    }
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
                access_type_color_legend();
                ImGui::SetCursorPos(cursor_after_table);
            }
            ImGui::SameLine();
        }
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
