#pragma once

#include "../impl_core.hpp"

#include <variant>
#include <sstream>
#include <daxa/utils/task_graph.hpp>
#include <daxa/utils/task_graph_types.hpp>
#include <memory_resource>

// This file should house the rewrite of taskgraph.
// Please DO NOT INCLUDE ANY existing impl TG headers.
// Please ONLY INCLUDE public api task graph headers.

///
/// === Design Notes === Important Implementation Details ===
///
/// Memory Allocation
/// * allocations are made using the MemoryArena/StackAllocator classes
/// * persistent allocations go into the task_memory MemoryArena.
/// * transient allocations go into the transient_memory StackAllocator.
/// * all input data to spans/string views is reallocated and copied internally
/// * the PagedVector and StaticVector classes are used for all arrays that can grow/ when the size of an array can not be known upfront.
///

namespace daxa
{
    struct ImplTaskGraphMk2
    {
        MemoryArena task_memory = {};

        ImplTaskGraphMk2(u32 task_memory_size);

        void add_task(
            void (*task_callback)(daxa::TaskInterface, void*),
            u64* task_callback_memory,
            std::span<TaskAttachmentInfo> attachments,
            u32 attachment_shader_blob_size,
            u32 attachment_shader_blob_alignment,
            TaskType task_type,
            std::string_view name,
            Queue queue);
    };

    static constexpr usize REALITY_CHECK_MK2_SIZE = sizeof(ImplTaskGraphMk2);
} // namespace daxa
