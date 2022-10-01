#pragma once

#include <daxa/context.hpp>

#include "impl_core.hpp"

namespace daxa
{
    struct ImplContext final : ManagedSharedState
    {
        ContextInfo info;
        VkInstance vk_instance = {};
        VkDebugUtilsMessengerEXT vk_debug_utils_messenger = {};
        bool enable_debug_names = true;

        explicit ImplContext(ContextInfo info);
        virtual ~ImplContext() override final;
    };
} // namespace daxa
