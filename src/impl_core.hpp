#pragma once

#ifdef VULKAN_H_
#error MOST NOT INCLUDE VULKAN H BEFORE THIS FILE!
#endif

#include <unordered_map>
#include <mutex>
#include <shared_mutex>
#include <fstream>
#include <map>
#include <deque>
#include <cstring>
#include <memory>

#include <format>
#if DAXA_VALIDATION
#include <iostream>
#include <cstdlib>

#define DAXA_GPU_ID_VALIDATION 1
#endif

#if defined(_WIN32)
#ifndef VK_USE_PLATFORM_WIN32_KHR
#define VK_USE_PLATFORM_WIN32_KHR
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#elif defined(__linux__)
#if DAXA_BUILT_WITH_X11
#include <X11/Xlib.h>
#define VK_USE_PLATFORM_XLIB_KHR
// NOTE(grundlett): Cope due to X11 defines
#ifdef None
#undef None
#endif
#endif
#if DAXA_BUILT_WITH_WAYLAND
// NOTE(grundlett) Somehow the Vulkan SDK may not include these
//    - to fix this we do it manually here
#include <wayland-client.h>
#define VK_USE_PLATFORM_WAYLAND_KHR
#endif
#endif

// INCLUDE ORDER MUST STAY LIKE THIS:
// ensure we always compile in the deprecated functions to avoid link issues
#ifdef DAXA_REMOVE_DEPRECATED
#undef DAXA_REMOVE_DEPRECATED
#endif

#define DAXA_REMOVE_DEPRECATED 0
#include <daxa/daxa.hpp>
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <daxa/c/daxa.h>

using namespace daxa;

// --- Begin Helpers ---

template <typename TO_T, typename FROM_T>
auto rc_cast(FROM_T const * ptr)
{
    return reinterpret_cast<TO_T>(const_cast<FROM_T *>(ptr));
}

template <typename TO_T, typename FROM_T>
auto r_cast(FROM_T * ptr)
{
    return reinterpret_cast<TO_T>(ptr);
}

// TODO: WTF IS THIS?!
static inline constexpr char const * MAX_PUSH_CONSTANT_SIZE_ERROR = {"push constant size is limited to 128 bytes/ 32 device words"};
static inline constexpr u32 GPU_TABLE_SET_BINDING = 0;
static inline constexpr u32 CONSTANT_BUFFER_BINDING_SET = 1;
static inline constexpr u32 CONSTANT_BUFFER_BINDING_COUNT = 8;

auto is_depth_format(Format format) -> bool;

auto is_stencil_format(Format format) -> bool;

auto infer_aspect_from_format(Format format) -> VkImageAspectFlags;

auto make_subresource_range(ImageMipArraySlice const & slice, VkImageAspectFlags aspect) -> VkImageSubresourceRange;

auto make_subresource_layers(ImageArraySlice const & slice, VkImageAspectFlags aspect) -> VkImageSubresourceLayers;

inline auto infer_aspect_from_format(VkFormat format) -> VkImageAspectFlags
{
    return infer_aspect_from_format(*r_cast<Format const *>(&format));
}

inline auto make_subresource_range(daxa_ImageMipArraySlice const & slice, VkImageAspectFlags aspect) -> VkImageSubresourceRange
{
    return make_subresource_range(*r_cast<ImageMipArraySlice const *>(&slice), aspect);
}

inline auto make_subresource_layers(daxa_ImageArraySlice const & slice, VkImageAspectFlags aspect) -> VkImageSubresourceLayers
{
    return make_subresource_layers(*r_cast<ImageArraySlice const *>(&slice), aspect);
}

auto create_surface(daxa_Instance instance, daxa_NativeWindowHandle handle, daxa_NativeWindowPlatform platform, VkSurfaceKHR * out_surface) -> daxa_Result;

auto construct_daxa_physical_device_properties(VkPhysicalDevice physical_device) -> daxa_DeviceProperties;

