#pragma once

#include <unordered_map>
#include <mutex>
#include <variant>

#include <daxa/core.hpp>

#include <volk.h>
#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#if !defined(NDEBUG)

// TODO: Figure out what to do for debug callback
// static std::function<void(daxa::MsgSeverity, daxa::MsgType, std::string_view)> * debug_callback;

#endif

#ifdef DAXA_DEBUG
#define DAXA_LOCK_WEAK(x)                                                                                \
    [&]() {                                                                                              \
        auto ptr = x.lock();                                                                             \
        if (ptr == nullptr)                                                                              \
        {                                                                                                \
            std::cerr << DAXA_DBG_ASSERT_FAIL_STRING << ": destroyed device before object" << std::endl; \
            throw std::exception("DAXA DEBUG ASSERT FAILURE");                                           \
        }                                                                                                \
        return ptr;                                                                                      \
    }()
#else
#define DAXA_LOCK_WEAK(x) \
    x.lock()
#endif
