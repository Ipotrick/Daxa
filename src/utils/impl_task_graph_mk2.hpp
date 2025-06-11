#pragma once

#include "../impl_core.hpp"

#include <variant>
#include <sstream>
#include <daxa/utils/task_graph.hpp>
#include <daxa/utils/task_graph_types.hpp>

// This file should house the rewrite of taskgraph.
// Please DO NOT INCLUDE ANY existing impl TG headers.
// Please ONLY INCLUDE public api task graph headers.

namespace daxa
{
    namespace detail
    {
        template <typename T, usize T_CAPACITY>
        struct StaticVector
        {
            using value_type = T;
            using size_type = usize;

            static constexpr inline size_type CAPACITY = T_CAPACITY;

            T data[CAPACITY] = {};
            size_type size = 0;

            void push_back(T const & value)
            {
                DAXA_DBG_ASSERT_TRUE_M(size < CAPACITY, "StaticVector overflow");
                data[size++] = value;
            }

            T & operator[](size_type index)
            {
                DAXA_DBG_ASSERT_TRUE_M(index < size, "StaticVector index out of bounds");
                return data[index];
            }

            T const & operator[](size_type index) const
            {
                DAXA_DBG_ASSERT_TRUE_M(index < size, "StaticVector index out of bounds");
                return data[index];
            }
        };


        template <usize T_CAPACITY>
        struct LinearAllocator
        {
            u8 task_memory[T_CAPACITY] = {};
            usize back_offset = {};

            template<typename T = u8>
            T* allocate(usize count = 1)
            {
                usize upaligned_back_offset = detail::align_up(back_offset, alignof(T));
                bool const fits = sizeof(T) * count + upaligned_back_offset < T_CAPACITY;
                if (!fits)
                {
                    return nullptr;
                }
                T* allocation = static_cast<T*>(&task_memory);

                return allocation;
            }
        };
    }

    struct ImplTaskMk2
    {
        ITask* task_allocation = {}; // Used only for calling destructor.
        void (ITask::*callback)(TaskInterface) = {};
        char const * name = {};
        TaskAttachmentInfo* attachment_infos = {};
        u32 attachment_count = {};
        u32 attachment_shader_blob_size = {};
        u32 attachment_shader_blob_alignment = {};
        TaskType task_type = {};
    };

    struct ImplTaskGraphMk2
    {
        static constexpr inline usize MAX_TASKS         = 1ull << 13ull; // 8192 tasks per graph
        static constexpr inline usize MAX_TASK_MEMORY   = 1ull << 20ull; // 1 Mib

        detail::LinearAllocator<MAX_TASK_MEMORY> task_memory = {};
        detail::StaticVector<ImplTaskMk2, MAX_TASKS> tasks = {};
    };

    static constexpr usize REALITY_CHECK_MK2_SIZE = sizeof(ImplTaskGraphMk2);
}