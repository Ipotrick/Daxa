#include <daxa/utils/task_graph_types.hpp>

#if DAXA_BUILT_WITH_UTILS_IMGUI && ENABLE_TASK_GRAPH_MK2

#include "impl_task_graph_mk2.hpp"
#include <daxa/utils/imgui.hpp>

static inline ImVec2 operator+(ImVec2 const & lhs, ImVec2 const & rhs) { return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y); }
static inline ImVec2 operator-(ImVec2 const & lhs, ImVec2 const & rhs) { return ImVec2(lhs.x - rhs.x, lhs.y - rhs.y); }

namespace daxa
{
    void TaskGraph::imgui_ui()
    {
        auto & impl = *r_cast<ImplTaskGraph *>(this->object);
        if (ImGui::Begin("Taskgraph debug UI", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar))
        {
            ImGui::Text("Task Timeline");

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