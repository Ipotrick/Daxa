#pragma once

#ifdef VULKAN_H_
#error MOST NOT INCLUDE VULKAN H BEFORE THIS FILE!
#endif

#include <unordered_map>
#include <mutex>
#include <fstream>
#include <map>
#include <deque>
#include <cstring>
#include <fmt/format.h>

#if DAXA_VALIDATION
#include <iostream>
#include <cstdlib>
#include <stdexcept>

#define DAXA_GPU_ID_VALIDATION 1
#endif

#if defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#elif defined(__linux__)
#if DAXA_BUILT_WITH_X11
#include <X11/Xlib.h>
#define VK_USE_PLATFORM_XLIB_KHR
#endif
#if DAXA_BUILT_WITH_WAYLAND
// NOTE(grundlett) Somehow the Vulkan SDK may not include these
//    - to fix this we do it manually here
#include <wayland-client.h>
#define VK_USE_PLATFORM_WAYLAND_KHR
#endif
#endif

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include <daxa/c/daxa.h>
#include <daxa/daxa.hpp>

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

static inline constexpr u32 MAX_PUSH_CONSTANT_WORD_SIZE = {32};
static inline constexpr u32 MAX_PUSH_CONSTANT_BYTE_SIZE = {MAX_PUSH_CONSTANT_WORD_SIZE * 4};
static inline constexpr u32 PIPELINE_LAYOUT_COUNT = {MAX_PUSH_CONSTANT_WORD_SIZE + 1};
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

// --- End Helpers ---

namespace daxa
{
    struct ImplHandle
    {
        // Used for user side reference count.
        u64 strong_count = 1;
        // Used for internal reference count.
        u64 weak_count = {};

        // For user side ref counting.
        auto inc_refcnt() const -> u64;
        auto dec_refcnt(void (*zero_ref_callback)(ImplHandle const *), daxa_Instance instance) const -> u64;
        auto get_refcnt() const -> u64;
        // For internal ref counting.
        auto inc_weak_refcnt() const -> u64;
        auto dec_weak_refcnt(void (*zero_ref_callback)(ImplHandle const *), daxa_Instance instance) const -> u64;
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
    MemoryBlockInfo info = {};
    VmaAllocation allocation = {};
    VmaAllocationInfo alloc_info = {};

    static void zero_ref_callback(ImplHandle const * handle);
};