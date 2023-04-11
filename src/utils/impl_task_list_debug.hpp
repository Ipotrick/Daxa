#pragma once

#include "impl_task_list.hpp"
#include <format>
namespace daxa
{
    enum struct Level
    {
        BASIC,
        DETAILED,
        VERBOSE
    };

    struct FormatIndent
    {
        std::string & indent;
        FormatIndent(std::string & a_indent) : indent{ a_indent } 
        {
            indent.push_back(' ');
            indent.push_back(' ');
            indent.push_back(' ');
            indent.push_back(' ');
        }
        ~FormatIndent()
        {
            indent.pop_back();
            indent.pop_back();
            indent.pop_back();
            indent.pop_back();
        }
    };

    void print_task_image(std::string & out, std::string & indent, TaskListPermutation const & permutation, ImplTaskList const & impl, TaskResourceIndex image)
    {
        std::string_view const & name = impl.global_image_infos[image].get_name();
        std::string persistent_info = "";
        if (impl.global_image_infos[image].is_persistent())
        {
            u32 const persistent_index = impl.global_image_infos[image].get_persistent().unique_index;
            persistent_info = std::format("persistent index: {}, ", persistent_index);
        }
        std::format_to(
            std::back_inserter(out), 
            "{}TaskImage: id: {}, name: {}, {}runtime images:\n", 
            indent, 
            to_string(TaskImageId{{.task_list_index = impl.unique_index, .index=image}}),
            name,
            persistent_info
        );
        TaskImageId const local_id = TaskImageId{{.task_list_index = impl.unique_index, .index = image }};
        {
            FormatIndent d0{indent};
            for (u32 child_i = 0; child_i < impl.get_actual_images(local_id, permutation).size(); ++child_i)
            {
                auto const child_id = impl.get_actual_images(local_id, permutation)[child_i];
                auto const & child_info = impl.info.device.info_image(child_id);
                std::format_to(
                    std::back_inserter(out), 
                    "{}id: {}, name: {}\n",
                    indent,
                    to_string(child_id),
                    child_info.name
                );
            }
        }
    }
}