#if DAXA_BUILT_WITH_UTILS_TASK_GRAPH
#include <daxa/utils/task_graph_types.hpp>

#define TASK_GRAPH_RESOURCE_VIEWER_ONLINE_COMPILE_SHADERS 1

#if DAXA_BUILT_WITH_UTILS_IMGUI && DAXA_ENABLE_TASK_GRAPH_MK2

#include "impl_task_graph_ui.hpp"
#include <daxa/utils/imgui.hpp>
#include <imgui_internal.h>
#include <implot.h>
#include "impl_task_graph_mk2.hpp"
#include <filesystem>

#if TASK_GRAPH_RESOURCE_VIEWER_ONLINE_COMPILE_SHADERS
#include <daxa/utils/pipeline_manager.hpp>
#else // #if TASK_GRAPH_RESOURCE_VIEWER_ONLINE_COMPILE_SHADERS
#include "image_impl_resource_viewer_spv.hpp"
#endif

static inline ImVec2 operator+(ImVec2 const & lhs, ImVec2 const & rhs) { return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y); }
static inline ImVec2 operator-(ImVec2 const & lhs, ImVec2 const & rhs) { return ImVec2(lhs.x - rhs.x, lhs.y - rhs.y); }
static inline ImVec2 operator*(ImVec2 const & lhs, ImVec2 const & rhs) { return ImVec2(lhs.x * rhs.x, lhs.y * rhs.y); }
static inline ImVec2 operator/(ImVec2 const & lhs, ImVec2 const & rhs) { return ImVec2(lhs.x / rhs.x, lhs.y / rhs.y); }

namespace ImGui
{
    auto IsItemDoubleClicked(int button) -> bool
    {
        return IsItemHovered() && GetIO().MouseClickedLastCount[button] == 2 && IsMouseReleased(button);
    }

    // Unlike TableHeadersRow() it is not expected that you can reimplement or customize this with custom widgets.
    // FIXME: No hit-testing/button on the angled header.
    void TableAngledHeadersRow2(ImU32 * colors, ImU32 * text_color)
    {
        ImGuiContext & g = *GImGui;
        ImGuiTable * table = g.CurrentTable;
        ImGuiTableTempData * temp_data = table->TempData;
        temp_data->AngledHeadersRequests.resize(0);
        temp_data->AngledHeadersRequests.reserve(table->ColumnsEnabledCount);

        // Which column needs highlight?
        ImGuiID const row_id = GetID("##AngledHeaders");
        ImGuiTableInstanceData * table_instance = TableGetInstanceData(table, table->InstanceCurrent);
        int highlight_column_n = table->HighlightColumnHeader;
        if (highlight_column_n == -1 && table->HoveredColumnBody != -1)
            if (table_instance->HoveredRowLast == 0 && table->HoveredColumnBorder == -1 && (g.ActiveId == 0 || g.ActiveId == row_id || (table->IsActiveIdInTable || g.DragDropActive)))
                highlight_column_n = table->HoveredColumnBody;

        // Build up request
        // ImU32 col_header_bg = GetColorU32(ImGuiCol_TableHeaderBg);
        // ImU32 col_text = GetColorU32(ImGuiCol_Text);
        for (int order_n = 0; order_n < table->ColumnsCount; order_n++)
            if (IM_BITARRAY_TESTBIT(table->EnabledMaskByDisplayOrder, order_n))
            {
                int const column_n = table->DisplayOrderToIndex[order_n];
                ImGuiTableColumn * column = &table->Columns[column_n];
                if ((column->Flags & ImGuiTableColumnFlags_AngledHeader) == 0) // Note: can't rely on ImGuiTableColumnFlags_IsVisible test here.
                    continue;
                ImGuiTableHeaderData request = {(ImGuiTableColumnIdx)column_n, text_color[order_n], colors[order_n], 0};
                temp_data->AngledHeadersRequests.push_back(request);
            }

        // Render row
        TableAngledHeadersRowEx(row_id, g.Style.TableAngledHeadersAngle, 0.0f, temp_data->AngledHeadersRequests.Data, temp_data->AngledHeadersRequests.Size);
    }

    void OpenAndDockWindowSide(ImGuiDir split_dir, float split, char const * target_window, char const * new_window_name, void const * new_window_id = nullptr)
    {
        ImGuiWindow * main_window = ImGui::FindWindowByName(target_window);

        // Call ui into existence
        if (new_window_id)
        {
            ImGui::PushID(new_window_id);
        }

        bool visible = ImGui::Begin(new_window_name);
        ImGui::End();
        if (new_window_id)
        {
            ImGui::PopID();
        }
        ImGuiWindow * window_to_dock = ImGui::FindWindowByName(new_window_name);

        bool const window_already_in_dock_node = main_window->DockNode != nullptr;

        if (main_window && visible)
        {
            ImGuiContext * g = ImGui::GetCurrentContext();

            if (window_already_in_dock_node)
            {
                bool const has_parent = main_window->DockNode->ParentNode != nullptr;
                bool const has_right_docked = has_parent && main_window->DockNode->ParentNode->ChildNodes[1]->Windows.Size > 0;
                auto target_node = has_right_docked ? main_window->DockNode->ParentNode->ChildNodes[1] : main_window->DockNode;
                bool const already_docked = target_node == window_to_dock->DockNode;
                if (!already_docked)
                {
                    ImGui::DockContextQueueDock(g, main_window, target_node, window_to_dock, has_right_docked ? ImGuiDir_None : split_dir, split, false);
                }
            }
            else
            {
                ImGui::DockContextQueueDock(g, main_window, nullptr, window_to_dock, split_dir, split, false);
            }
        }
    }
} // namespace ImGui

namespace daxa
{
    static std::unordered_map<std::string, ImplTaskGraphDebugUi> contexts = {};

    auto destroy_resource_viewer(ImplTaskGraphDebugUi & ui_context, std::string const & resource_name) -> decltype(ui_context.resource_viewer_states.begin())
    {
        auto viewer_iter = ui_context.resource_viewer_states.find(resource_name);
        if (!viewer_iter->second.image.destroy_clone_image.is_empty())
        {
            ui_context.device.destroy_image(viewer_iter->second.image.destroy_clone_image);
            viewer_iter->second.image.destroy_clone_image = {};
        }
        if (!viewer_iter->second.image.destroy_display_image.is_empty())
        {
            ui_context.device.destroy_image(viewer_iter->second.image.destroy_display_image);
            viewer_iter->second.image.destroy_display_image = {};
        }
        if (!viewer_iter->second.image.clone_image.is_empty())
        {
            ui_context.device.destroy_image(viewer_iter->second.image.clone_image);
            viewer_iter->second.image.clone_image = {};
        }
        if (!viewer_iter->second.image.display_image.is_empty())
        {
            ui_context.device.destroy_image(viewer_iter->second.image.display_image);
            viewer_iter->second.image.display_image = {};
        }
        if (!viewer_iter->second.image.readback_buffer.is_empty())
        {
            ui_context.device.destroy_buffer(viewer_iter->second.image.readback_buffer);
            viewer_iter->second.image.readback_buffer = {};
        }
        if (ui_context.device.is_buffer_id_valid(viewer_iter->second.buffer.clone_buffer))
        {
            ui_context.device.destroy_buffer(viewer_iter->second.buffer.clone_buffer);
        }
        if (ui_context.device.is_buffer_id_valid(viewer_iter->second.buffer.destroy_clone_buffer))
        {
            ui_context.device.destroy_buffer(viewer_iter->second.buffer.destroy_clone_buffer);
        }
        for (auto & child_viewer : viewer_iter->second.buffer.child_buffer_inspectors)
        {
            if (ui_context.device.is_buffer_id_valid(child_viewer.second.clone_buffer))
            {
                ui_context.device.destroy_buffer(child_viewer.second.clone_buffer);
            }
            if (ui_context.device.is_buffer_id_valid(child_viewer.second.destroy_clone_buffer))
            {
                ui_context.device.destroy_buffer(child_viewer.second.destroy_clone_buffer);
            }
        }
        return ui_context.resource_viewer_states.erase(viewer_iter);
    }

    void open_or_focus_resource_detail_ui(ImplTaskGraphDebugUi & ui_context, ImplTaskGraph * impl_tg, u32 resource_index)
    {
        ImplTaskResource const & resource = impl_tg->resources[resource_index];
        bool const detail_window_open = ui_context.open_resource_detail_windows.contains(std::string(resource.name).c_str());
        if (detail_window_open)
        {
            ImGui::SetWindowFocus(resource.name.data());
        }
        else
        {
            ui_context.open_resource_detail_windows.insert(std::string(resource.name));
            ImGui::OpenAndDockWindowSide(ImGuiDir_Right, 0.7f, "Task Graph Debug UI", resource.name.data());
        }
    }

