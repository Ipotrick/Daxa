#pragma once

#include <unordered_map>
#include <mutex>
#include <fstream>
#include <map>
#include <deque>
#include <cstring>
#include <format>

#include <daxa/core.hpp>

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

namespace daxa
{
    static inline constexpr u32 MAX_PUSH_CONSTANT_WORD_SIZE = {32};
    static inline constexpr u32 MAX_PUSH_CONSTANT_BYTE_SIZE = {MAX_PUSH_CONSTANT_WORD_SIZE * 4};
    static inline constexpr u32 PIPELINE_LAYOUT_COUNT = {MAX_PUSH_CONSTANT_WORD_SIZE + 1};
    static inline constexpr char const * MAX_PUSH_CONSTANT_SIZE_ERROR = {"push constant size is limited to 128 bytes/ 32 device words"};
    static inline constexpr u32 GPU_TABLE_SET_BINDING = 0;
    static inline constexpr u32 CONSTANT_BUFFER_BINDING_SET = 1;
    static inline constexpr u32 CONSTANT_BUFFER_BINDING_COUNT = 8;
} // namespace daxa
