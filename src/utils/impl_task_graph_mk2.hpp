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
    template <typename T>
    concept TrivialType = std::is_trivially_destructible_v<T>;

    struct MemoryArena
    {
        std::unique_ptr<u8[]> owned_allocation = {};
        u8* memory = {};
        u32 size = {};
        u32 used_size = {};
        u32 allocations_made = {};
        std::string_view name = {};

        MemoryArena() = default;
        MemoryArena(std::string_view name, u32 size)
        {
            this->owned_allocation = std::make_unique<u8[]>(size);
            this->memory = this->owned_allocation.get();
            this->size = size;
            this->used_size = 0;
            this->name = allocate_copy_string(name);
        }
        MemoryArena(std::string_view name, std::span<u8> external_memory)
        {
            this->owned_allocation = {};
            this->memory = external_memory.data();
            this->size = static_cast<u32>(external_memory.size());
            this->used_size = 0;
            this->name = allocate_copy_string(name);
        }

        auto allocate(usize a_size, usize a_align, usize a_count = 1u) -> u8 *
        {
            u32 const new_used_size = align_up(used_size, static_cast<u32>(a_align)) + static_cast<u32>(a_size) * static_cast<u32>(a_count);
            if (new_used_size <= size)
            {
                allocations_made += 1;
                auto new_allocation = memory + used_size;
                used_size = new_used_size;
                return new_allocation;
            }
            DAXA_DBG_ASSERT_TRUE_M(
                false,
                std::format(
                    "MemoryArena \"{}\" ran out of memory (missing_size: {}, total_size: {}, used_size: {}) while trying to allocate: {} (align: {})!",
                    name, new_used_size - used_size, size, used_size, a_size * a_count, a_align));
            return nullptr;
        }

        template <typename T>
        auto allocate_single(auto && values) -> std::unique_ptr<T, void (*)(T *)>
        {
            auto alloc = allocate(sizeof(T), alignof(T));
            return std::unique_ptr{new (alloc) T(std::forward(values)), [](T * v)
                                   { v->~T(); }};
        }

        template <TrivialType T>
        auto allocate_trivial_single() -> T *
        {
            auto alloc = allocate(sizeof(T), alignof(T));
            return reinterpret_cast<T *>(alloc);
        }

        template <TrivialType T>
        auto allocate_trivial_span(usize count) -> std::span<T>
        {
            auto alloc = allocate(sizeof(T), alignof(T), count);
            auto span = std::span<T>{reinterpret_cast<T *>(alloc), count};
            for (auto & v : span)
            {
                new (&v) T();
            }
            return span;
        }

        template <TrivialType T>
        auto allocate_trivial_span_fill(usize count, T const & fill_v) -> std::span<T>
        {
            auto alloc = allocate(sizeof(T), alignof(T), count);
            auto span = std::span<T>{reinterpret_cast<T *>(alloc), count};
            for (auto & v : span)
            {
                new (&v) T(fill_v);
            }
            return span;
        }

        auto allocate_copy_string(std::string_view sv) -> std::string_view
        {
            auto alloc = allocate_trivial_span<char>(sv.size());
            std::memcpy(alloc.data(), sv.data(), sv.size());
            return std::string_view{alloc.data(), alloc.size()};
        }
    };

    // Linearly allocated growable vector with up to 8192 elements
    template <TrivialType T>
    struct DynamicArenaArray8k
    {
        static inline constexpr u32 BLOCK_BASE_ELEMENT_COUNT = 4u; // MAKE SURE ITS POWER OF TWO
        static inline constexpr u32 BLOCK_COUNT = 12u;
        static inline constexpr u32 BLOCK_SIZES[BLOCK_COUNT] = {
            BLOCK_BASE_ELEMENT_COUNT * (1ull << 0ull), // 4
            BLOCK_BASE_ELEMENT_COUNT * (1ull << 0ull), // 4
            BLOCK_BASE_ELEMENT_COUNT * (1ull << 1ull), // 8
            BLOCK_BASE_ELEMENT_COUNT * (1ull << 2ull), // 16
            BLOCK_BASE_ELEMENT_COUNT * (1ull << 3ull), // 32
            BLOCK_BASE_ELEMENT_COUNT * (1ull << 4ull), // 64
            BLOCK_BASE_ELEMENT_COUNT * (1ull << 5ull), // 128
            BLOCK_BASE_ELEMENT_COUNT * (1ull << 6ull), // 256
            BLOCK_BASE_ELEMENT_COUNT * (1ull << 7ull), // 512
            BLOCK_BASE_ELEMENT_COUNT * (1ull << 8ull), // 1024
            BLOCK_BASE_ELEMENT_COUNT * (1ull << 9ull), // 2048
            BLOCK_BASE_ELEMENT_COUNT * (1ull << 10ull), // 4096
        };
        static inline constexpr u32 CAPACITY =
            BLOCK_BASE_ELEMENT_COUNT * (1ull << 0ull) +
            BLOCK_BASE_ELEMENT_COUNT * (1ull << 1ull) +
            BLOCK_BASE_ELEMENT_COUNT * (1ull << 2ull) +
            BLOCK_BASE_ELEMENT_COUNT * (1ull << 3ull) +
            BLOCK_BASE_ELEMENT_COUNT * (1ull << 4ull) +
            BLOCK_BASE_ELEMENT_COUNT * (1ull << 5ull) +
            BLOCK_BASE_ELEMENT_COUNT * (1ull << 6ull) +
            BLOCK_BASE_ELEMENT_COUNT * (1ull << 7ull) +
            BLOCK_BASE_ELEMENT_COUNT * (1ull << 8ull) +
            BLOCK_BASE_ELEMENT_COUNT * (1ull << 9ull) +
            BLOCK_BASE_ELEMENT_COUNT * (1ull << 10ull) +
            BLOCK_BASE_ELEMENT_COUNT * (1ull << 11ull);

        MemoryArena * allocator = {};
        u32 element_count = {};
        struct DebugView
        {
            T (*block_allocation0)[BLOCK_SIZES[0]];
            T (*block_allocation1)[BLOCK_SIZES[1]];
            T (*block_allocation2)[BLOCK_SIZES[2]];
            T (*block_allocation3)[BLOCK_SIZES[3]];
            T (*block_allocation4)[BLOCK_SIZES[4]];
            T (*block_allocation5)[BLOCK_SIZES[5]];
            T (*block_allocation6)[BLOCK_SIZES[6]];
            T (*block_allocation7)[BLOCK_SIZES[7]];
            T (*block_allocation8)[BLOCK_SIZES[8]];
            T (*block_allocation9)[BLOCK_SIZES[9]];
            T (*block_allocation10)[BLOCK_SIZES[10]];
            T (*block_allocation11)[BLOCK_SIZES[11]];
        };
        union
        {
            T * block_allocations[BLOCK_COUNT] = {};
            DebugView debug_view;
        };

        struct BlockIndex
        {
            u32 block = {};
            u32 in_block_index = {};
            static auto element_index_to_block(u32 element_index) -> BlockIndex
            {
                // First significant bit index past BLOCK_BASE_ELEMENT_COUNT is the block index:
                u32 const first_significant_bit_idx = 32u - static_cast<u32>(std::countl_zero(element_index / BLOCK_BASE_ELEMENT_COUNT));
                // Remove the block index first significant bit from index, to get in_block_index: 
                // mask is identical for first two blocks as they have the same size:
                u32 const mask_block_idx = std::max(1u,first_significant_bit_idx) - 1u;
                u32 const in_block_idx_mask = ((BLOCK_BASE_ELEMENT_COUNT << mask_block_idx) - 1u);

                BlockIndex ret = {};
                ret.block = first_significant_bit_idx;
                ret.in_block_index = element_index & in_block_idx_mask;
                return ret;
            }
        };

        DynamicArenaArray8k() = default;
        DynamicArenaArray8k(MemoryArena * a_allocator) : allocator{a_allocator} {}

        void initialize(MemoryArena * a_allocator)
        {
            this->allocator = a_allocator;
        }

        auto clone(MemoryArena * a_allocator) const -> DynamicArenaArray8k<T>
        {
            DynamicArenaArray8k<T> ret = { a_allocator };
            ret.reserve(element_count);
            for (u32 i = 0; i < element_count; ++i)
            {
                ret.push_back(this->at(i));
            }
            return ret;
        }

        auto size() const -> usize
        {
            return static_cast<usize>(element_count);
        }

        auto at(usize idx) -> T &
        {
            auto bi = BlockIndex::element_index_to_block(static_cast<u32>(idx));
            return this->block_allocations[bi.block][bi.in_block_index];
        }

        auto at(usize idx) const -> T const &
        {
            auto bi = BlockIndex::element_index_to_block(static_cast<u32>(idx));
            return this->block_allocations[bi.block][bi.in_block_index];
        }

        auto operator[](usize idx) -> T &
        {
            return this->at(idx);
        }

        auto operator[](usize idx) const -> T const &
        {
            return this->at(idx);
        }

        void push_back(T const & v)
        {
            DAXA_DBG_ASSERT_TRUE_M(element_count < CAPACITY, "ERROR: exceeded MemoryArenaVector64k capacity");
            u32 index = element_count++;
            auto bi = BlockIndex::element_index_to_block(index);

            if (block_allocations[bi.block] == 0)
            {
                block_allocations[bi.block] = allocator->allocate_trivial_span<T>(BLOCK_SIZES[bi.block]).data();
            }

            this->block_allocations[bi.block][bi.in_block_index] = v;
        }

        auto clear()
        {
            element_count = 0;
        }

        void reserve(u32 new_size)
        {
            DAXA_DBG_ASSERT_TRUE_M(new_size <= CAPACITY, "ERROR: exceeded MemoryArenaVector64k capacity");

            auto bi = BlockIndex::element_index_to_block(std::max(new_size, 1u) - 1u);
            auto max_block = bi.block;
            for (u32 b = 0; b <= max_block; ++b)
            {
                if (block_allocations[b] == nullptr)
                {
                    this->block_allocations[b] = allocator->allocate_trivial_span<T>(BLOCK_SIZES[b]).data();
                }
            }
        }

        void resize(u32 new_size, T const & value = {})
        {
            if (new_size <= element_count)
            {
                element_count = new_size;
                return;
            }
            this->reserve(new_size);
            for (u32 i = element_count; i < new_size; ++i)
            {
                this->at(i) = value;
            }
            element_count = new_size;
        }

        auto back() -> T &
        {
            DAXA_DBG_ASSERT_TRUE_M(element_count > 0, "ERROR: called back on empty Vector");
            return at(element_count - 1);
        }

        auto back() const -> T const &
        {
            DAXA_DBG_ASSERT_TRUE_M(element_count > 0, "ERROR: called back on empty Vector");
            return at(element_count - 1);
        }

        template<bool CONST_ITER>
        struct IterInternalT;

        template <> struct IterInternalT<true> { using TYPE = DynamicArenaArray8k<T> const*; };
        template <> struct IterInternalT<false> { using TYPE = DynamicArenaArray8k<T>*; };

        template <typename IT, bool CONST_ITER>
        struct ForwardIterator
        {
            IterInternalT<CONST_ITER>::TYPE vector = {};
            u32 current_index = {};

            using difference_type = std::ptrdiff_t;
            using value_type = T;

            ForwardIterator() = default;
            ForwardIterator(IterInternalT<CONST_ITER>::TYPE vector, u32 current_index)
                : vector{vector},
                  current_index{current_index}
            {
            }
            auto operator+(isize change) const -> ForwardIterator
            {
                isize const new_index = isize(current_index) + change;
                DAXA_DBG_ASSERT_TRUE_M(new_index >= 0 && new_index < vector->element_count, "ERROR: invalid iterator change");
                return ForwardIterator{vector, static_cast<u32>(new_index)};
            }
            auto operator*() -> T & requires(CONST_ITER == false) {
                return vector->at(current_index);
            } auto operator*() const -> T const &
            {
                return vector->at(current_index);
            }
            auto operator->() -> T * requires(CONST_ITER == false) {
                return &vector->at(current_index);
            } auto operator->() const -> T const *
            {
                return &vector->at(current_index);
            }
            auto operator++() -> ForwardIterator &
            {
                ++current_index;
                return *this;
            }
            auto operator++(int) -> ForwardIterator
            {
                auto tmp = *this;
                ++*this;
                return tmp;
            }
            auto operator==(ForwardIterator const & other) const -> bool
            {
                return this->vector == other.vector && this->current_index == other.current_index;
            }
            auto operator!=(ForwardIterator const & other) const -> bool
            {
                return !(*this == other);
            }
            auto distance(ForwardIterator const & other) const -> i32
            {
                return static_cast<i32>(this->current_index) - static_cast<i32>(other.current_index);
            }
            auto advance(isize change) const -> ForwardIterator
            {
                isize const new_index = isize(current_index) + change;
                DAXA_DBG_ASSERT_TRUE_M(new_index >= 0 && new_index < vector->element_count, "ERROR: invalid iterator change");
                return ForwardIterator{vector, static_cast<u32>(new_index)};
            }
        };

        using Iterator = ForwardIterator<T, false>;
        using ConstIterator = ForwardIterator<T, true>;

        auto begin() -> Iterator { return Iterator{this, 0}; }
        auto begin() const -> ConstIterator { return ConstIterator{this, 0}; }
        auto end() -> Iterator { return Iterator{this, element_count}; }
        auto end() const -> ConstIterator { return ConstIterator(this, element_count); }

        auto erase(Iterator const & iter) -> Iterator
        {
            DAXA_DBG_ASSERT_TRUE_M(iter.current_index < element_count || iter.vector != this, "ERROR: invalid iterator");
            u32 idx = iter.current_index;

            for (u32 i = idx; i < this->element_count - 1; ++i)
            {
                this->at(i) = std::move(this->at(i + 1));
            }
            --this->element_count;

            return iter;
        }

        auto insert(Iterator const & iter, T const & v) -> Iterator
        {
            DAXA_DBG_ASSERT_TRUE_M(iter.current_index <= element_count || iter.vector != this, "ERROR: invalid iterator");
            DAXA_DBG_ASSERT_TRUE_M((iter.current_index + 1) <= CAPACITY, "ERROR: insert would overflow CAPACITY");

            u32 idx = iter.current_index;

            this->push_back({});
            u32 const elements_to_move = this->element_count - idx;
            for (u32 i = 0; i < elements_to_move; ++i)
            {
                u32 const src = this->element_count - 1 - i;
                u32 const dst = this->element_count - i;
                this->at(dst) = std::move(this->at(src));
            }
            this->at(idx) = std::move(v);

            return iter;
        }

        auto insert(Iterator const & iter, Iterator const & other_begin, Iterator const & other_end) -> Iterator
        {
            DAXA_DBG_ASSERT_TRUE_M(iter.current_index <= element_count || iter.vector != this, "ERROR: invalid iterator");
            DAXA_DBG_ASSERT_TRUE_M(other_begin.vector == other_end.vector, "ERROR: insert range iterator missmatch");
            i32 const distance = other_end.distance(other_begin);
            DAXA_DBG_ASSERT_TRUE_M(distance >= 0, "ERROR: insertion range end is before begin");
            u32 const insert_size = static_cast<u32>(distance);
            DAXA_DBG_ASSERT_TRUE_M((insert_size + element_count) <= CAPACITY, "ERROR: insert would overflow CAPACITY");

            if (insert_size == 0) 
            {
                return iter;
            }

            u32 const new_size = element_count + insert_size;
            this->resize(new_size);

            // Move all elements past the insertion point back by the insertion size
            u32 const elements_to_move = this->element_count - other_begin.current_index;
            for (u32 i = 0; i < elements_to_move; ++i)
            {
                u32 const src = this->element_count - 1 - i;
                u32 const dst = this->element_count - i;
                this->at(dst) = std::move(this->at(src));
            }
            // Copy over inserted elements
            for (u32 i = 0; i < insert_size; ++i)
            {
                this->at(iter.current_index + i) = *(other_begin + i);
            }

            return iter;
        }

        auto empty() const -> bool { return element_count == 0u; }
    };

    struct ImplTaskGraphMk2
    {
        MemoryArena task_memory = {};

        ImplTaskGraphMk2(u32 task_memory_size);
    };

    static constexpr usize REALITY_CHECK_MK2_SIZE = sizeof(ImplTaskGraphMk2);
} // namespace daxa