void daxa_as_build_info_to_vk(
    daxa_Device device,
    daxa_TlasBuildInfo const * tlas_infos,
    usize tlas_count,
    daxa_BlasBuildInfo const * blas_infos,
    usize blas_count,
    std::vector<VkAccelerationStructureBuildGeometryInfoKHR> & vk_build_geometry_infos,
    std::vector<VkAccelerationStructureGeometryKHR> & vk_geometry_infos,
    std::vector<u32> & primitive_counts,
    std::vector<u32 const *> & primitive_counts_ptrs);

// --- End Helpers ---

namespace daxa
{
    struct ImplHandle
    {
        // Used for user side reference count.
        mutable u64 strong_count = 1;
        // Used for internal reference count.
        mutable u64 weak_count = {};

#define inc_weak_refcnt() impl_inc_weak_refcnt(__FUNCTION__)
#define dec_weak_refcnt(CB, IS) impl_dec_weak_refcnt(CB, IS, __FUNCTION__)

        // For user side ref counting.
        auto inc_refcnt() const -> u64;
        auto dec_refcnt(void (*zero_ref_callback)(ImplHandle const *), daxa_Instance instance) const -> u64;
        auto get_refcnt() const -> u64;
        // For internal ref counting.
        auto impl_inc_weak_refcnt(char const * callsite) const -> u64;
        auto impl_dec_weak_refcnt(void (*zero_ref_callback)(ImplHandle const *), daxa_Instance instance, char const * callsite) const -> u64;
        auto get_weak_refcnt() const -> u64;
    };
} // namespace daxa

struct MemoryBlockZombie
{
    VmaAllocation allocation = {};
};

struct daxa_ImplMemoryBlock final : ImplHandle
{
    daxa_Device device = {};
    daxa_MemoryBlockInfo info = {};
    VmaAllocation allocation = {};
    VmaAllocationInfo alloc_info = {};

    static void zero_ref_callback(ImplHandle const * handle);
};

#ifndef defer
struct defer_dummy
{
};
template <class F>
struct deferrer
{
    F f;
    ~deferrer() { f(); }
};
template <class F>
deferrer<F> operator*(defer_dummy, F f) { return {f}; }
#define DEFER_(LINE) zz_defer##LINE
#define DEFER(LINE) DEFER_(LINE)
#define defer auto DEFER(__LINE__) = defer_dummy{} * [&]()
#endif // defer

#if defined(_MSC_VER)
#define _DAXA_DEBUG_BREAK __debugbreak();
#else
#define _DAXA_DEBUG_BREAK
#endif

constexpr bool is_daxa_result_success(daxa_Result v)
{
    return v == DAXA_RESULT_SUCCESS;
}

#define _DAXA_RETURN_IF_ERROR(V, RET) \
    if (!is_daxa_result_success(V))   \
    {                                 \
        _DAXA_DEBUG_BREAK             \
        return RET;                   \
    }

#define _DAXA_DEFER_ON_ERROR(V)


/// 
/// Daxa Implementation Core Library
/// 

#include <functional>

namespace daxa
{
    template <typename T>
    concept TrivialType = std::is_trivially_destructible_v<T>;

    struct MemoryArena
    {
        u8* owned_memory = {};
        u64 owned_memory_count = {};
        u8* memory = {};
        u64 previous_memory_size = {};
        u64 total_size = {};
        u64 total_used_size = {};
        u64 current_memory_size = {};
        u64 current_memory_used_size = {};
        u64 allocations_made = {};
        std::string_view name = {};