    void open_or_focus_resource_viewer(ImplTaskGraphDebugUi & ui_context, ImplTaskGraph * impl_tg, u32 resource_index, u32 access_timeline_index = ~0u)
    {
        ImplTaskResource const & resource = impl_tg->resources[resource_index];
        
        if (resource.kind != TaskResourceKind::IMAGE && resource.kind != TaskResourceKind::BUFFER)
        {
            return;
        }

        bool const viewer_exists = ui_context.resource_viewer_states.contains(std::string(resource.name));
        auto const viewer_name = std::format("{}::Viewer", resource.name);

        DAXA_DBG_ASSERT_TRUE_M(access_timeline_index == ~0u || access_timeline_index < resource.access_timeline.size(), "IMPOSSIBLE CASE! Access index is out of resource access bounds");

        if (viewer_exists)
        {
            ResourceViewerState & state = ui_context.resource_viewer_states.at(std::string(resource.name));
            if (access_timeline_index != ~0u)
            {
                state.timeline_index = access_timeline_index;
            }
            if (state.free_window)
            {
                ImGui::SetWindowFocus(viewer_name.c_str());
            }
            else
            {
                open_or_focus_resource_detail_ui(ui_context, impl_tg, resource_index);
            }
        }
        else
        {
            open_or_focus_resource_detail_ui(ui_context, impl_tg, resource_index);

            if (resource.access_timeline.size() == 0)
            {
                return;
            }

            DAXA_DBG_ASSERT_TRUE_M(!ui_context.resource_viewer_states.contains(std::string(resource.name)), "IMPOSSIBLE CASE! Internal logic error in resource viewer ui!");
            ui_context.resource_viewer_states[std::string(resource.name)] = {};
            
            if (impl_tg->resources[resource_index].kind == TaskResourceKind::BUFFER)
            {
                if(ui_context.buffer_layout_cache_folder.has_value())
                {
                    // read the file from disk
                    std::string file_path = std::format("{}/{}_{}.dbgstruct", ui_context.buffer_layout_cache_folder.value().string(), std::string(impl_tg->info.name.data()), std::string(resource.name.data()));
                    std::ifstream infile(file_path);
                    if (infile.is_open())
                    {
                        ui_context.resource_viewer_states[std::string(resource.name)].buffer.format_c_struct_code = std::string((std::istreambuf_iterator<char>(infile)), std::istreambuf_iterator<char>());
                        infile.close();
                    }
                }
            }
            if (impl_tg->resources[resource_index].kind == TaskResourceKind::IMAGE)
            {
                ui_context.resource_viewer_states[std::string(resource.name)].image.readback_buffer = ui_context.device.create_buffer({
                    .size = sizeof(TaskGraphDebugUiImageReadbackStruct) * READBACK_CIRCULAR_BUFFER_SIZE,
                    // We perform atomics on the readback buffer so we want the memory to be on the GPU even though it is still read by the CPU.
                    .allocate_info = MemoryFlagBits::HOST_ACCESS_SEQUENTIAL_WRITE,
                    .name = std::format("{} readback buffer", resource.name),
                });
            }
            if (access_timeline_index != ~0u)
            {
                ui_context.resource_viewer_states[std::string(resource.name)].timeline_index = access_timeline_index;
            }
        }
    }

    auto to_lower(std::string str) -> std::string
    {
        for (auto & c : str)
        {
            c = std::tolower(c);
        }
        return str;
    }

    auto task_type_to_str(TaskType task_type) -> char const *
    {
        switch (task_type)
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
        switch (queue_family)
        {
        case QueueFamily::MAIN: return "GN";
        case QueueFamily::COMPUTE: return "CT";
        case QueueFamily::TRANSFER: return "TF";
        default: return "UNKNOWN";
        }
    }

    auto access_type_to_color(TaskAccessType access) -> ImVec4
    {
        ImVec4 ret = {};
        switch (access)
        {
        case TaskAccessType::NONE: ret = ColorPalette::GREY; break;
        case TaskAccessType::WRITE: ret = ColorPalette::RED; break;
        case TaskAccessType::READ: ret = ColorPalette::GREEN; break;
        case TaskAccessType::SAMPLED: ret = ColorPalette::GREEN; break;
        case TaskAccessType::READ_WRITE: ret = ColorPalette::BLUE; break;
        case TaskAccessType::WRITE_CONCURRENT: ret = ColorPalette::DARK_RED; break;
        case TaskAccessType::READ_WRITE_CONCURRENT: ret = ColorPalette::DARK_BLUE; break; 
        }
        return ret;
    }

    static constexpr ImVec4 LAYOUT_INITIALIZATION_COLOR = ColorPalette::YELLOW;
    static constexpr ImVec4 BARRIER_COLOR = ColorPalette::YELLOW;
    static constexpr ImVec4 SUBMIT_COLOR = ImVec4(0.4f, 0.4f, 0.4f, 1.0f);
    static constexpr ImVec4 QUEUE_SUBMIT_COLOR = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
    static constexpr ImVec4 RESOURCE_ALIVE_COLOR = ImVec4(0.4f, 0.4f, 0.4f, 1.0f);

    // Formats the header names to always be exactly 36 chars wide
    static constexpr char const * TIMELINE_SUBMIT_OR_QUEUE_SUBMIT_HEADER_STR_FORMAT = "{:^36.36}";
    static constexpr char const * TIMELINE_TASK_HEADER_STR_FORMAT = "{:<36.36}";

    auto task_type_to_color(TaskType type) -> ImVec4
    {
        ImVec4 ret = {};
        switch (type)
        {
        case TaskType::GENERAL: ret = ImVec4(0.47060f, 0.52941f, 0.61960f, 1.0f); break;     //        #8D99AE
        case TaskType::RASTER: ret = ImVec4(0.70110f, 0.21223f, 0.13287f, 1.0f); break;      //         #e07a5fff
        case TaskType::COMPUTE: ret = ImVec4(0.95500f, 0.58500f, 0.09300f, 1.0f); break;     //        #F9C74F
        case TaskType::RAY_TRACING: ret = ImVec4(0.70110f, 0.02113f, 0.04362f, 1.0f); break; //    #EF233C
        case TaskType::TRANSFER: ret = ImVec4(0.03112f, 0.78413f, 0.60552f, 1.0f); break;    //       #06D6A0
        }
        return ret;
    }

    auto resource_kind_to_color(TaskResourceKind const kind)
    {
        ImVec4 ret = {};

        switch (kind)
        {
        case TaskResourceKind::BUFFER: ret = ImVec4(0.1804f, 0.8000f, 0.4431f, 1.0f); break;
        case TaskResourceKind::IMAGE: ret = ImVec4(0.2039f, 0.5961f, 0.8588f, 1.0f); break;
        case TaskResourceKind::BLAS: ret = ImVec4(0.6078f, 0.3490f, 0.7137f, 1.0f); break;
        case TaskResourceKind::TLAS: ret = ImVec4(0.9451f, 0.7686f, 0.0588f, 1.0f); break;
        }
        return ret;
    }

    auto queue_family_to_color(QueueFamily type) -> ImVec4
    {
        ImVec4 ret = {};
        switch (type)
        {
        case QueueFamily::MAIN: ret = ImVec4(0.47060f, 0.52941f, 0.61960f, 1.0f); break;     //           #8D99AE
        case QueueFamily::COMPUTE: ret = ImVec4(0.95500f, 0.58500f, 0.09300f, 1.0f); break;  //        #F9C74F
        case QueueFamily::TRANSFER: ret = ImVec4(0.03112f, 0.78413f, 0.60552f, 1.0f); break; //       #06D6A0
        }
        // ret.x = std::sqrt(ret.x);
        // ret.y = std::sqrt(ret.y);
        // ret.z = std::sqrt(ret.z);
        return ret;
    }

