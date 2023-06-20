#pragma once

#include "impl_task_list.hpp"
#include <format>
#include <fmt/format.h>

namespace daxa
{
    enum struct Level
    {
        BASIC,
        DETAILED,
        VERBOSE,
        MAX_ENUM = 0x7fffffff,
    };

    void print_seperator_to(std::string & out, std::string & indent)
    {
        char const last = indent.back();
        indent.back() = '#';
        fmt::format_to(std::back_inserter(out), "{}--------------------------------\n", indent);
        indent.back() = last;
    }

    void begin_indent(std::string & out, std::string & indent, bool contained = false)
    {
        indent.push_back(' ');
        indent.push_back(' ');
        indent.push_back(' ');
        indent.push_back(contained ? '|' : ' ');
        if (contained)
        {
            print_seperator_to(out, indent);
        }
    }

    void end_indent(std::string &, std::string & indent)
    {
        indent.pop_back();
        indent.pop_back();
        indent.pop_back();
        indent.pop_back();
    }

    struct FormatIndent
    {
        std::string & out;
        std::string & indent;
        FormatIndent(std::string & a_out, std::string & a_indent, bool contained = false) : out{a_out}, indent{a_indent}
        {
            begin_indent(out, indent, contained);
        }
        ~FormatIndent()
        {
            end_indent(out, indent);
        }
    };

} // namespace daxa