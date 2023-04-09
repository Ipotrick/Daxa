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

    void print_task_image(std::ostream & os, ImplTaskList const & impl, TaskResourceIndex image, Level level)
    {
        // BASIC - name and usage
        // DETAILED - above and mip levels, array layers, acesses
        // VERBOSE - above and ids and layous
        std::string ret = std::format("test {}\n", "3");
    }
}