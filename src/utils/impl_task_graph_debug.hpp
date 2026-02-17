#pragma once

#include "impl_task_graph.hpp"
#include <format>
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
        std::format_to(std::back_inserter(out), "{}--------------------------------\n", indent);
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
        DAXA_DBG_ASSERT_TRUE_M(!impl.compiled, "Completed task graphs can not record new tasks");
    }

    template <typename BufferBlasTlasAttachmentT>
    void validate_buffer_blas_tlas_task_view(ImplTask const & task, u32 attach_index, BufferBlasTlasAttachmentT const & attach)
    {
        bool const type_restriction_upheld = attach.task_access.restriction == BufferBlasTlasAttachmentT::ATTACHMENT_TYPE || attach.task_access.restriction == TaskAttachmentType::UNDEFINED;
        bool const view_filled_or_null = !attach.view.is_empty();
        DAXA_DBG_ASSERT_TRUE_M(
            type_restriction_upheld,
            std::format("Detected TaskAccess that is not compatible with Resource type \"{}\" (index: {}, access: {}) in task \"{}\"\n",
                        attach.name, attach_index, to_string(attach.task_access), task.name));
        DAXA_DBG_ASSERT_TRUE_M(
            view_filled_or_null,
            std::format("Detected unassigned task buffer view for attachment \"{}\" (index: {}, access: {}) in task \"{}\"\n",
                        attach.name, attach_index, to_string(attach.task_access), task.name));
    }

    void validate_image_task_view(ImplTask const & task, u32 attach_index, TaskImageAttachmentInfo const & attach)
    {
        bool const type_restriction_upheld = attach.task_access.restriction == TaskImageAttachmentInfo::ATTACHMENT_TYPE || attach.task_access.restriction == TaskAttachmentType::UNDEFINED;
        bool const view_filled_or_null = !attach.view.is_empty();
        DAXA_DBG_ASSERT_TRUE_M(
            type_restriction_upheld,
            std::format("Detected TaskAccess that is not compatible with Resource type \"{}\" (index: {}, access: {}) in task \"{}\"\n",
                        attach.name, attach_index, to_string(attach.task_access), task.name));
        DAXA_DBG_ASSERT_TRUE_M(
            view_filled_or_null,
            std::format("Detected unassigned task image view for attachment \"{}\" (index: {}, access: {}) in task \"{}\"\n",
                        attach.name, attach_index, to_string(attach.task_access), task.name));
    }

    template <typename BufferBlasTlasT>
    void validate_task_buffer_blas_tlas_runtime_data(ImplTask & task, BufferBlasTlasT const & attach)
    {
#if DAXA_VALIDATION
        if constexpr (std::is_same_v<BufferBlasTlasT, TaskBufferAttachmentInfo>)
        {
            DAXA_DBG_ASSERT_TRUE_M(
                attach.ids.size() >= attach.shader_array_size,
                std::format("Detected invalid runtime buffer count.\n"
                            "Attachment \"{}\" in task \"{}\" requires {} runtime buffer(s), but only {} runtime buffer(s) are present when executing task.\n"
                            "Attachment runtime buffers must be at least as many as its shader array size!",
                            attach.name, task.name, attach.shader_array_size, attach.ids.size()));
        }
#endif
    }
} // namespace daxa