        MemoryArena() = default;
        MemoryArena(std::string_view name, u64 a_size)
        {
            a_size = std::max(sizeof(u8) * name.size() + 1ull, a_size);
            this->owned_memory = static_cast<u8*>(::operator new[](a_size, std::align_val_t(alignof(void*))));
            this->owned_memory_count += 1ull;
            this->memory = this->owned_memory;
            this->current_memory_size = a_size;
            this->total_size = this->current_memory_size;
            this->current_memory_used_size = sizeof(void*);
            this->total_used_size = this->current_memory_used_size;
            *reinterpret_cast<void**>(this->owned_memory) = nullptr;
            this->name = allocate_copy_string(name);
        }
        MemoryArena(std::string_view name, std::span<u8> external_memory)
        {
            this->owned_memory = {};
            this->memory = external_memory.data();
            this->current_memory_size = external_memory.size();
            this->total_size = this->current_memory_size;
            this->current_memory_used_size = 0ull;
            this->total_size = 0ull;
            this->name = allocate_copy_string(name);
        }
        ~MemoryArena()
        {
            void* previous_owned_memory = reinterpret_cast<void*>(this->owned_memory);
            while (previous_owned_memory != nullptr)
            {
                void* owned_memory_before_previous = *reinterpret_cast<void**>(previous_owned_memory);
                ::operator delete[] (static_cast<u8*>(previous_owned_memory), std::align_val_t(alignof(void*)));
                previous_owned_memory = owned_memory_before_previous;
            }
            this->owned_memory = {};
            this->owned_memory_count = {};
            this->memory = {};
            this->previous_memory_size = {};
            this->total_size = {};
            this->total_used_size = {};
            this->current_memory_size = {};
            this->current_memory_used_size = {};
            this->allocations_made = {};
            this->name = {};
        }

        auto allocate(u64 a_size, u64 a_align, u64 a_count = 1u) -> u8 *
        {
            if (a_count == 0)
            {
                return nullptr;
            }
            u64 const new_used_size = align_up(this->current_memory_used_size, a_align) + a_size * a_count;

            if (new_used_size > this->current_memory_size)
            {
                auto allow_growing = this->owned_memory != nullptr;
                if (!allow_growing)
                {
                    DAXA_DBG_ASSERT_TRUE_M(
                        false,
                        std::format(
                            "MemoryArena \"{}\" ran out of memory (missing_size: {}, total_size: {}, used_size: {}) while trying to allocate: {} (align: {})!",
                            this->name, new_used_size - this->current_memory_used_size, this->current_memory_size, this->current_memory_used_size, a_size * a_count, a_align));
                    return nullptr;
                }
                else
                {
                    auto previous_memory = this->owned_memory;
                    auto new_memory_size = std::max(this->current_memory_size * 4ull, a_size * a_count * 2ull + sizeof(void*));
                    auto new_memory = static_cast<u8*>(::operator new[](new_memory_size, std::align_val_t(alignof(void*))));

                    this->previous_memory_size += this->current_memory_size;
                    this->current_memory_size = new_memory_size;
                    this->current_memory_used_size = sizeof(void*);
                    this->owned_memory_count += 1ull;
                    this->owned_memory = new_memory;
                    this->memory = new_memory;
                    this->total_size = this->previous_memory_size + new_memory_size;

                    *reinterpret_cast<void**>(new_memory) = reinterpret_cast<void*>(previous_memory);
                }
            }

            auto new_allocation = this->memory + this->current_memory_used_size;
            this->current_memory_used_size = new_used_size;
            this->total_used_size = this->previous_memory_size + this->current_memory_used_size;
            this->allocations_made += 1;
            return new_allocation;
        }

        void clear()
        {
            this->previous_memory_size = 0ull;
            this->total_size = this->current_memory_size;

            void* previous_owned_memory = nullptr;
            this->owned_memory_count = 0ull;
            this->current_memory_used_size = 0ull;
            if (this->owned_memory != nullptr)
            {
                previous_owned_memory = *reinterpret_cast<void**>(owned_memory);
                *reinterpret_cast<void**>(this->owned_memory) = nullptr;
                this->owned_memory_count = 1ull;
                this->current_memory_used_size = sizeof(void*);
            }
            this->total_used_size = this->current_memory_used_size;
            this->allocations_made = 0ull;
            this->name = this->allocate_copy_string(this->name);
            
            // Deallocation must happen after we copy the name string!
            while (previous_owned_memory != nullptr)
            {
                void* owned_memory_before_previous = *reinterpret_cast<void**>(previous_owned_memory);
                ::operator delete[] (static_cast<u8*>(previous_owned_memory), std::align_val_t(alignof(void*)));
                previous_owned_memory = owned_memory_before_previous;
            }
        }