    void colored_banner_text(std::string text, bool draw_circle, ImVec4 banner_color, ImVec4 circle_color)
    {
        float const text_height = ImGui::CalcTextSize("X").y;

        ImDrawList * draw_list = ImGui::GetWindowDrawList();
        draw_list->ChannelsSplit(2);

        auto const pre_text_pos = ImGui::GetCursorPos();
        {
            draw_list->ChannelsSetCurrent(1);
            if (draw_circle)
            {
                draw_list->AddCircleFilled(ImVec2(ImGui::GetCursorScreenPos().x + 10.0f, ImGui::GetCursorScreenPos().y + text_height / 2.0f), 6.0f, ImGui::ColorConvertFloat4ToU32(ImVec4(circle_color)));
                ImGui::SameLine(15, 15);
            }
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1));
            ImGui::Text(text.c_str());
            ImGui::PopStyleColor();
            ImGui::SameLine();
        }
        auto const post_text_pos = ImGui::GetCursorPos();
        {
            ImGui::Dummy({0, text_height});
            draw_list->ChannelsSetCurrent(0);
            ImGui::SetCursorPos(pre_text_pos);
            draw_list->AddRectFilled(ImGui::GetCursorScreenPos(), ImVec2((ImGui::GetCursorScreenPos().x + post_text_pos.x - pre_text_pos.x), ImGui::GetCursorScreenPos().y + text_height), ImGui::ColorConvertFloat4ToU32(banner_color), 5.0f);
            draw_list->ChannelsMerge();
        }
    }

    void pin_resource_checkbox(ImplTaskGraphDebugUi & ui_context, ImplTaskGraph * impl_tg, u32 resource_index)
    {
        ImplTaskResource const & resource = impl_tg->resources[resource_index];
        bool const pinned = ui_context.pinned_resources.contains(std::string(resource.name));
        bool gui_pinned = pinned;
        if (ImGui::Checkbox("Pin", &gui_pinned))
        {
            if (pinned)
            {
                ui_context.pinned_resources.erase(std::string(resource.name));
            }
            else
            {
                ui_context.pinned_resources[std::string(resource.name)] = resource_index;
            }
        }
    }

    void filter_resource_checkbox(ImplTaskGraphDebugUi & ui_context, ImplTaskGraph * impl_tg, u32 resource_index)
    {
        ImplTaskResource const & resource = impl_tg->resources[resource_index];
        bool const search_contains_resource_name =
            strstr(to_lower(std::string(ui_context.resource_name_search.data())).c_str(), to_lower(std::string(resource.name)).c_str()) != nullptr;
        bool gui_contains_name = search_contains_resource_name;
        if (ImGui::Checkbox("Filter", &gui_contains_name))
        {
            if (search_contains_resource_name)
            {
                ui_context.resource_name_search = {};
            }
            else
            {
                for (u32 i = 0; i < std::min(255ull, resource.name.size()); ++i)
                {
                    ui_context.resource_name_search[i] = resource.name[i];
                }
                ui_context.resource_name_search[std::min(255ull, resource.name.size())] = 0;
            }
        }
    }

    void pin_task_attachments_checkbox(ImplTaskGraphDebugUi & ui_context, ImplTaskGraph * impl_tg, u32 task_index)
    {
        ImplTask const & task = impl_tg->tasks[task_index];
        bool pinned = true;
        for (auto const & attach_resource_pair : task.attachment_resources)
        {
            if (attach_resource_pair.first == nullptr)
            {
                continue;
            }
            pinned &= ui_context.pinned_resources.contains(std::string(attach_resource_pair.first->name));
            if (!pinned)
            {
                break;
            }
        }
        bool gui_pinned = pinned;
        if (ImGui::Checkbox("Pin all resources", &gui_pinned))
        {
            if (pinned)
            {
                for (auto const & attach_resource_pair : task.attachment_resources)
                {
                    if (attach_resource_pair.first != nullptr && ui_context.pinned_resources.contains(std::string(attach_resource_pair.first->name)))
                    {
                        ui_context.pinned_resources.erase(std::string(attach_resource_pair.first->name));
                    }
                }
            }
            else
            {
                for (auto const & attach_resource_pair : task.attachment_resources)
                {
                    if (attach_resource_pair.first != nullptr && !ui_context.pinned_resources.contains(std::string(attach_resource_pair.first->name)))
                    {
                        ui_context.pinned_resources[std::string(attach_resource_pair.first->name)] = attach_resource_pair.second;
                    }
                }
            }
        }
    }

    void filter_task_checkbox(ImplTaskGraphDebugUi & ui_context, ImplTaskGraph * impl_tg, u32 task_index)
    {
        ImplTask const & task = impl_tg->tasks[task_index];
        bool const search_contains_task_name =
            strstr(to_lower(std::string(ui_context.task_name_search.data())).c_str(), to_lower(std::string(task.name)).c_str()) != nullptr;
        bool gui_contains_name = search_contains_task_name;
        if (ImGui::Checkbox("Filter", &gui_contains_name))
        {
            if (search_contains_task_name)
            {
                ui_context.task_name_search = {};
            }
            else
            {
                for (u32 i = 0; i < std::min(255ull, task.name.size()); ++i)
                {
                    ui_context.task_name_search[i] = task.name[i];
                }
                ui_context.task_name_search[std::min(255ull, task.name.size())] = 0;
            }
        }
    }

    void highlight_task_checkbox(ImplTaskGraphDebugUi & ui_context, ImplTaskGraph * impl_tg, u32 task_index)
    {
        ImplTask const & task = impl_tg->tasks[task_index];
        bool const marked = ui_context.extra_highlighted_tasks.contains(std::string(task.name));
        bool gui_marked = marked;
        if (ImGui::Checkbox("Highlight", &gui_marked))
        {
            if (marked)
            {
                ui_context.extra_highlighted_tasks.erase(std::string(task.name));
            }
            else
            {
                ui_context.extra_highlighted_tasks.insert(std::string(task.name));
            }
        }
    }

    void highlight_all_tasks_checkbox(ImplTaskGraphDebugUi & ui_context, ImplTaskGraph * impl_tg, u32 resource_index)
    {
        ImplTaskResource const & resource = impl_tg->resources[resource_index];
        bool highlighted = true;
        for (u32 agi = 0; agi < resource.access_timeline.size(); ++agi)
        {
            AccessGroup const & ag = resource.access_timeline[agi];
            for (u32 t = 0; t < ag.tasks.size(); ++t)
            {
                highlighted &= ui_context.extra_highlighted_tasks.contains(std::string(ag.tasks[t].task->name));
                if (!highlighted)
                {
                    break;
                }
            }
        }
        bool gui_highlighted = highlighted;
        if (ImGui::Checkbox("Highlight all tasks", &gui_highlighted))
        {
            if (highlighted)
            {
                for (u32 agi = 0; agi < resource.access_timeline.size(); ++agi)
                {
                    AccessGroup const & ag = resource.access_timeline[agi];
                    for (u32 t = 0; t < ag.tasks.size(); ++t)
                    {
                        if (ui_context.extra_highlighted_tasks.contains(std::string(ag.tasks[t].task->name)))
                        {
                            ui_context.extra_highlighted_tasks.erase(std::string(ag.tasks[t].task->name));
                        }
                    }
                }
            }
            else
            {
                for (u32 agi = 0; agi < resource.access_timeline.size(); ++agi)
                {
                    AccessGroup const & ag = resource.access_timeline[agi];
                    for (u32 t = 0; t < ag.tasks.size(); ++t)
                    {
                        if (!ui_context.extra_highlighted_tasks.contains(std::string(ag.tasks[t].task->name)))
                        {
                            ui_context.extra_highlighted_tasks.insert(std::string(ag.tasks[t].task->name));
                        }
                    }
                }
            }
        }
    }

    void resource_viewer_checkbox(ImplTaskGraphDebugUi & ui_context, ImplTaskGraph * impl_tg, u32 resource_index)
    {
        ImplTaskResource const & resource = impl_tg->resources[resource_index];
        if (resource.kind != TaskResourceKind::IMAGE && resource.kind != TaskResourceKind::BUFFER)
        {
            return;
        }

        bool const viewer_exists = ui_context.resource_viewer_states.contains(std::string(resource.name));

        bool gui_viewer_exists = viewer_exists;
        if (ImGui::Checkbox("Resource Viewer", &gui_viewer_exists))
        {
            if (viewer_exists)
            {
                destroy_resource_viewer(ui_context, std::string(resource.name));
            }
            else
            {
                open_or_focus_resource_viewer(ui_context, impl_tg, resource_index);
            }
        }
    }

    auto task_popup_context_ui(ImplTaskGraphDebugUi & ui_context, ImplTaskGraph * impl_tg, u32 task_index, u32 attachment_index = ~0u)
    {
        ImplTask const & task = impl_tg->tasks[task_index];
        bool const detail_window_open = ui_context.open_task_detail_windows.contains(std::string(task.name).c_str());
        if (ImGui::IsItemDoubleClicked(0))
        {
            if (detail_window_open)
            {
                ImGui::SetWindowFocus(task.name.data());
            }
            else
            {
                ui_context.open_task_detail_windows.insert(std::string(task.name));
                ImGui::OpenAndDockWindowSide(ImGuiDir_Right, 0.7f, "Task Graph Debug UI", task.name.data());
            }
        }
        if (ImGui::BeginPopupContextItem(task.name.data()))
        {
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, ImGui::GetStyle().FramePadding.y));
            ImGui::Text(task.name.data());
            if (ImGui::Button("Open Task Detail Ui (Double Click)"))
            {
                if (detail_window_open)
                {
                    ImGui::SetWindowFocus(task.name.data());
                }
                else
                {
                    ui_context.open_task_detail_windows.insert(std::string(task.name));
                    ImGui::OpenAndDockWindowSide(ImGuiDir_Right, 0.7f, "Task Graph Debug UI", task.name.data());
                }
            }
            pin_task_attachments_checkbox(ui_context, impl_tg, task_index);
            filter_task_checkbox(ui_context, impl_tg, task_index);
            highlight_task_checkbox(ui_context, impl_tg, task_index);
            if (attachment_index != ~0u)
            {
                u32 resource_index = task.attachment_resources[attachment_index].second;
                resource_viewer_checkbox(ui_context, impl_tg, resource_index);

                ImplTaskResource & resource = impl_tg->resources[resource_index];
                bool const viewer_exists = ui_context.resource_viewer_states.contains(std::string(resource.name));
                if (viewer_exists)
                {
                    if (ImGui::Button("Set Viewer access to task"))
                    {
                        ui_context.resource_viewer_states[std::string(resource.name)].timeline_index = task.attachment_access_groups[attachment_index].second;
                    }
                }
            }
            ImGui::PopStyleVar();
            ImGui::EndPopup();
        }
    }

    auto resource_popup_context_ui(
        ImplTaskGraphDebugUi & ui_context,
        ImplTaskGraph * impl_tg,
        u32 resource_index,
        bool show_resource_viewer_checkbox = false,
        u32 access_timeline_index = ~0u)
    {
        if (resource_index == ~0u)
        {
            return;
        }
        ImplTaskResource const & resource = impl_tg->resources[resource_index];
        bool const detail_window_open = ui_context.open_resource_detail_windows.contains(std::string(resource.name).c_str());
        static bool double_clicked_in_previous_frame = {};
        if (ImGui::IsItemDoubleClicked(0))
        {
            open_or_focus_resource_detail_ui(ui_context, impl_tg, resource_index);
            open_or_focus_resource_viewer(ui_context, impl_tg, resource_index, access_timeline_index);
        }
        double_clicked_in_previous_frame = ImGui::IsMouseDoubleClicked(0);
        if (ImGui::BeginPopupContextItem(resource.name.data()))
        {
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, ImGui::GetStyle().FramePadding.y));
            ImGui::Text(resource.name.data());
            if (ImGui::Button("Open Resource Detail Ui (Double Click)"))
            {
                open_or_focus_resource_detail_ui(ui_context, impl_tg, resource_index);
                open_or_focus_resource_viewer(ui_context, impl_tg, resource_index, access_timeline_index);
            }
            filter_resource_checkbox(ui_context, impl_tg, resource_index);
            pin_resource_checkbox(ui_context, impl_tg, resource_index);
            highlight_all_tasks_checkbox(ui_context, impl_tg, resource_index);
            if (show_resource_viewer_checkbox)
            {
                resource_viewer_checkbox(ui_context, impl_tg, resource_index);
            }
            if (access_timeline_index != ~0u)
            {
                ImplTaskResource & resource = impl_tg->resources[resource_index];
                bool const viewer_exists = ui_context.resource_viewer_states.contains(std::string(resource.name));
                if (viewer_exists)
                {
                    if (ImGui::Button("Set Viewer access to task"))
                    {
                        ui_context.resource_viewer_states[std::string(resource.name)].timeline_index = access_timeline_index;
                    }
                }
            }
            ImGui::PopStyleVar();
            ImGui::EndPopup();
        }
    }

    void memory_ui(ImplTaskGraphDebugUi & ui_context, ImplTaskGraph * impl_tg)
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

        ImVec2 const mouse_pos = io.MousePos;
        bool hovered = mouse_pos.x > screen_canvas_top_left_corner.x && mouse_pos.x < screen_canvas_bottom_right_corner.x &&
                       mouse_pos.y > screen_canvas_top_left_corner.y && mouse_pos.y < screen_canvas_bottom_right_corner.y;
        if (hovered)
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

        // Pan (we use a zero mouse threshold when there's no ui_context menu)
        // You may decide to make that threshold dynamic based on whether the mouse is hovering something etc.
        float const mouse_threshold_for_pan = 0.0f;
        if (/*is_active &&*/ ImGui::IsMouseDragging(ImGuiMouseButton_Right, mouse_threshold_for_pan))
        {
            world_canvas_offset.x -= io.MouseDelta.x / world_canvas_scale;
            world_canvas_offset.y -= io.MouseDelta.y / world_canvas_scale;
        }

        // Context menu (under default mouse threshold)
        ImVec2 drag_delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);

        float const GRID_STEP = 50.0f;
        // Draw grid + all lines in the canvas
        draw_list->PushClipRect(screen_canvas_top_left_corner, screen_canvas_bottom_right_corner, true);
        {
            draw_list->ChannelsSplit(2);
            /// ==============================
            /// ==== DRAW BACKGROUND GRID ====
            /// ==============================
            {
                draw_list->ChannelsSetCurrent(0); // Foreground
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

            /// ===================================
            /// ==== DRAW RESOURCE MEMORY HEAP ====
            /// ===================================

            ImVec2 const max_extent = ImVec2(impl_tg->flat_batch_count, impl_tg->transient_memory_block.info().requirements.size);
            ImVec2 const world_extent = ImVec2((max_extent.x) * GRID_STEP, (max_extent.y / (8192.0f * 1024 * 4)) * GRID_STEP);

            float const width_scale = 1.0f / max_extent.x * world_extent.x;

            draw_list->ChannelsSetCurrent(0); // Background
            draw_list->AddRectFilled(world_to_screen(ImVec2(0, 0)), world_to_screen(ImVec2(world_extent.x - GRID_STEP, world_extent.y)), IM_COL32(80, 80, 100, 127), 0.0f);
            draw_list->ChannelsMerge();
            for (u32 resource_index = 0; resource_index < impl_tg->resources.size(); ++resource_index)
            {
                ImplTaskResource const & resource = impl_tg->resources[resource_index];

                if (resource.external)
                {
                    continue;
                }

                ImVec2 start = ImVec2(
                    resource.final_schedule_first_batch * width_scale,
                    (resource.allocation_offset / max_extent.y) * world_extent.y);

                ImVec2 end = ImVec2(
                    resource.final_schedule_last_batch * width_scale + width_scale,
                    ((resource.allocation_offset + resource.allocation_size) / max_extent.y) * world_extent.y);

                {
                    auto clip = [&](ImVec2 in) -> ImVec2
                    {
                        return ImVec2(
                            std::clamp(in.x, screen_canvas_top_left_corner.x, screen_canvas_bottom_right_corner.x),
                            std::clamp(in.y, screen_canvas_top_left_corner.y, screen_canvas_bottom_right_corner.y));
                    };

                    auto const clipped_start = clip(world_to_screen(start));
                    auto const clipped_end = clip(world_to_screen(end));
                    bool const fully_clipped = clipped_start.x == clipped_end.x || clipped_start.y == clipped_end.y;
                    if (!fully_clipped)
                    {
                        ImGui::SetCursorScreenPos(clipped_start);
                        auto const size = ImVec2(clipped_end.x - clipped_start.x, clipped_end.y - clipped_start.y);
                        ImGui::InvisibleButton(std::format("##LALA{}", resource.name.data()).c_str(), size);
                        ImGui::SetItemTooltip(resource.name.data());
                        resource_popup_context_ui(ui_context, impl_tg, resource_index, true);
                        ImVec4 const fill_color = resource_kind_to_color(resource.kind);
                        ImVec4 const fill_color_final = ImGui::IsItemHovered() ? ImVec4(fill_color.x * 1.3f, fill_color.y * 1.3f, fill_color.z * 1.3f, 1.0f) : fill_color;
                        ImVec4 const border_color = ImVec4(fill_color_final.x * 0.4f, fill_color_final.y * 0.4f, fill_color_final.z * 0.4f, 1.0f);
                        draw_list->AddRectFilled(world_to_screen(start), world_to_screen(end), ImGui::ColorConvertFloat4ToU32(fill_color_final), 0.0f);
                        draw_list->AddRect(world_to_screen(start), world_to_screen(end), ImGui::ColorConvertFloat4ToU32(border_color), 0.0f, 0, 2 * world_canvas_scale);
                    }
                }
            }

            /// ====================
            /// ==== DRAW TASKS ====
            /// ====================
            if (false)
            {
                float const WS_BATCH_HORIZONTAL_DISTANCE = 256.0f;
                float const WS_TASK_VERTICAL_DISTANCE = 256.0f;
                ImVec2 const NODE_WINDOW_PADDING(8.0f, 8.0f);

                float acc_submit_hor_offset = 0.0f;
                for (u32 submit_index = 0; submit_index < impl_tg->submits.size(); ++submit_index)
                {
                    TasksSubmit const & submit = impl_tg->submits[submit_index];

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

                    acc_submit_hor_offset += ((submit.final_schedule_last_batch - submit.final_schedule_first_batch + 1) + 1) * WS_BATCH_HORIZONTAL_DISTANCE;
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
        ImplTask * task = {};
    };
    struct RowUiData
    {
        u32 global_resource_index = {};
        ImplTaskResource * resource = {};
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
        ImplTask * task = {};
        u32 task_index = ~0u;
    };
    struct BatchUiData
    {
        AccessGroup const * access_group = {};
    };

    auto task_details_ui(ImplTaskGraphDebugUi & ui_context, ImplTaskGraph * impl_tg, u32 task_idx) -> bool
    {
        ImplTask const & task = impl_tg->tasks[task_idx];
        bool open = true;
        ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
        if (ImGui::Begin(task.name.data(), &open, ImGuiWindowFlags_NoSavedSettings))
        {
            if (ImGui::IsWindowHovered())
            {
                ui_context.hovered_task = task_idx;
            }
            else if (ui_context.hovered_task == task_idx)
            {
                ui_context.hovered_task = ~0;
            }

            pin_task_attachments_checkbox(ui_context, impl_tg, task_idx);
            ImGui::SameLine();
            filter_task_checkbox(ui_context, impl_tg, task_idx);
            ImGui::SameLine();
            highlight_task_checkbox(ui_context, impl_tg, task_idx);

            if (ImGui::BeginTable("##task_details_ui", 2, ImGuiTableFlags_SizingFixedFit))
            {
                ImGui::TableSetupColumn("##col1");
                ImGui::TableSetupColumn("##col2");

                ImGui::TableNextColumn();
                ImGui::Text("Task Type");
                ImGui::TableNextColumn();
                ImVec4 const task_type_color = task_type_to_color(task.task_type);
                colored_banner_text(
                    std::format("{}", task_type_to_str(task.task_type), task.queue.index),
                    true,
                    task_type_color,
                    ImVec4(task_type_color.x * 0.6f, task_type_color.y * 0.6f, task_type_color.z * 0.6f, 1.0f));

                ImGui::TableNextColumn();
                ImGui::Text("Submit");
                ImGui::TableNextColumn();
                ImGui::Text("%i", task.submit_index);

                ImGui::TableNextColumn();
                ImGui::Text("Scheduled batch index");
                ImGui::TableNextColumn();
                ImGui::Text("%i", task.final_schedule_batch);

                ImGui::TableNextColumn();
                ImGui::Text("Queue");
                ImGui::TableNextColumn();
                ImVec4 const queue_family_color = queue_family_to_color(task.queue.family);
                colored_banner_text(
                    std::format("Family {} - Index {}", queue_family_to_str(task.queue.family), task.queue.index),
                    true,
                    queue_family_color,
                    ImVec4(queue_family_color.x * 0.6f, queue_family_color.y * 0.6f, queue_family_color.z * 0.6f, 1.0f));

                ImGui::EndTable();
            }

            u32 const flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;
            ImVec4 default_color = ImGui::GetStyle().Colors[ImGuiCol_Text];
            static std::array const columns = {"Name", "Type", "Access Stage", "Access", "Resource"};

            /// ===========================
            /// ======= ATTACHMENTS =======
            /// ===========================

            ImGui::SeparatorText("Attachments");
            if (ImGui::BeginTable("", static_cast<int>(columns.size()), flags))
            {
                for (auto const & column : columns)
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

                    switch (attachment_info.type)
                    {
                    case TaskAttachmentType::BUFFER:
                    case TaskAttachmentType::TLAS:
                    case TaskAttachmentType::BLAS:
                    {
                        auto const & buffer_attach_info = attachment_info.value.buffer;
                        row_info.cell_text = {std::string(buffer_attach_info.name), std::string("Buffer"), std::string(to_string(buffer_attach_info.task_access.stage)), std::string(to_string(buffer_attach_info.task_access.type))};
                        row_info.color = access_type_to_color(buffer_attach_info.task_access.type);
                        row_info.resource_index = buffer_attach_info.translated_view.index;
                        break;
                    }
                    case TaskAttachmentType::IMAGE:
                    {
                        auto const & image_attach_info = attachment_info.value.image;
                        row_info.cell_text = {std::string(image_attach_info.name), std::string("Image"), std::string(to_string(image_attach_info.task_access.stage)), std::string(to_string(image_attach_info.task_access.type))};
                        row_info.color = access_type_to_color(image_attach_info.task_access.type);
                        row_info.resource_index = image_attach_info.translated_view.index;
                        break;
                    }
                    }

                    ImGui::BeginDisabled(row_info.resource_index == ~0u);
                    if (row_info.resource_index == ~0u)
                    {
                        ImGui::PushStyleColor(ImGuiCol_TableRowBg, ImVec4(0.3f, 0.25f, 0.25f, 1.0f));
                    }
                    for (u32 cell_idx = 0; cell_idx < static_cast<u32>(row_info.cell_text.size()); ++cell_idx)
                    {
                        ImGui::TableNextColumn();
                        if (cell_idx == 0)
                        {
                            bool currently_hovered = ui_context.hovered_resource == row_info.resource_index;
                            ImGui::Selectable(row_info.cell_text.at(cell_idx).c_str(), &currently_hovered, ImGuiSelectableFlags_SpanAllColumns);
                            if (ImGui::IsItemHovered())
                            {
                                ui_context.hovered_resource = row_info.resource_index;
                            }
                            u32 const access_time_index = task.attachment_access_groups[attach_i].second;
                            resource_popup_context_ui(ui_context, impl_tg, row_info.resource_index, true, access_time_index);
                        }
                        else if (cell_idx == 3)
                        {
                            ImVec4 const circle_color = ImVec4(row_info.color.x * 0.6f + 0.1f, row_info.color.x * 0.6f + 0.1f, row_info.color.x * 0.6f + 0.1f, 1.0f);
                            colored_banner_text(row_info.cell_text.at(cell_idx).c_str(), true, row_info.color, circle_color);
                        }
                        else if (cell_idx == 4)
                        {
                            if (row_info.resource_index == ~0u)
                            {
                                ImGui::Text("NullResource");
                            }
                            else
                            {
                                ImGui::Text(impl_tg->resources[row_info.resource_index].name.data());
                            }
                        }
                        else
                        {
                            ImGui::Text(row_info.cell_text.at(cell_idx).c_str());
                        }
                    }
                    if (row_info.resource_index == ~0u)
                    {
                        ImGui::PopStyleColor();
                    }
                    ImGui::EndDisabled();
                }
                ImGui::EndTable();
            }
        }
        ImGui::End();
        return open;
    }

    auto resource_detail_ui(ImplTaskGraphDebugUi & ui_context, ImplTaskGraph * impl_tg, u32 resource_index) -> bool
    {
        ImplTaskResource const & resource = impl_tg->resources[resource_index];
        bool open = true;
        ImGui::SetNextWindowSize(ImVec2(500, 500), ImGuiCond_FirstUseEver);
        if (ImGui::Begin(resource.name.data(), &open, ImGuiWindowFlags_NoSavedSettings))
        {
            ImGui::Separator();

            /// ======================
            /// ======= HEADER =======
            /// ======================

            filter_resource_checkbox(ui_context, impl_tg, resource_index);
            ImGui::SameLine();
            pin_resource_checkbox(ui_context, impl_tg, resource_index);
            ImGui::SameLine();
            highlight_all_tasks_checkbox(ui_context, impl_tg, resource_index);
            if (resource.kind == TaskResourceKind::IMAGE || resource.kind == TaskResourceKind::BUFFER)
            {
                ImGui::SameLine();
                resource_viewer_checkbox(ui_context, impl_tg, resource_index);
            }

            /// ======================================
            /// ======= DOCKED RESOURCE VIEWER =======
            /// ======================================

            if (resource.kind == TaskResourceKind::IMAGE || resource.kind == TaskResourceKind::BUFFER)
            {
                auto const resource_name_string = std::string(resource.name);
                bool const viewer_open = ui_context.resource_viewer_states.contains(resource_name_string);
                if (viewer_open)
                {
                    ResourceViewerState & state = ui_context.resource_viewer_states.at(resource_name_string);

                    if (!state.free_window)
                    {
                        ImGui::SeparatorText("Viewer");
                        resource_viewer_ui(ui_context, impl_tg, resource_name_string, state);
                    }
                }
            }

            /// ===========================================
            /// ======= GENERAL RESOURCE ATTRIBUTES =======
            /// ===========================================

            static constexpr char const * ATTRIBUTE_FORMATTING = "- {:<28.28} ";
            ImGui::SeparatorText(std::format("{} Attributes", to_string(resource.kind).data()).c_str());

            auto set_table_cell_name_color = []()
            {
                ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::ColorConvertFloat4ToU32(ImGui::GetStyleColorVec4(ImGuiCol_TableRowBgAlt)));
            };
            
            if (resource.kind == TaskResourceKind::IMAGE && ImGui::BeginTable("Attribute Table Sizes", 11, ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit))
            {
                ImageId id = resource.external ? resource.external->id.image : resource.id.image;
                ImageInfo info = ui_context.device.image_info(id).value_or({});

                ImGui::TableNextColumn();
                set_table_cell_name_color();
                ImGui::Text("Dimensions");
                ImGui::TableNextColumn();
                ImGui::Text("%iD", info.dimensions);

                if (info.dimensions == 1)
                {
                    ImGui::TableNextColumn();
                    set_table_cell_name_color();
                    ImGui::Text("Size");
                    ImGui::TableNextColumn();
                    ImGui::Text("(%i<)", info.size.x);
                }
                if (info.dimensions == 2)
                {
                    ImGui::TableNextColumn();
                    set_table_cell_name_color();
                    ImGui::Text("Size");
                    ImGui::TableNextColumn();
                    ImGui::Text("(%i,%i)", info.size.x, info.size.y);
                }
                if (info.dimensions == 3)
                {
                    ImGui::TableNextColumn();
                    set_table_cell_name_color();
                    ImGui::Text("Size");
                    ImGui::TableNextColumn();
                    ImGui::Text("(%i,%i,%i)", info.size.x, info.size.y, info.size.z);
                }

                ImGui::TableNextColumn();
                set_table_cell_name_color();
                ImGui::Text("Array Layers");
                ImGui::TableNextColumn();
                ImGui::Text("%i", info.array_layer_count);

                ImGui::TableNextColumn();
                set_table_cell_name_color();
                ImGui::Text("Mip Levels");
                ImGui::TableNextColumn();
                ImGui::Text("%i", info.mip_level_count);

                ImGui::TableNextColumn();
                set_table_cell_name_color();
                ImGui::Text("Sample Count");
                ImGui::TableNextColumn();
                ImGui::Text("%i", info.sample_count);

                ImGui::TableNextColumn();
                ImGui::EndTable();
            }

            if (resource.allocation_size != 0 && ImGui::BeginTable("Attribute Table Transient Memory", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit))
            {
                ImGui::TableNextColumn();
                set_table_cell_name_color();
                ImGui::Text("Transient Memory Offset");
                ImGui::TableNextColumn();
                ImGui::Text("%i", resource.allocation_offset);

                ImGui::TableNextColumn();
                set_table_cell_name_color();
                ImGui::Text("Transient Memory Size");
                ImGui::TableNextColumn();
                ImGui::Text("%i", resource.allocation_size);

                ImGui::TableNextColumn();
                ImGui::EndTable();
            }

            if (ImGui::BeginTable("Attribute Table", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit))
            {
                ImGui::TableSetupColumn("Attribute");
                ImGui::TableSetupColumn("Value");

                if (resource.external)
                {
                    ImGui::TableNextColumn();
                    set_table_cell_name_color();
                    ImGui::Text("External Resource Link");
                    ImGui::TableNextColumn();
                    ImGui::Text(resource.external->name.data());
                }

                switch (resource.kind)
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
                    switch (resource.kind)
                    {
                    case TaskResourceKind::BUFFER:
                    {
                        BufferId id = resource.external ? resource.external->id.buffer : resource.id.buffer;
                        BufferInfo info = ui_context.device.buffer_info(id).value_or({});
                        size = info.size;
                        device_address = ui_context.device.buffer_device_address(id).value_or(0);
                        host_address = std::bit_cast<u64>(ui_context.device.buffer_host_address(id).value_or(0));
                        external_resource_name = info.name;
                        memory_flags = info.allocate_info;
                        break;
                    }
                    case TaskResourceKind::BLAS:
                    {
                        BlasId id = resource.external ? resource.external->id.blas : resource.id.blas;
                        BlasInfo info = ui_context.device.blas_info(id).value_or({});
                        size = info.size;
                        device_address = ui_context.device.blas_device_address(id).value_or(0);
                        external_resource_name = info.name;
                        memory_flags = MemoryFlagBits::NONE;
                        host_address = 0;
                        break;
                    }
                    case TaskResourceKind::TLAS:
                    {
                        TlasId id = resource.external ? resource.external->id.tlas : resource.id.tlas;
                        TlasInfo info = ui_context.device.tlas_info(id).value_or({});
                        size = info.size;
                        device_address = ui_context.device.tlas_device_address(id).value_or(0);
                        external_resource_name = info.name;
                        memory_flags = MemoryFlagBits::NONE;
                        host_address = 0;
                        break;
                    }
                    }

                    ImGui::TableNextColumn();
                    set_table_cell_name_color();
                    ImGui::Text("MemoryFlags");
                    ImGui::TableNextColumn();
                    ImGui::Text(to_string(memory_flags).data());

                    ImGui::TableNextColumn();
                    set_table_cell_name_color();
                    ImGui::Text("Device Address");
                    ImGui::TableNextColumn();
                    ImGui::Text(std::to_string(device_address).c_str());

                    if (host_address)
                    {
                        ImGui::TableNextColumn();
                        set_table_cell_name_color();
                        ImGui::Text("Host Address");
                        ImGui::TableNextColumn();
                        ImGui::Text(std::to_string(host_address).c_str());
                    }

                    break;
                }
                case TaskResourceKind::IMAGE:
                {
                    ImageId id = resource.external ? resource.external->id.image : resource.id.image;
                    ImageInfo info = ui_context.device.image_info(id).value_or({});

                    ImGui::TableNextColumn();
                    set_table_cell_name_color();
                    ImGui::Text("Format");
                    ImGui::TableNextColumn();
                    ImGui::Text(to_string(info.format).data());

                    if (info.flags != ImageCreateFlagBits::NONE)
                    {
                        ImGui::TableNextColumn();
                        set_table_cell_name_color();
                        ImGui::Text("Flags");
                        ImGui::TableNextColumn();
                        ImGui::Text(to_string(info.flags).data());
                    }

                    ImGui::TableNextColumn();
                    set_table_cell_name_color();
                    ImGui::Text("Usage Flags");
                    ImGui::TableNextColumn();
                    ImGui::Text(to_string(info.usage).data());

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
                    ImGui::TableNextColumn();
                    set_table_cell_name_color();
                    ImGui::Text("Used In Stages");
                    ImGui::TableNextColumn();
                    // ImGui::Text("%s", std::format(ATTRIBUTE_FORMATTING, "Used in Pipeline Stages:").c_str());
                    u64 stage_iter = pipeline_stage_bits;
                    while (stage_iter)
                    {
                        u64 index = 63ull - static_cast<u64>(std::countl_zero(stage_iter));
                        stage_iter &= ~(1ull << index);

                        PipelineStageFlags stage = std::bit_cast<PipelineStageFlags>(1ull << index);
                        auto str = to_string(stage);
                        if (str.size() > 0)
                        {
                            ImGui::Text("%s", str.c_str());
                        }

                        if (stage_iter)
                        {
                            ImGui::SameLine();
                            ImGui::Text("|");
                            ImGui::SameLine();
                        }
                    }
                }

                /// ==============================
                /// ======= USED IN QUEUES =======
                /// ==============================
                {
                    ImGui::TableNextColumn();
                    set_table_cell_name_color();
                    ImGui::Text("Used In Queues");
                    ImGui::TableNextColumn();
                    u32 iter = resource.queue_bits;
                    while (iter)
                    {
                        u32 index = queue_bits_to_first_queue_index(iter);
                        iter &= ~(1ull << index);

                        Queue queue = queue_index_to_queue(index);
                        ImGui::Text("%s", to_string(queue).data());

                        if (iter)
                        {
                            ImGui::SameLine();
                            ImGui::Text("|");
                            ImGui::SameLine();
                        }
                    }
                }
                ImGui::EndTable();
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
                        ImGui::Text(std::format("Barrier. Src: {} Dst: {}", to_string(ag.final_schedule_pre_barrier->src_access), to_string(ag.final_schedule_pre_barrier->dst_access)).c_str());
                        ImGui::EndTable();
                    }
                }
                if (ImGui::BeginTable("Access Group Table", 4, ACCESS_GROUP_TABLE_FLAGS))
                {
                    for (u32 t = 0; t < ag.tasks.size(); ++t)
                    {
                        u32 task_index = ag.tasks[t].task_index;
                        ImplTask * task = ag.tasks[t].task;
                        u32 attach_i = ui_context.task_resource_to_attachment_lookup[resource_index][task_index];
                        ImGui::TableNextColumn();
                        {
                            ImGui::PushStyleColor(ImGuiCol_Header, ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 1.0f, 1.0f, 0.2f)));
                            bool selected = ui_context.hovered_task == task_index;
                            ImGui::Selectable(task->name.data(), &selected, ImGuiSelectableFlags_SpanAllColumns);
                            task_popup_context_ui(ui_context, impl_tg, task_index, attach_i);
                            ImGui::PopStyleColor();
                        }
                        ImGui::TableNextColumn();
                        ImGui::Text("%i", task->final_schedule_batch);
                        ImGui::TableNextColumn();
                        ImVec4 color = access_type_to_color(resource.access_timeline[agi].type);
                        colored_banner_text(
                            to_string(task->attachments[attach_i].value.buffer.task_access),
                            true,
                            color,
                            ImVec4(color.x * 0.6f, color.y * 0.6f, color.z * 0.6f, 1.0f));
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
        ImGui::ColorButton("NONE", access_type_to_color(TaskAccessType::NONE));
        ImGui::SameLine();
        ImGui::Text("NONE");
        ImGui::ColorButton("WRITE", access_type_to_color(TaskAccessType::WRITE));
        ImGui::SameLine();
        ImGui::Text("WRITE");
        ImGui::ColorButton("READ", access_type_to_color(TaskAccessType::READ));
        ImGui::SameLine();
        ImGui::Text("READ");
        ImGui::ColorButton("READ_WRITE", access_type_to_color(TaskAccessType::READ_WRITE));
        ImGui::SameLine();
        ImGui::Text("READ_WRITE");
        ImGui::ColorButton("WRITE_CONCURRENT", access_type_to_color(TaskAccessType::WRITE_CONCURRENT));
        ImGui::SameLine();
        ImGui::Text("WRITE_CONCURRENT");
        ImGui::ColorButton("READ_WRITE_CONCURRENT", access_type_to_color(TaskAccessType::READ_WRITE_CONCURRENT));
        ImGui::SameLine();
        ImGui::Text("READ_WRITE_CONCURRENT");
        ImGui::ColorButton("IMAGE LAYOUT INIT", LAYOUT_INITIALIZATION_COLOR);
        ImGui::SameLine();
        ImGui::Text("IMAGE LAYOUT INIT");
        ImGui::ColorButton("BARRIER", BARRIER_COLOR);
        ImGui::SameLine();
        ImGui::Text("BARRIER");
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

    void task_timeline_ui(ImplTaskGraphDebugUi & ui_context, ImplTaskGraph * impl_tg)
    {
        /// =========================
        /// ======= HEADER UI =======
        /// =========================

        if (ImGui::Button("X"))
        {
            ui_context.resource_name_search = {};
        }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(128);
        ImGui::InputText("Filter Resource", ui_context.resource_name_search.data(), ui_context.resource_name_search.size());
        ImGui::SameLine();
        if (ImGui::Button("X##1"))
        {
            ui_context.task_name_search = {};
        }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(128);
        ImGui::InputText("Filter Task", ui_context.task_name_search.data(), ui_context.task_name_search.size());

        ImGui::SameLine();
        ImGui::Button("Filter Options");
        if (ImGui::BeginPopupContextItem("Filter", ImGuiPopupFlags_MouseButtonLeft))
        {
            // Buttons:
            {
                ImGui::PushStyleColor(ImGuiCol_Button, ColorPalette::GREY);
                
                if (ImGui::Button(" ##Un-Highlight all Tasks"))
                {
                    ui_context.extra_highlighted_tasks.clear();
                }
                ImGui::SameLine();
                ImGui::Text("Un-Highlight all Tasks");

                if (ImGui::Button(" ##Un-Pin all Resources"))
                {
                    ui_context.pinned_resources.clear();
                }
                ImGui::SameLine();
                ImGui::Text("Un-Pin all Resources");

                if (ImGui::Button(" ##UClose all Task Uis"))
                {
                    ui_context.open_task_detail_windows.clear();
                }
                ImGui::SameLine();
                ImGui::Text("Close all Task Uis");

                if (ImGui::Button(" ##UClose all Resource Uis"))
                {
                    ui_context.open_resource_detail_windows.clear();
                }
                ImGui::SameLine();
                ImGui::Text("Close all Resource Uis");

                ImGui::PopStyleColor(1);
            }

            ImGui::Checkbox("Show Batch Borders", &ui_context.show_batch_borders);
            ImGui::Checkbox("Show Transfer Tasks", &ui_context.show_transfer_tasks);
            ImGui::Checkbox("Show Async Queues", &ui_context.show_async_queues);
            ImGui::EndPopup();
        }
        ImGui::SameLine();
        ImGui::Button("Statistics");
        if (ImGui::BeginPopupContextItem("Statistics", ImGuiPopupFlags_MouseButtonLeft))
        {
            if (ImGui::BeginTable("Statistics Table", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedSame | ImGuiTableFlags_RowBg))
            {
                ImGui::TableSetupColumn("Statistic");
                ImGui::TableSetupColumn("Value");

                ImGui::TableNextColumn();
                ImGui::Text("Total Tasks");
                ImGui::TableNextColumn();
                ImGui::Text("%i", impl_tg->tasks.size());

                ImGui::TableNextColumn();
                ImGui::Text("Total Resources");
                ImGui::TableNextColumn();
                ImGui::Text("%i", impl_tg->resources.size());

                ImGui::TableNextColumn();
                ImGui::Text("Total Batches");
                ImGui::TableNextColumn();
                ImGui::Text("%i", impl_tg->flat_batch_count);

                ImGui::TableNextColumn();
                ImGui::Text("Transient Memory");
                ImGui::TableNextColumn();
                ImGui::Text("%imb", impl_tg->transient_memory_block.info().requirements.size / (1u << 20u));

                ImGui::EndTable();
            }
            ImGui::EndPopup();
        }

        /// ===========================================
        /// ======= TABLE AND INDIRECTION SETUP =======
        /// ===========================================

        static ImGuiTableFlags table_flags = ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY;
        static ImGuiTableColumnFlags column_flags = ImGuiTableColumnFlags_AngledHeader | ImGuiTableColumnFlags_WidthFixed;

        float const TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();

        if (ui_context.recreated)
        {
            // find the resource indices based on names
            for (auto pr = ui_context.pinned_resources.begin(); pr != ui_context.pinned_resources.end(); ++pr)
            {
                if (impl_tg->name_to_resource_table.contains(pr->first.c_str()))
                {
                    pr->second = impl_tg->name_to_resource_table[pr->first.c_str()].second;
                }
                else
                {
                    pr = ui_context.pinned_resources.erase(pr);
                }
            }
        }

        // Construct Row Ui Data
        std::vector<RowUiData> row_ui_data = {};
        std::vector<u32> resource_to_row_lookup = std::vector<u32>(impl_tg->resources.size(), ~0u);
        for (auto const & [name, resource_index] : ui_context.pinned_resources)
        {
            row_ui_data.push_back({
                .global_resource_index = resource_index,
                .resource = &impl_tg->resources[resource_index],
            });
            resource_to_row_lookup[resource_index] = static_cast<u32>(row_ui_data.size() - 1);
        }
        for (u32 r = 0; r < impl_tg->resources.size(); ++r)
        {
            ImplTaskResource & resource = impl_tg->resources[r];

            std::string resource_name = std::string(resource.name);
            bool const search_substring_match =
                strstr(to_lower(std::string(resource.name)).c_str(), to_lower(std::string(ui_context.resource_name_search.data())).c_str()) != nullptr;
            if (!ui_context.pinned_resources.contains(std::string(resource.name)) && search_substring_match)
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

        std::vector<char const *> col_names = {}; // get rid of this
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
        for (u32 submit_index = 0; submit_index < impl_tg->submits.size(); ++submit_index)
        {
            TasksSubmit const & submit = impl_tg->submits[submit_index];

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

                if (!ui_context.show_async_queues && queue_index != 0)
                {
                    continue;
                }

                // Only show queue border when showing multiple queues
                if (ui_context.show_async_queues)
                {
                    col_ui_data.push_back({
                        .submit_index = submit_index,
                        .queue_index = queue_index,
                        .local_batch_index = ~0u,
                        .is_queue_submit_border = true,
                    });
                    col_names.push_back("");
                }

                std::vector<BatchUiData> batch_ui_data = {};

                for (u32 batch_i = 0; batch_i < queue_batches.size(); ++batch_i)
                {
                    u32 const global_batch_index = submit.final_schedule_first_batch + batch_i;
                    batch_count = std::max(batch_count, global_batch_index + 1);

                    if (ui_context.show_batch_borders)
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

                        if (!ui_context.show_transfer_tasks && task_pair.first->task_type == TaskType::TRANSFER)
                        {
                            continue;
                        }

                        bool const search_substring_match =
                            strstr(to_lower(std::string(task_pair.first->name)).c_str(), to_lower(std::string(ui_context.task_name_search.data())).c_str()) != nullptr;

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

        if (ui_context.recreated)
        {
            ui_context.task_resource_to_attachment_lookup.clear();
            ui_context.task_resource_to_attachment_lookup.resize(impl_tg->resources.size(), std::vector<u32>(impl_tg->tasks.size(), ~0u)); // first row index, second column index

            // go over all columns (tasks) and write conversion attachment indices when applicable
            for (u32 t = 0; t < impl_tg->tasks.size(); ++t)
            {
                ImplTask const & task = impl_tg->tasks[t];
                for (u32 attach_i = 0; attach_i < task.attachments.size(); ++attach_i)
                {
                    bool const null_attachment = task.attachment_resources[attach_i].first == nullptr;
                    if (null_attachment)
                    {
                        continue;
                    }
                    u32 const global_resource_index = task.attachment_resources[attach_i].second;
                    ui_context.task_resource_to_attachment_lookup[global_resource_index][t] = attach_i;
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
        ImVec2 const scale = ImVec2(1.0f, 0.0f);

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

            ImGui::TableSetupScrollFreeze(1, static_cast<u32>(2 + ui_context.pinned_resources.size()));
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
                    bool highlighted = ui_context.extra_highlighted_tasks.contains(std::string(col_ui_data[n].task->name));
                    if (highlighted)
                    {
                        background_color.x = 0.9f;
                        background_color.y = 0.9f;
                        background_color.z = 0.9f;
                        text_color = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
                    }
                    name = std::format(TIMELINE_TASK_HEADER_STR_FORMAT, col_ui_data[n].task->name);
                    task_popup_context_ui(ui_context, impl_tg, col_ui_data[n].global_task_index);
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
                    batch_border_cell ? batch_border_cell_size : TEXT_BASE_HEIGHT * scale.x);
            }

            ImGui::TableAngledHeadersRow2(colors.data(), text_colors.data()); // Draw angled headers for all columns with the ImGuiTableColumnFlags_AngledHeader flag.
            ImGui::TableNextRow(ImGuiTableRowFlags_Headers);

            /// ======================================
            /// ======= FIRST ROW IS TASK DATA =======
            /// ======================================

            ImGui::TableSetColumnIndex(0);
            u32 first_visible_column = ~0u;
            u32 last_visible_column = 0;
            for (int column = 1; column < static_cast<i32>(col_count); column++)
            {
                if(!ImGui::TableSetColumnIndex(column))
                {
                    if(first_visible_column != ~0u)
                    {
                        // We have exited the visible columns, break to save performance.
                        break;
                    }
                    continue;
                }
                last_visible_column = std::max(last_visible_column, static_cast<u32>(column));
                first_visible_column = std::min(first_visible_column, static_cast<u32>(column));

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

                        bool highlighted = ui_context.extra_highlighted_tasks.contains(std::string(col_ui_data[column].task->name));
                        if (highlighted)
                        {
                            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::ColorConvertFloat4ToU32(ImVec4(0.9f, 0.9f, 0.9f, 1.0f)));
                            color = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
                        }
                    }

                    ImGui::PushID(column);
                    ImGui::PushStyleColor(ImGuiCol_Text, color);
                    float const text_width = ImGui::CalcTextSize("XX").x;
                    float const column_width = ImGui::GetColumnWidth();
                    float const text_offset = (column_width - text_width) / 2;
                    ImGui::Dummy(ImVec2(0.0f, 0.0f));
                    ImGui::SameLine(text_offset);
                    ImGui::Text(name);
                    ImGui::PopStyleColor();
                    if (!col_ui.is_queue_submit_border && !col_ui.is_submit_border)
                    {
                        task_popup_context_ui(ui_context, impl_tg, col_ui.global_task_index);
                    }
                    ImGui::PopID();
                }
            }

            /// ===================================
            /// ======= ROW BY ROW TABLE UI =======
            /// ===================================

            ImGuiListClipper clipper = {};
            clipper.Begin(row_count);
            while(clipper.Step())
            {
                for (u32 row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row)
                {
                    RowUiData const & row_ui = row_ui_data[row];

                    /// ========================================
                    /// ======= SETUP BATCH DATA FOR ROW =======
                    /// ========================================

                    u32 resource_index = row_ui_data[row].global_resource_index;
                    ImplTaskResource const & resource = impl_tg->resources[resource_index];

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

                    bool const is_pinned = ui_context.pinned_resources.contains(std::string(resource.name));
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
                                ui_context.pinned_resources.clear();
                            }
                            else
                            {
                                ui_context.pinned_resources.erase(std::string(resource.name));
                            }
                        }
                        else
                        {
                            ui_context.pinned_resources[std::string(resource.name)] = resource_index;
                        }
                    }
                    ImGui::SameLine();
                    if (is_pinned)
                    {
                        ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::ColorConvertFloat4ToU32(ImVec4(0.9, 0.9, 0.9, 1)));
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1));
                    }
                    ImGui::Text(resource.name.data());
                    if (is_pinned)
                    {
                        ImGui::PopStyleColor();
                    }
                    ImGui::SetItemTooltip(resource.name.data());
                    resource_popup_context_ui(ui_context, impl_tg, resource_index, true);

                    /// =========================================
                    /// ======= COLUMN BY COLUMN TABLE UI =======
                    /// =========================================

                    for (u32 column = first_visible_column; column < last_visible_column + 1; ++column)
                    {
                        if (ImGui::TableSetColumnIndex(column))
                        {
                            ColUiData const & col_ui = col_ui_data[column]; // -1 as the first column is for the name.
                            bool const is_batch_border_cell = col_ui.task == nullptr;

                            u32 const global_batch_index = impl_tg->submits[col_ui.submit_index].final_schedule_first_batch + col_ui.local_batch_index;

                            bool make_cell_darker = true;
                            ImVec4 cell_color = ImVec4(0.05f, 0.05f, 0.05f, 1.0f);
                            if (col_ui.is_submit_border)
                            {
                                cell_color = SUBMIT_COLOR;
                                make_cell_darker = false;
                            }
                            else if (col_ui.is_queue_submit_border)
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
                                        batch_ui.access_group->final_schedule_first_batch == global_batch_index)
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
                                    else
                                    {
                                        cell_color.x *= 0.4f + 0.8f;
                                        cell_color.y *= 0.4f + 0.8f;
                                        cell_color.z *= 0.4f + 0.8f;
                                    }
                                }
                                else // Give attachment info in Cells
                                {
                                    u32 attachment_index = ui_context.task_resource_to_attachment_lookup[resource_index][col_ui.task_index];
                                    bool const resource_accessed_by_task = attachment_index != ~0u;
                                    if (resource_accessed_by_task)
                                    {
                                        ImGui::PushID(column + row * 60000);
                                        ImGui::Selectable("");

                                        /// ===========================
                                        /// ======= HOVER LOGIC =======
                                        /// ===========================

                                        bool const hovered_previously =
                                            ui_context.hovered_task == col_ui.task_index &&
                                            ui_context.hovered_attachment_index == attachment_index &&
                                            ui_context.hovered_resource == resource_index;

                                        bool const hovered_now = ImGui::IsItemHovered();
                                        if (hovered_now)
                                        {
                                            ui_context.hovered_task = col_ui.task_index;
                                            ui_context.hovered_attachment_index = attachment_index;
                                            ui_context.hovered_resource = resource_index;
                                        }
                                        if (!hovered_now && hovered_previously)
                                        {
                                            ui_context.hovered_task = ~0u;
                                            ui_context.hovered_attachment_index = ~0u;
                                            ui_context.hovered_resource = ~0u;
                                        }

                                        /// ===============================
                                        /// ======= CELL POPUP MENU =======
                                        /// ===============================

                                        u32 const access_timeline_index = col_ui.task->attachment_access_groups[attachment_index].second;
                                        resource_popup_context_ui(ui_context, impl_tg, resource_index, true, access_timeline_index);
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

                            bool const is_highlighted =
                                (col_ui.task && ui_context.extra_highlighted_tasks.contains(std::string(col_ui.task->name))) ||
                                (col_ui.task && ui_context.hovered_task == col_ui.task_index) ||
                                ui_context.hovered_resource == resource_index;

                            if (is_highlighted)
                            {
                                cell_color.x = cell_color.x * 0.4f + 0.6f;
                                cell_color.y = cell_color.y * 0.4f + 0.6f;
                                cell_color.z = cell_color.z * 0.4f + 0.6f;
                            }

                            if (make_cell_darker)
                            {
                                cell_color.x = cell_color.x * 0.4f + 0.07f;
                                cell_color.y = cell_color.y * 0.4f + 0.07f;
                                cell_color.z = cell_color.z * 0.4f + 0.07f;
                            }

                            u32 const color = ImGui::ColorConvertFloat4ToU32(cell_color);

                            ImGui::PushID(column);
                            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, color);
                            ImGui::PopID();
                        }
                    }
                    ImGui::PopID();
                }
            }
            ImGui::EndTable();
        }
        ImGui::PopStyleVar();

        ImVec2 cursor_after_table = ImGui::GetCursorPos();
        ImGui::SetCursorPos(cursor_before_table);
        timeline_legend();
        ImGui::SetCursorPos(cursor_after_table);
    }

    TaskGraphDebugUi::TaskGraphDebugUi(TaskGraphDebugUiInfo const & info)
    {
        this->object = new ImplTaskGraphDebugUi{};
        auto & ui_context = *r_cast<ImplTaskGraphDebugUi *>(this->object);
        ui_context.imgui_renderer = info.imgui_renderer;
        ui_context.device = info.device;
        ui_context.buffer_layout_cache_folder = info.buffer_layout_cache_folder;

        if(ui_context.buffer_layout_cache_folder.has_value())
        {
            if(!std::filesystem::exists(ui_context.buffer_layout_cache_folder.value()))
            {
                std::filesystem::create_directories(ui_context.buffer_layout_cache_folder.value());
            }
        }

        DAXA_DBG_ASSERT_TRUE_M(ui_context.resource_viewer_sampler.is_empty(), "IMPOSSIBLE CASE! The UI cannot already ahve a sampler created");
        {
            ui_context.resource_viewer_sampler = ui_context.device.create_sampler({
                .magnification_filter = Filter::NEAREST,
                .minification_filter = Filter::NEAREST,
            });
        }

#if TASK_GRAPH_RESOURCE_VIEWER_ONLINE_COMPILE_SHADERS

        /// ====================================================
        /// ======= DEV LIVE RESOURCE VIEWER COMPILATION =======
        /// ====================================================

        DAXA_DBG_ASSERT_TRUE_M(!ui_context.pipeline_manager.is_valid(), "IMPOSSIBLE CASE! The UI cannot already have a pipeline manager");
        if (!ui_context.pipeline_manager.is_valid())
        {
            ui_context.pipeline_manager = daxa::PipelineManager(daxa::PipelineManagerInfo2{
                .device = info.device,
                .root_paths = {
                    DAXA_SHADER_INCLUDE_DIR,
                },
                .write_out_spirv = std::nullopt,
                .default_language = daxa::ShaderLanguage::SLANG,
                .default_enable_debug_info = false,
                .name = "tg_attach_viewer_pipeline_manager",
            });

            auto compilation_result = ui_context.pipeline_manager.add_compute_pipeline2({
                .source = daxa::ShaderFile{"../src/utils/impl_resource_viewer.slang"},
                .entry_point = "main",
                .push_constant_size = sizeof(TaskGraphDebugUiPush),
                .name = "impl_resource_viewer.slang",
            });
            if (compilation_result.value()->is_valid())
            {
                std::cout << "[TaskGraphDebugUi] SUCCESSFULLY compiled image resource viewer pipeline" << std::endl;
            }
            else
            {
                std::cout << std::format("[TaskGraphDebugUi] FAILED to compile image resource viewer pipeline with message \n {}", compilation_result.message()) << std::endl;
            }
            ui_context.resource_viewer_pipeline = compilation_result.value();
        }

        {
            auto reloaded_result = ui_context.pipeline_manager.reload_all();
            if (auto reload_err = daxa::get_if<daxa::PipelineReloadError>(&reloaded_result))
            {
                std::cout << "Failed to reload " << reload_err->message << '\n';
            }
            if (auto _ = daxa::get_if<daxa::PipelineReloadSuccess>(&reloaded_result))
            {
                std::cout << "Successfully reloaded!\n";
            }
        }
#else // #if TASK_GRAPH_RESOURCE_VIEWER_ONLINE_COMPILE_SHADERS

        ui_context.resource_viewer_pipeline = std::make_shared<ComputePipeline>(ui_context.device.create_compute_pipeline({
            .shader_info = daxa::ShaderInfo({
                .byte_code = image_impl_resource_viewer_spv.data(),
                .byte_code_size = static_cast<u32>(image_impl_resource_viewer_spv.size()),
                .create_flags = {},
                .required_subgroup_size = {},
                .entry_point = "main",
            }),
            .push_constant_size = sizeof(TaskGraphDebugUiPush),
            .name = "task_graph_image_resource_viewer",
        }));

#endif
    }

    ImplTaskGraphDebugUi::~ImplTaskGraphDebugUi()
    {
        this->device.destroy_sampler(this->resource_viewer_sampler);

        for (auto av_iter = this->resource_viewer_states.begin(); av_iter != this->resource_viewer_states.end();)
        {
            av_iter = destroy_resource_viewer(*this, av_iter->first);
        }
    }

    auto TaskGraphDebugUi::inc_refcnt(ImplHandle const * object) -> u64
    {
        return object->inc_refcnt();
    }

    auto TaskGraphDebugUi::dec_refcnt(ImplHandle const * object) -> u64
    {
        return object->dec_refcnt(
            ImplTaskGraphDebugUi::zero_ref_callback,
            nullptr);
    }

    void ImplTaskGraphDebugUi::zero_ref_callback(ImplHandle const * handle)
    {
        auto const * self = r_cast<ImplTaskGraphDebugUi const *>(handle);
        delete self;
    }

    auto TaskGraphDebugUi::update(TaskGraph & task_graph) -> bool
    {
        auto & ui_context = *r_cast<ImplTaskGraphDebugUi *>(this->object);
        auto * impl_tg = r_cast<ImplTaskGraph *>(task_graph.get());

        ui_context.recreated = ui_context.last_exec_tg_unique_index != impl_tg->unique_index;
        ui_context.last_exec_tg_unique_index = impl_tg->unique_index;

        /// ==============================================
        /// ======= PRE UPDATE RESOURCE VIEW STATE =======
        /// ==============================================

        for (auto av_iter = ui_context.resource_viewer_states.begin(); av_iter != ui_context.resource_viewer_states.end();)
        {
            ResourceViewerState & state = av_iter->second;
            state.image.imgui_image_id = {};
            state.free_window = av_iter->second.free_window_next_frame;

            state.open_frame_count += 1;
            u32 const readback_index = (state.open_frame_count) % READBACK_CIRCULAR_BUFFER_SIZE;

            if (!state.image.readback_buffer.is_empty())
            {
                auto * readback_struct = ui_context.device.buffer_host_address_as<TaskGraphDebugUiImageReadbackStruct>(state.image.readback_buffer).value();
                state.image.latest_readback = readback_struct[readback_index];
                readback_struct[readback_index] = {
                    .pos_min_value = ~0u,
                    .neg_min_value = ~0u,
                };
            }

            if (!state.image.destroy_display_image.is_empty())
            {
                ui_context.device.destroy_image(state.image.destroy_display_image);
                state.image.destroy_display_image = {};
            }
            if (!state.image.destroy_clone_image.is_empty())
            {
                ui_context.device.destroy_image(state.image.destroy_clone_image);
                state.image.destroy_clone_image = {};
            }
            ++av_iter;
        }

        /// =======================================
        /// ======= TASK GRAPH MAIN UI TABS =======
        /// =======================================

        bool open = true;
        if (ImGui::Begin("Task Graph Debug UI", &open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollWithMouse))
        {
            ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
            if (ImGui::BeginTabBar("TgDebugUiTabBar", tab_bar_flags))
            {
                if (ImGui::BeginTabItem("Timeline"))
                {
                    task_timeline_ui(ui_context, impl_tg);
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Memory"))
                {
                    memory_ui(ui_context, impl_tg);
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
        }
        ImGui::End();
        if (!open)
        {
            return open;
        }

        /// ==================================
        /// ======= RESOURCE VIEWER UI =======
        /// ==================================

        for (auto av_iter = ui_context.resource_viewer_states.begin(); av_iter != ui_context.resource_viewer_states.end();)
        {
            if (av_iter->second.free_window)
            {
                if (!resource_viewer_ui(ui_context, impl_tg, av_iter->first, av_iter->second))
                {
                    av_iter = destroy_resource_viewer(ui_context, av_iter->first);
                    continue;
                }
            }

            ++av_iter;
        }

        // broadcasting
        for (auto av_iter = ui_context.resource_viewer_states.begin(); av_iter != ui_context.resource_viewer_states.end();)
        {
            if (ui_context.broadcast_resource_viewer_freeze.has_value())
            {
                av_iter->second.freeze_resource = ui_context.broadcast_resource_viewer_freeze.value();
            }

            ++av_iter;
        }
        ui_context.boardcast_image_viewer_limits_prev_frame = ui_context.boardcast_image_viewer_limits;
        ui_context.boardcast_image_viewer_limits = {};
        ui_context.broadcast_resource_viewer_freeze = {};

        /// ==============================================
        /// ======= DETAIL RESOURCE DETAIL TASK UI =======
        /// ==============================================

        for (auto res_detail_iter = ui_context.open_resource_detail_windows.begin(); res_detail_iter != ui_context.open_resource_detail_windows.end();)
        {
            if (!impl_tg->name_to_resource_table.contains(*res_detail_iter))
            {
                res_detail_iter = ui_context.open_resource_detail_windows.erase(res_detail_iter);
                continue;
            }

            if (!resource_detail_ui(ui_context, impl_tg, impl_tg->name_to_resource_table[*res_detail_iter].second))
            {
                res_detail_iter = ui_context.open_resource_detail_windows.erase(res_detail_iter);
                continue;
            }

            ++res_detail_iter;
        }

        for (auto task_detail_iter = ui_context.open_task_detail_windows.begin(); task_detail_iter != ui_context.open_task_detail_windows.end();)
        {
            if (!task_details_ui(ui_context, impl_tg, std::get<1>(impl_tg->name_to_task_table[std::string_view{*task_detail_iter}])))
            {
                task_detail_iter = ui_context.open_task_detail_windows.erase(task_detail_iter);
                continue;
            }

            ++task_detail_iter;
        }

        return open;
    }
} // namespace daxa

#endif // #if DAXA_BUILT_WITH_UTILS_IMGUI && DAXA_ENABLE_TASK_GRAPH_MK2
#endif