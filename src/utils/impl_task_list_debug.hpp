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

    void print_seperator_to(std::string & out, std::string & indent)
    {
        char const last = indent.back();
        indent.back() = '#';
        std::format_to(std::back_inserter(out), "{}--------------------------------\n", indent);
        indent.back() = last;
    }

    struct FormatIndent
    {
        std::string & indent;
        FormatIndent(std::string & out, std::string & a_indent, bool contained = false) : indent{ a_indent }
        {
            indent.push_back(' ');
            indent.push_back(' ');
            indent.push_back(' ');
            indent.push_back(contained ? '|' : ' ');
            if (contained)
            {
                print_seperator_to(out, a_indent);
            }
        }
        ~FormatIndent()
        {
            indent.pop_back();
            indent.pop_back();
            indent.pop_back();
            indent.pop_back();
        }
    };


}