        template <TrivialType T>
        auto allocate_trivial_single() -> T *
        {
            auto alloc = allocate(sizeof(T), alignof(T));
            return reinterpret_cast<T *>(alloc);
        }

        template <TrivialType T>
        auto allocate_trivial_span(u64 count, u64 alignment = alignof(T)) -> std::span<T>
        {
            auto alloc = allocate(sizeof(T), alignment, count);
            return std::span<T>{reinterpret_cast<T *>(alloc), count};
        }

        template <TrivialType T>
        auto allocate_trivial_span_fill(u64 count, T const & fill_v) -> std::span<T>
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
            auto alloc = allocate_trivial_span<char>(sv.size() + 1u);
            std::memcpy(alloc.data(), sv.data(), sv.size());
            alloc.data()[sv.size()] = 0u; // makes debugger happy, it does not understand string view.
            return std::string_view{alloc.data(), alloc.size() - 1u};
        }
    };

    // Linearly allocated growable vector with up to 8192 elements
    template <TrivialType T>
    struct ArenaDynamicArray8k
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

        ArenaDynamicArray8k() = default;
        ArenaDynamicArray8k(MemoryArena * a_allocator) : allocator{a_allocator} {}

        void initialize(MemoryArena * a_allocator)
        {
            this->allocator = a_allocator;
        }

        auto clone(MemoryArena * a_allocator) const -> ArenaDynamicArray8k<T>
        {
            ArenaDynamicArray8k<T> ret = { a_allocator };
            ret.reserve(this->element_count);
            ret.element_count = this->element_count;
            for (u32 e = 0; e < this->element_count; ++e)
            {
                auto bi = BlockIndex::element_index_to_block(static_cast<u32>(e));
                ret.block_allocations[bi.block][bi.in_block_index] = this->block_allocations[bi.block][bi.in_block_index];
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
            DAXA_DBG_ASSERT_TRUE_M(element_count < CAPACITY, "ERROR: exceeded ArenaDynamicArray8k capacity");
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
            DAXA_DBG_ASSERT_TRUE_M(new_size <= CAPACITY, "ERROR: exceeded ArenaDynamicArray8k capacity");

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
            DAXA_DBG_ASSERT_TRUE_M(element_count > 0, "ERROR: called back on empty ArenaDynamicArray8k");
            return at(element_count - 1);
        }

        auto back() const -> T const &
        {
            DAXA_DBG_ASSERT_TRUE_M(element_count > 0, "ERROR: called back on empty ArenaDynamicArray8k");
            return at(element_count - 1);
        }

        template <typename IT, bool CONST_ITER, typename IterT>
        struct ForwardIterator
        {
            IterT vector = {};
            u32 current_index = {};

            using difference_type = std::ptrdiff_t;
            using value_type = T;

            ForwardIterator() = default;
            ForwardIterator(IterT vector, u32 current_index)
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

        using Iterator = ForwardIterator<T, false, ArenaDynamicArray8k<T> *>;
        using ConstIterator = ForwardIterator<T, true, ArenaDynamicArray8k<T> const*>;

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

        auto clone_to_contiguous(MemoryArena* arena = nullptr) const -> std::span<T>
        {
            MemoryArena* cloning_arena = arena ? arena : this->allocator;
            std::span<T> ret = cloning_arena->allocate_trivial_span<T>(this->element_count);
            u32 i = 0;
            for (u32 block = 0; block < BLOCK_COUNT; ++block)
            {
                for (u32 e = 0; e < BLOCK_SIZES[block]; ++e)
                {
                    if (i >= this->element_count)
                    {
                        return ret;
                    }
                    ret[i] = this->block_allocations[block][e];
                    ++i;
                }
            }
            return ret;
        }
    };
}