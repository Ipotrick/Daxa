#pragma once

#include <unordered_map>
#include <mutex>
#include <fstream>
#include <map>
#include <deque>
#include <cstring>

#include <daxa/core.hpp>

#if defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#elif defined(__linux__)
#if DAXA_BUILT_WITH_X11
#define VK_USE_PLATFORM_XLIB_KHR
#endif
#if DAXA_BUILT_WITH_WAYLAND
#define VK_USE_PLATFORM_WAYLAND_KHR
#endif
#endif

#if DAXA_BUILT_WITH_DXC
#if defined(_WIN32)
#include "Windows.h"
#include "wrl/client.h"
using namespace Microsoft::WRL;
#include <dxcapi.h>
#else
#define SCARD_E_FILE_NOT_FOUND static_cast<HRESULT>(0x80100024)
#define SCARD_E_INVALID_PARAMETER static_cast<HRESULT>(0x80100004)
#include <dxc/dxcapi.h>
template <typename T>
using ComPtr = CComPtr<T>;
#endif
#endif

#if DAXA_BUILT_WITH_GLSLANG
#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <glslang/Include/ResourceLimits.h>
#endif

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

namespace daxa
{
    static inline constexpr u32 MAX_PUSH_CONSTANT_WORD_SIZE = {32};
    static inline constexpr u32 MAX_PUSH_CONSTANT_BYTE_SIZE = {MAX_PUSH_CONSTANT_WORD_SIZE * 4};
    static inline constexpr u32 PIPELINE_LAYOUT_COUNT = {MAX_PUSH_CONSTANT_WORD_SIZE + 1};
    static inline constexpr char const * MAX_PUSH_CONSTANT_SIZE_ERROR = {"push constant size is limited to 128 bytes/ 32 device words"};
} // namespace daxa
