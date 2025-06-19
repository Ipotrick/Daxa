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
/// * allocations are made using the LinearAllocator/StackAllocator classes
/// * persistent allocations go into the task_memory LinearAllocator.
/// * transient allocations go into the transient_memory StackAllocator.
/// * all input data to spans/string views is reallocated and copied internally
/// * the PagedVector and StaticVector classes are used for all arrays that can grow/ when the size of an array can not be known upfront.
///

namespace daxa
{
    struct LinearAllocator
    {
        std::unique_ptr<u8[]> memory = {};
        u32 size = {};
        u32 used_size = {};
        u32 allocations_made = {};
        std::string_view name = {};

        LinearAllocator() = default;
        LinearAllocator(std::string_view name, u32 size)
        {
            this->memory = std::make_unique<u8[]>(size);
            this->size = size;
            this->used_size = 0;
            this->name = allocate_copy_string(name);
        }

        auto allocate(usize a_size, usize a_align, usize a_count = 1u) -> u8*
        {
            u32 const new_used_size = align_up(used_size, static_cast<u32>(a_align)) + static_cast<u32>(a_size) * static_cast<u32>(a_count);
            if (new_used_size <= size)
            {
                allocations_made += 1;
                auto allocation = memory.get() + used_size;
                used_size = new_used_size;
                return allocation;
            }
            DAXA_DBG_ASSERT_TRUE_M(
                false, 
                std::format(
                    "LinearAllocator \"{}\" ran out of memory (missing_size: {}, total_size: {}, used_size: {}) while trying to allocate: {} (align: {})!", 
                    name, new_used_size - used_size, size, used_size, a_size * a_count, a_align
                )
            );
            return nullptr;
        }

        template<typename T>
        auto allocate_placed(auto && values) -> std::unique_ptr<T, void(*)(T*)>
        {
            auto alloc = allocate(sizeof(T), alignof(T));
            return std::unique_ptr{new (alloc) T(std::forward(values)), [](T* v){ v->~T(); }};
        }

        template<typename T>
        auto allocate_trivial_single() -> T*
        {
            auto alloc = allocate(sizeof(T), alignof(T));
            return reinterpret_cast<T*>(alloc);
        }

        template<typename T>
        auto allocate_trivial_span(usize count) -> std::span<T>
        {
            auto alloc = allocate(sizeof(T), alignof(T), count);
            return std::span<T>{ reinterpret_cast<T*>(alloc), count };
        }

        auto allocate_copy_string(std::string_view sv) -> std::string_view
        {
            auto alloc = allocate_trivial_span<char>(sv.size());
            std::memcpy(alloc.data(), sv.data(), sv.size());
            return std::string_view{ alloc.data(), alloc.size() };
        }
    };

    struct ImplTaskGraphMk2
    {
        LinearAllocator task_memory = {};

        ImplTaskGraphMk2(u32 task_memory_size);
    };

    static constexpr usize REALITY_CHECK_MK2_SIZE = sizeof(ImplTaskGraphMk2);
}