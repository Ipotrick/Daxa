#pragma once

#include "impl_task_graph.hpp"
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

    void print_separator_to(std::string & out, std::string & indent)
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
            print_separator_to(out, indent);
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

    void validate_not_compiled(ImplTaskGraph & impl)
    {
        DAXA_DBG_ASSERT_TRUE_M(!impl.compiled, "completed task graphs can not record new tasks");
    }

    void validate_buffer_task_view(ITask & task, u32 attach_index, TaskBufferAttachment const & attach)
    {
        DAXA_DBG_ASSERT_TRUE_M(
            !attach.view.is_empty(),
            fmt::format("detected unassigned task buffer view for attachment \"{}\" (index: {}, access: {}) in task \"{}\"\n",
                        attach.name, attach_index, to_string(attach.access), task.name()));
    }

    void validate_image_task_view(ITask & task, u32 attach_index, TaskImageAttachment const & attach)
    {
        DAXA_DBG_ASSERT_TRUE_M(
            !attach.view.is_empty(),
            fmt::format("detected unassigned task image view for attachment \"{}\" (index: {}, access: {}) in task \"{}\"\n",
                        attach.name, attach_index, to_string(attach.access), task.name()));
    }

} // namespace daxa